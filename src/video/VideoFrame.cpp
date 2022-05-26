#include "VideoFrame.h"

/**
    Construct a VideoFrame object which allocates its own internal AVPicture.

    The AVPicture will be freed in the destructor.
*/
VideoFrame::VideoFrame( AVPixelFormat format, uint32_t width, uint32_t height )
:
    m_format        ( format ),
    m_width         ( width  ),
    m_height        ( height ),
    m_freePicture   ( true )
{
    avpicture_alloc( &m_picture, format, width, height );
}

/**
    Construct a VideoFrame object which wraps the specified buffer.

    This object will never modify the wrapped buffer.
*/
VideoFrame::VideoFrame( uint8_t* buffer, AVPixelFormat format, uint32_t width, uint32_t height, uint32_t stride )
:
    m_format        ( format ),
    m_width         ( width  ),
    m_height        ( height ),
    m_freePicture   ( false )
{
    memset( &m_picture, 0 , sizeof(m_picture) );

    m_picture.data[0] = buffer;
    m_picture.data[1] = 0;
    m_picture.data[2] = 0;
    m_picture.data[3] = 0;
    m_picture.linesize[0] = stride;
    m_picture.linesize[1] = 0;
    m_picture.linesize[2] = 0;
    m_picture.linesize[3] = 0;

    if ( format == AV_PIX_FMT_YUV420P )
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

/*void VideoFrame::operator = ( VideoFrame&& moved )
{
    std::swap( m_format, moved.m_format );
    std::swap( m_width, moved.m_width );
    std::swap( m_height, moved.m_height );
    std::swap( const_cast<bool&>(m_freePicture), const_cast<bool&>(moved.m_freePicture) );
    /// @todo - Does this work?:
    std::swap( m_picture, moved.m_picture );
}*/

/**
    Copy the internal AVPicture data pointers to the specified AVFrame.

    @note This is intended for quick copyless transfers of frame data - no
    data is not copied only the pointers - hence the AVFrame parameter
    must never be free'd with avframe_free().

    @param frame The frame whos pointers will be modified.
*/
void VideoFrame::FillAvFramePointers( AVFrame& frame ) const
{
    frame.data[0] = m_picture.data[0];
    frame.data[1] = m_picture.data[1];
    frame.data[2] = m_picture.data[2];
    frame.data[3] = m_picture.data[3];
    frame.linesize[0] = m_picture.linesize[0];
    frame.linesize[1] = m_picture.linesize[1];
    frame.linesize[2] = m_picture.linesize[2];
    frame.linesize[3] = m_picture.linesize[3];
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

AVPixelFormat VideoFrame::GetAvPixelFormat() const
{
    return m_format;
}

