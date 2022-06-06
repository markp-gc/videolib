#include "LibAvVideoStream.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <assert.h>

#include "LibAvWriter.h"

/**
    Static method for choosing optimal encoding pixel format.

    @note PixelFormat is a enum defiend by ffmpeg (in pixfmt.h).

    @param id the CodecID to be used
    @param inputFormat the pixel format the data will be supplied in.
    @return a valid format for the specified CodecID
*/
AVPixelFormat LibAvVideoStream::ChooseCodecFormat( AVCodecID id, AVPixelFormat inputFormat )
{
    AVPixelFormat pixelFormat = AV_PIX_FMT_YUV420P;

    switch ( id )
    {
        case AV_CODEC_ID_FFV1:
        pixelFormat = AV_PIX_FMT_YUV422P;
        break;

        case AV_CODEC_ID_HUFFYUV:
        pixelFormat = AV_PIX_FMT_YUV422P;
        break;

        case AV_CODEC_ID_MJPEG:
        pixelFormat = AV_PIX_FMT_YUVJ420P;
        break;

        case AV_CODEC_ID_RAWVIDEO:
        pixelFormat = inputFormat;
        break;

        default:
        break;
    }

    return pixelFormat;
}

/**
    @todo input format should be set here and passed to ChooseCodecFormat - it can then choose optimal codec format.

    @param width  The desired width of the output video.
    @param height The desired height of the output video.
    @param fps    The desired frame rate of the output video in frames per second.
    @param fourcc The fourcc code for the video compression codec to be used.
*/
LibAvVideoStream::LibAvVideoStream( AVFormatContext* context, uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc )
:
    m_stream    (0),
    m_encodingBuffer (0)
{
    const AVCodecTag *tags[] = { avformat_get_riff_video_tags(), 0 };
    AVCodecID codecId = av_codec_get_id( tags, fourcc );

    m_codec = avcodec_find_encoder( codecId );
    m_stream = avformat_new_stream( context, m_codec );
    if ( m_stream != 0 )
    {
        CodecContext()->codec_id  = codecId;
        CodecContext()->codec_tag = fourcc;
        CodecContext()->pix_fmt = LibAvVideoStream::ChooseCodecFormat( codecId, AV_PIX_FMT_RGB24 );
        CodecContext()->bit_rate = 5000000;
        CodecContext()->bit_rate_tolerance = 1000000;
        CodecContext()->gop_size = 100;
        // B frames induce latency and buffering which we do not want for a live stream:
        CodecContext()->max_b_frames = 0;

        CodecContext()->thread_count = 0;
        CodecContext()->thread_type = FF_THREAD_SLICE;

        assert( width%2 == 0 );
        assert( height%2 == 0 );
        CodecContext()->width = width;
        CodecContext()->height = height;
        CodecContext()->time_base.num = 1;
        if ( fps == 0) { fps = 1; }
        CodecContext()->time_base.den = fps; /** @todo - we want to enable non-fixed fps content so need to allow caller to specify time-base somehow. */

        m_stream->time_base = CodecContext()->time_base;

        avcodec_parameters_from_context(m_stream->codecpar, CodecContext());

        m_bufferSize = width*height*4;
        m_encodingBuffer = reinterpret_cast<uint8_t*>( av_malloc( m_bufferSize ) );
    }
}

LibAvVideoStream::~LibAvVideoStream()
{
    av_free( m_encodingBuffer );
    avcodec_close( m_stream->codec );
}

bool LibAvVideoStream::IsValid() const
{
    return m_stream != 0;
}

/**
    Return the codec context of the underlying video stream.

    It is an error to call this if LibAvVideoStream::IsValid() returns false.
*/
AVCodecContext* LibAvVideoStream::CodecContext()
{
    return m_stream->codec;
}

AVCodec* LibAvVideoStream::Codec()
{
    return m_codec;
}

uint32_t LibAvVideoStream::BufferSize() const
{
    return m_bufferSize;
}

uint8_t* LibAvVideoStream::Buffer()
{
    return m_encodingBuffer;
}

int LibAvVideoStream::Index() const
{
    return m_stream->index;
}

/**
    Return the time base of the underlying video stream as an AVRational structure.

    It is an error to call this if LibAvVideoStream::IsValid() returns false.
*/
AVRational LibAvVideoStream::TimeBase()
{
    return m_stream->time_base;
}


