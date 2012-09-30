#include "LibAvWriter.h"

#include "VideoFrame.h"
#include "LibAvCapture.h"
#include "LibAvVideoStream.h"
#include "FFmpegCustomIO.h"

extern "C" {
#include <libavutil/mathematics.h>
}

#include <ctype.h>
#include <assert.h>
#include <time.h>

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

/**
    Static method returns an 32-bit integer representing the four character code (fourcc).

    Lower case characters are converted to upper case before computing the code.
*/
int32_t LibAvWriter::FourCc( char c1, char c2, char c3, char c4 )
{
    int fourcc = toupper(c4);
    fourcc <<= 8;
    fourcc |= toupper(c3);
    fourcc <<= 8;
    fourcc |= toupper(c2);
    fourcc <<= 8;
    fourcc |= toupper(c1);
    return fourcc;
}

/**
    Initialisation common to all constructors.
    
    Should only be called from within a constructor.
*/
void LibAvWriter::Init()
{
    LibAvCapture::InitLibAvCodec();

    m_formatContext = avformat_alloc_context();

    if ( m_formatContext == 0 )
    {
        return;
    }

    if ( m_customIO != 0 )
    {
        m_formatContext->pb = m_customIO->GetAVIOContext();
    }

    m_open = true;
}

/**
    Opens video file for writing. Auto-detects format from filename extension.

    @param videoFile File name to write - if the file exists it will be over-written.
*/
LibAvWriter::LibAvWriter( const char* videoFile )
:
    m_formatContext  (0),
    m_customIO       (0),
    m_stream         (0),
    m_open ( false )
{
    Init();
    if ( m_open )
    {
        // This guesses the container format from the file name (e.g. .avi, .ogg):
        m_outputFormat = av_guess_format( 0, videoFile, 0 );
        if ( m_outputFormat == 0 )
        {
            m_open = false;
            return;
        }

        m_formatContext->oformat = m_outputFormat;
        snprintf( m_formatContext->filename, sizeof(m_formatContext->filename), "%s", videoFile );
    }
}

/**
    Construct a writer that will use custom I/O.

    @param customIO the custom io object that must provide an AVIOContext
    that is valid for output.
*/
LibAvWriter::LibAvWriter( FFMpegCustomIO& customIO )
:
    m_formatContext  (0),
    m_customIO       (&customIO), // m_customIO ptr should not need to be deleted locally!
    m_stream         (0),
    m_open ( false )
{
    Init();
    if ( m_open )
    {
        // This guesses the container format (e.g. .avi, .ogg):
        m_outputFormat = av_guess_format( "avi", 0, 0 );
        m_formatContext->oformat = m_outputFormat;
        snprintf( m_formatContext->filename, sizeof(m_formatContext->filename), "%s", customIO.GetStreamName() );
    }
}

LibAvWriter::~LibAvWriter()
{
    if ( m_stream && m_stream->IsValid() )
    {
        avpicture_free( reinterpret_cast<AVPicture*>( &m_codecFrame ) );
        av_write_trailer( m_formatContext );

        if ( m_customIO == 0 )
        {
            avio_close( m_formatContext->pb );
        }
    }

    delete m_stream;

    if ( m_formatContext != 0 )
    {
        avformat_free_context( m_formatContext );
    }
}

bool LibAvWriter::IsOpen() const
{
    return m_open;
}

