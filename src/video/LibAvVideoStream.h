/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LIB_AV_VIDEO_STREAM_H__
#define __LIB_AV_VIDEO_STREAM_H__

#include <stdint.h>

extern "C" {
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
}

struct AVStream;
struct AVFormatContext;
struct AVCodec;
struct AVCodecContext;

class LibAvVideoStream
{
public:
    LibAvVideoStream( AVFormatContext* context, uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc );
    virtual ~LibAvVideoStream();

    bool IsValid() const;

    AVCodecContext* CodecContext();
    AVCodec* Codec();
    uint32_t BufferSize() const;
    uint8_t* Buffer();

    int Index() const;

    AVRational TimeBase();

    static AVPixelFormat ChooseCodecFormat( AVCodecID id, AVPixelFormat inputFormat );

private:
    AVStream* m_stream;
    AVCodec* m_codec;
    uint32_t m_bufferSize;
    uint8_t* m_encodingBuffer;
};

#endif /* __LIB_AV_VIDEO_STREAM_H__ */

