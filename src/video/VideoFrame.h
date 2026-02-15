/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __VIDEO_FRAME_H__
#define __VIDEO_FRAME_H__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

/**
    Holds or wraps video frame data.
*/
class VideoFrame
{
public:
    VideoFrame( const VideoFrame& ) = delete;
    VideoFrame( AVPixelFormat format, uint32_t width, uint32_t height );
    VideoFrame( uint8_t* buffer, AVPixelFormat format, uint32_t width, uint32_t height, uint32_t stride );
    virtual ~VideoFrame();

    void operator = ( const VideoFrame& ) = delete;
    void operator = ( VideoFrame&& ) = delete;

    uint8_t** GetData();
    const uint8_t* const* GetData() const;
    const int* GetLineSize() const;

    void FillAvFramePointers( AVFrame& frame ) const;

    int GetWidth() const;
    int GetHeight() const;
    AVPixelFormat GetAvPixelFormat() const;

private:
    uint8_t*      m_data[AV_NUM_DATA_POINTERS];
    int           m_linesize[AV_NUM_DATA_POINTERS];
    AVPixelFormat m_format;
    int         m_width;
    int         m_height;
    const bool  m_freePicture;
};

#endif /* __VIDEO_FRAME_H__ */
