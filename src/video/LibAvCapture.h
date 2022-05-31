/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LIB_AV_CAPTURE_H__
#define __LIB_AV_CAPTURE_H__

#include <stdint.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "FrameConverter.h"

#include "Capture.h"

class FFMpegCustomIO;

/**
    Class which uses libavcodec to read video streams from a video file.
*/
class LibAvCapture : public Capture
{
private:
    void Init( const char* streamName );

public:
    LibAvCapture( const char* videoFile );
    LibAvCapture( FFMpegCustomIO& customIO );
    virtual ~LibAvCapture();

    bool IsOpen() const;
    bool IoError() const;

    bool GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    timespec GetFrameTimestamp() const;

    void ExtractLuminanceImage( uint8_t* data, int stride );
    void ExtractRgbaImage( uint8_t* data, int stride );
    void ExtractRgbImage( uint8_t* data, int stride );
    void ExtractBgrImage( uint8_t* data, int stride );
    void ExtractRgb565Image( uint8_t* data, int stride );

    static void InitLibAvCodec();

protected:
    void FrameConversion( AVPixelFormat format, uint8_t* data, int stride );

private:
    AVFormatContext* m_formatContext;
    FFMpegCustomIO*  m_customIO;
    AVCodecContext*  m_codecContext;
    AVCodec*         m_codec;
    AVFrame*         m_avFrame;
    int              m_videoStream;
    FrameConverter   m_converter;
    bool m_open;
};

#endif /* __LIB_AV_CAPTURE_H__ */

