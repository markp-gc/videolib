#include "VideoFrame.h"

/**
    Construct a VideoFrame object which allocates its own internal AVPicture.

    The AVPicture will be freed in the destructor.
*/
VideoFrame::VideoFrame( PixelFormat format, uint32_t width, uint32_t height )
:
    m_format( format ),
    m_width ( width  ),
    m_height( height ),
    m_freePicture (true)
{
    avpicture_alloc( &m_picture, format, width, height );
}

/**
    Construct a VideoFrame object which wraps the specified buffer.

    This object will never modify the wrapped buffer.
*/
VideoFrame::VideoFrame( uint8_t* buffer, PixelFormat format, uint32_t width, uint32_t height, uint32_t stride )
:
    m_format( format ),
    m_width ( width  ),
    m_height( height ),
    m_freePicture (false)
{
    m_picture.data[0] = buffer;
    m_picture.data[1] = 0;
    m_picture.data[2] = 0;
    m_picture.data[3] = 0;
    m_picture.linesize[0] = stride;
    m_picture.linesize[1] = 0;
    m_picture.linesize[2] = 0;
    m_picture.linesize[3] = 0;

    if ( format == PIX_FMT_YUV420P )
    {
        m_picture.data[1]   = buffer + (width*height);
        m_picture.linesize[1] = width/2;
        m_picture.data[2]   = m_picture.data[1] + (width*height/4);
        m_picture.linesize[2] = width/2;
    }
}

VideoFrame::~VideoFrame()
{
    if ( m_freePicture )
    {
        avpicture_free( &m_picture );
    }
}

AVPicture& VideoFrame::GetAvPicture()
{
    return m_picture;
}

const AVPicture& VideoFrame::GetAvPicture() const
{
    return m_picture;
}

int VideoFrame::GetWidth() const
{
    return m_width;
}

int VideoFrame::GetHeight() const
{
    return m_height;
}

PixelFormat VideoFrame::GetAvPixelFormat() const
{
    return m_format;
}

