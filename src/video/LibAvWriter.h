/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LIB_AV_WRITER_H__
#define __LIB_AV_WRITER_H__

#include <stdint.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "FrameConverter.h"

class LibAvVideoStream;
class FFMpegCustomIO;
class VideoFrame;

/**
    Class which uses libavcodec to write video streams to a video file.
*/
class LibAvWriter
{
private:
    void Init();

public:
    LibAvWriter( const char* videoFile );
    LibAvWriter( FFMpegCustomIO& customIO );
    virtual ~LibAvWriter();

    bool IsOpen() const;
    bool IoError() const;

    bool AddVideoStream( uint32_t width, uint32_t height, uint32_t fps, int32_t fourcc );
    bool PutVideoFrame( VideoFrame& frame );

protected:
    bool WriteCodecFrame( AVFrame* );

private:
    static const int BUFFER_SIZE = AV_INPUT_BUFFER_MIN_SIZE+(640*480*4);

    AVOutputFormat*  m_outputFormat;
    AVFormatContext* m_formatContext;
    FFMpegCustomIO*  m_customIO;
    AVFrame*         m_codecFrame;

    LibAvVideoStream* m_stream;
    FrameConverter    m_converter;

    bool    m_open;

public:
    // For benchmarking only:
    double lastConvertTime_ms;
    double lastEncodeTime_ms;
    double lastPacketWriteTime_ms;
};

#endif /* __LIB_AV_WRITER_H__ */

