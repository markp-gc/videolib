#include "LibAvVideoStream.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
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
    m_codec (0),
    m_codecContext (0),
    m_encodingBuffer (0)
{
    const AVCodecTag *tags[] = { avformat_get_riff_video_tags(), 0 };
    AVCodecID codecId = av_codec_get_id( tags, fourcc );

    m_codec = avcodec_find_encoder( codecId );
    m_stream = avformat_new_stream( context, m_codec );
    if ( m_stream != 0 )
    {
        m_codecContext = avcodec_alloc_context3( m_codec );
        m_codecContext->codec_id  = codecId;
        m_codecContext->codec_tag = 0;
        m_codecContext->pix_fmt = LibAvVideoStream::ChooseCodecFormat( codecId, AV_PIX_FMT_RGB24 );
        m_codecContext->bit_rate = 12000000;
        m_codecContext->bit_rate_tolerance = 4000000;
        // Short GOP improves streaming recovery and reduces latency.
        m_codecContext->gop_size = 15;
        // B frames induce latency and buffering which we do not want for a live stream:
        m_codecContext->max_b_frames = 0;

        m_codecContext->thread_count = 0;
        m_codecContext->thread_type = FF_THREAD_SLICE;

        if (context->oformat && (context->oformat->flags & AVFMT_GLOBALHEADER)) {
            m_codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        if ( codecId == AV_CODEC_ID_H264 )
        {
            // Low-latency encoder tuning.
            av_opt_set(m_codecContext->priv_data, "preset", "ultrafast", 0);
            av_opt_set(m_codecContext->priv_data, "tune", "zerolatency", 0);
            av_opt_set(m_codecContext->priv_data, "rc-lookahead", "0", 0);
        }

        assert( width%2 == 0 );
        assert( height%2 == 0 );
        m_codecContext->width = width;
        m_codecContext->height = height;
        m_codecContext->time_base.num = 1;
        if ( fps == 0) { fps = 1; }
        m_codecContext->time_base.den = fps; /** @todo - we want to enable non-fixed fps content so need to allow caller to specify time-base somehow. */

        m_stream->time_base = m_codecContext->time_base;

        avcodec_parameters_from_context(m_stream->codecpar, m_codecContext);

        m_bufferSize = width*height*4;
        m_encodingBuffer = reinterpret_cast<uint8_t*>( av_malloc( m_bufferSize ) );
    }
}

LibAvVideoStream::~LibAvVideoStream()
{
    av_free( m_encodingBuffer );
    avcodec_free_context( &m_codecContext );
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
    return m_codecContext;
}

const AVCodec* LibAvVideoStream::Codec()
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
