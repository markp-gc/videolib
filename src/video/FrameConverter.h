/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FRAME_CONVERTER_H__
#define __FRAME_CONVERTER_H__

#include <stdint.h>

extern "C" {
#include <libswscale/swscale.h>
}

class VideoFrame;

/**
    Class which holds and caches the context for frame conversion.

    This currently wraps an SwsContext from the sws_scale library.
*/
class FrameConverter
{
public:
    FrameConverter(  );
    virtual ~FrameConverter();

    bool Configure( int srcW, int srcH, AVPixelFormat srcFormat, int dstW, int dstH, AVPixelFormat dstFormat );
    bool Convert( uint8_t* src[], int srcStride[], int rowOffset, int rowHeight, uint8_t* dst[], int dstStride[] ) const;
    bool Convert( const VideoFrame& frame, uint8_t* dst[], int dstStride[] ) const;

private:
    SwsContext* m_context;
};

#endif /* __FRAME_CONVERTER_H__ */

