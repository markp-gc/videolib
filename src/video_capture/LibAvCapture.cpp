#include "LibAvCapture.h"

#include <assert.h>

/**
    Static member to register all codecs with libav.
    This is not thread safe.
*/
void LibAvCapture::InitLibAvCodec()
{
    static int called = 0;
    if ( called == 0 )
    {
        called = 1;
        av_register_all();
    }
}

/**
    Construct a LibAvCapture object that is ready to read from the specified file.

    @param videoFile video file in a valid format (please see libavcodec docs on your platform for supported formats).

    @note This calls the static member InitLibAvCodec() so is not thread safe. If you are using this
    class in a multi-threded system then call InitLibAvCodec() manually in your start up code before launching the multi-threaded components.
*/
LibAvCapture::LibAvCapture( const char* videoFile )
:
    m_formatContext ( 0 ),
    m_codecContext  ( 0 ),
    m_open ( false )
{
    InitLibAvCodec();

    m_open = avformat_open_input( &m_formatContext, videoFile, 0, 0) == 0;
    if ( m_open == false )
    {
        return;
    }

    int foundStreamInfo = av_find_stream_info( m_formatContext );
    if ( foundStreamInfo == -1)
    {
        m_open = false;
        return;
    }

    // Find the first video stream
    m_videoStream = -1;
    for(unsigned int i=0; i<m_formatContext->nb_streams; i++)
    {
        if(m_formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            m_videoStream = i;
            break;
        }
    }

    if ( m_videoStream == -1)
    {
        m_open = false;
        return;
    }

    // Get a pointer to the codec context for the video stream
    m_codecContext = m_formatContext->streams[m_videoStream]->codec;

    // Find the decoder for the video stream
    m_codec = avcodec_find_decoder( m_codecContext->codec_id );
    if( m_codec == 0 )
    {
        m_open = false;
        return;
    }

    // Open codec
    if( avcodec_open2( m_codecContext, m_codec, 0 ) < 0 )
    {
        m_open = false;
        return;
    }

    // Hack to correct wrong frame rates that seem to be generated by some codecs
    if( m_codecContext->time_base.num>1000 && m_codecContext->time_base.den==1)
    {
        m_codecContext->time_base.den=1000;
    }

    // Allocate video frame
    m_avFrame = avcodec_alloc_frame();
}

LibAvCapture::~LibAvCapture()
{
    if ( m_open )
    {
        av_free( m_avFrame );
        avcodec_close( m_codecContext );
        av_close_input_file( m_formatContext );
    }
}

bool LibAvCapture::IsOpen() const
{
    return m_open;
}

/**
    Read frame and buffer it internally.

    @return false if there are no more frames to read, true otherwise.
*/
bool LibAvCapture::GetFrame()
{
    if ( m_open == false )
    {
        return false;
    }

    int frameFinished;

    while( av_read_frame( m_formatContext, &m_packet) >= 0 )
    {
        // Is this a packet from the video stream?
        if( m_packet.stream_index == m_videoStream )
        {
            // Decode video frame
            avcodec_decode_video2( m_codecContext, m_avFrame, &frameFinished, &m_packet );

            // Did we get a video frame?
            if( frameFinished )
            {
                return true;
            }
        }
    }

    return false;
}

/**
    Frees memory allocated during GetFrame(), hence not calling this will cause a memory leak.
*/
void LibAvCapture::DoneFrame()
{
    av_free_packet( &m_packet );
}

int32_t LibAvCapture::GetFrameWidth() const
{
    return m_codecContext->width;
}

int32_t LibAvCapture::GetFrameHeight() const
{
    return m_codecContext->height;
}

uint64_t LibAvCapture::GetFrameTimestamp() const
{
    return m_avFrame->pts; // @todo this is not correct - needs to be converted from 'time_base' units
}

void LibAvCapture::ExtractLuminanceImage( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_GRAY8, data, stride );
}

void LibAvCapture::ExtractRgbImage( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_RGB24, data, stride );
}

void LibAvCapture::ExtractBgrImage( uint8_t* data, int stride )
{
    FrameConversion( PIX_FMT_BGR24, data, stride );
}

/**
    Uses swscale library to convert the most recently read frame to
    the specified format.

    @note It is currently assumed the output image will be the same size as the
    video frame read from the file.

    @param data pointer to buffer that must be large enough to hold the data
    @param stride number of bytes to jump between rows in data.
*/
void LibAvCapture::FrameConversion( PixelFormat format, uint8_t* data, int stride )
{
    const int w = m_codecContext->width;
    const int h = m_codecContext->height;
    uint8_t* dstPlanes[4] = { data, 0, 0, 0 };
    int dstStrides[4] = { stride, 0, 0, 0 };

    m_converter.Configure( w, h, m_codecContext->pix_fmt, w, h, format );
    m_converter.Convert( m_avFrame->data, m_avFrame->linesize, 0, h, dstPlanes, dstStrides );
}