/**
    @param width width of images to be encoded in this stream
    @param height height of images to be encoded in this stream
    @param fps Frame rate in frames-per-second
    @param fourcc four character code as returned from LibAvWriter::FourCc().
*/
bool LibAvWriter::AddVideoStream( uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc )
{
    bool success = false;

    if ( IsOpen() )
    {
        m_stream = new LibAvVideoStream( m_formatContext, width, height, fps, fourcc );
        if ( m_stream->IsValid() )
        {
            avcodec_get_frame_defaults( &m_codecFrame );
            int err = avpicture_alloc( reinterpret_cast<AVPicture*>( &m_codecFrame ), m_stream->CodecContext()->pix_fmt, m_stream->CodecContext()->width, m_stream->CodecContext()->height );
            assert( err == 0 );

            m_codecFrame.pts = 0;
            av_dump_format( m_formatContext, 0, m_formatContext->filename, 1 );

            // We don't check result of above because the following fails gracefully if m_codec==null
            err = avcodec_open2( m_stream->CodecContext(), m_stream->Codec(), 0 );
            if ( err == 0 )
            {
                success = true;
            }
        }
    }

    // @todo - can this be done elsewhere - what if we had more than 1 stream?
    if ( success )
    {
        // We only need to call avio_open if we are not using custom I/O:
        if ( m_customIO == 0 )
        {
            int err = avio_open( &m_formatContext->pb, m_formatContext->filename, AVIO_FLAG_WRITE );
            if ( err != 0 )
            {
                success = false;
            }
        }

        if ( success )
        {
            avformat_write_header( m_formatContext, 0 );
        }
    }

    return success;
}

/**
    Write a video frame to the video file.
    @note If the width and height of the VideoFrame do not match those of the stream, the image will be scaled.
    @note If a colour video stream has not been setup then the video will be converted to greyscale.
*/
bool LibAvWriter::PutVideoFrame( VideoFrame& frame )
{
    const int width  = frame.GetWidth();
    const int height = frame.GetHeight();
    const PixelFormat format = frame.GetAvPixelFormat();
    AVCodecContext* codecContext = m_stream->CodecContext();

    struct timespec t1;
    struct timespec t2;
    clock_gettime( CLOCK_MONOTONIC, &t1 );

    AVFrame srcFrame;
    avcodec_get_frame_defaults( &srcFrame );

    AVFrame* frameToSend;
    if (  format == codecContext->pix_fmt )
    {
        // No conversion needed so just copy pointers:
        frame.FillAvFramePointers( srcFrame );
        frameToSend = &srcFrame;
    }
    else
    {
        if ( m_converter.Configure( width, height, format, codecContext->width, codecContext->height, codecContext->pix_fmt ) )
        {
            m_converter.Convert( frame, m_codecFrame.data, m_codecFrame.linesize );
            frameToSend = &m_codecFrame;
        }
        else
        {
            // Could not configure the frame convertor:
            return false;
        }
    }

    clock_gettime( CLOCK_MONOTONIC, &t2 );
    lastConvertTime_ms = milliseconds(t2) - milliseconds(t1);

    m_codecFrame.pts += 1;
    srcFrame.pts = m_codecFrame.pts;
    bool success = WriteCodecFrame( frameToSend );
    return success;
}

/**
    Write the current codec picture to the current stream.
*/
bool LibAvWriter::WriteCodecFrame( AVFrame* frame )
{
    bool ok = false;

    assert( frame != 0 );

    AVCodecContext* codecContext = m_stream->CodecContext();
    AVPacket pkt;
    // Note: used to set pkt.stream_index but encode_video2 now seems to set it using the codec context.
    pkt.data = m_stream->Buffer();
    pkt.size = m_stream->BufferSize();
    int packetOk;

    struct timespec t1;
    struct timespec t2;

    clock_gettime( CLOCK_MONOTONIC, &t1 );
    int err = avcodec_encode_video2( codecContext, &pkt, frame, &packetOk );
    clock_gettime( CLOCK_MONOTONIC, &t2 );
    lastEncodeTime_ms = milliseconds(t2) - milliseconds(t1);

    if ( err == 0 && packetOk == 1 )
    {
        // Note: not sure if we need to do this anymore as it gets set in encode_video2:
        pkt.pts = av_rescale_q( codecContext->coded_frame->pts, codecContext->time_base, m_stream->TimeBase() );

        if ( codecContext->coded_frame->key_frame )
        {
            pkt.flags |= AV_PKT_FLAG_KEY;
        }

        clock_gettime( CLOCK_MONOTONIC, &t1 );
        err = av_write_frame( m_formatContext, &pkt );
        clock_gettime( CLOCK_MONOTONIC, &t2 );
        lastPacketWriteTime_ms = milliseconds(t2) - milliseconds(t1);

        ok = err == 0;
    }

    return ok;
}

