#include "FrameConverter.h"

#include "VideoFrame.h"

/**
    Construct an empty convertor which has no
    context allocated - you must call Configure() to
    setup the conversion before you can call Convert().
*/
FrameConverter::FrameConverter()
:
    m_context ( 0 )
{
}

FrameConverter::~FrameConverter()
{
    if ( m_context != 0 )
    {
        sws_freeContext( m_context );
    }
}

/**
    If no context is allocated a new one is created.
    If the current context is suitable it is left as is.
    If the current context is unsuitable it is freed and
    a new context is created. (I.e. it calls sws_getCachedContext()).

    @return true if the configuration was successful, false
    if there was an error.
*/
bool FrameConverter::Configure( int srcW, int srcH, AVPixelFormat srcFormat,
                                int dstW, int dstH, AVPixelFormat dstFormat )
{
    m_context = sws_getCachedContext( m_context,
                                    srcW, srcH, srcFormat, dstW, dstH, dstFormat,
                                    SWS_FAST_BILINEAR, 0, 0, 0 );

    return m_context != 0;
}

/**
    Perform an image conversion using the internally stored context:

    @param src An array of 4 pointers to planar frame data - for single plane formats the remaining 3 planes must be set to null pointers.
    @param srcStride Strides in bytes for each of the src data planes.
    @param rowOffset The row to begin at in the source data - i.e. 0 for whole image.
    @param rowHeight The number of rows to process (i.e. equal to image height to process whole image).
    @param dst An array of 4 pointers to planar frame storage.
    @param dstStride Strides in bytes for each of the dst data planes.

    @return true if conversion was successful, of false if there was an error.
*/
bool FrameConverter::Convert( uint8_t* src[], int srcStride[], int rowOffset, int rowHeight, uint8_t* dst[], int dstStride[] ) const
{
    if ( m_context != 0 )
    {
        sws_scale( m_context, src, srcStride, rowOffset, rowHeight, dst, dstStride );
    }

    return m_context != 0;
}

bool FrameConverter::Convert( const VideoFrame& frame, uint8_t* dst[], int dstStride[] ) const
{
    if ( m_context != 0 )
    {
        const AVPicture& picture = frame.GetAvPicture();
        sws_scale( m_context, picture.data, picture.linesize, 0, frame.GetHeight(), dst, dstStride );
    }

    return m_context != 0;
}

