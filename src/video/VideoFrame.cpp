#include "VideoFrame.h"

extern "C" {
#include <libavutil/imgutils.h>
}

#include <iostream>

/**
    Construct a VideoFrame object which allocates its own internal image buffers.

    The image buffers will be freed in the destructor.
*/
VideoFrame::VideoFrame( AVPixelFormat format, uint32_t width, uint32_t height )
:
    m_format        ( format ),
    m_width         ( width  ),
    m_height        ( height ),
    m_freePicture   ( true )
{
    int err = av_image_alloc(m_data, m_linesize, width, height, format, 32);
    if ( err < 0 )
    {
        std::cerr << "av_image_alloc failed with error " << err << "\n";
    }

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
    memset( m_data, 0, sizeof(m_data) );
    memset( m_linesize, 0, sizeof(m_linesize) );

    m_data[0] = buffer;
    m_linesize[0] = stride;

    if ( format == AV_PIX_FMT_YUV420P )
    {
        m_data[1] = buffer + (width*height);
        m_linesize[1] = width/2;
        m_data[2] = m_data[1] + (width*height/4);
        m_linesize[2] = width/2;
    }

}

VideoFrame::~VideoFrame()
{
    if ( m_freePicture )
    {
        av_freep( &m_data[0] );
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
    Copy the internal frame data pointers to the specified AVFrame.

    @note This is intended for quick copyless transfers of frame data - no
    data is not copied only the pointers - hence the AVFrame parameter
    must never be free'd with avframe_free().

    @param frame The frame whos pointers will be modified.
*/
void VideoFrame::FillAvFramePointers( AVFrame& frame ) const
{
    for ( int i = 0; i < AV_NUM_DATA_POINTERS; ++i )
    {
        frame.data[i] = m_data[i];
        frame.extended_data[i] = m_data[i];
        frame.linesize[i] = m_linesize[i];
    }
}

uint8_t** VideoFrame::GetData()
{
    return m_data;
}

const uint8_t* const* VideoFrame::GetData() const
{
    return m_data;
}

const int* VideoFrame::GetLineSize() const
{
    return m_linesize;
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
