#include "FFmpegStdFunctionIO.h"

#include <assert.h>

/**
    Copy from the user buffer to the av buffer.
    @return the number of bytes copied
*/
int std_function_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegStdFunctionIO& io = *reinterpret_cast<FFMpegStdFunctionIO*>( opaque );
    return io.m_callback( buffer, size );
}

FFMpegStdFunctionIO::FFMpegStdFunctionIO(  FFMpegCustomIO::BufferType direction, callback_t&& callable )
:
    m_callback( callable )
{
    // allocate io context and its buffer:
    AllocateBuffer();
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, (int)direction, this, std_function_packet, std_function_packet, 0 );
    m_io->seekable = 0;
}

FFMpegStdFunctionIO::~FFMpegStdFunctionIO()
{
    if ( m_io->buffer == m_buffer )
    {
        av_free( m_buffer );
    }
    av_free( m_io );
}

AVIOContext* FFMpegStdFunctionIO::GetAVIOContext()
{
    return m_io;
}

const char* FFMpegStdFunctionIO::GetStreamName() const
{
    return "std::function<> callback";
}

/**
    @return true if there was a low level IO error.
*/
bool FFMpegStdFunctionIO::IoError() const
{
    return m_io->error < 0;
}

void FFMpegStdFunctionIO::AllocateBuffer()
{
    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE );
    assert( m_buffer != 0 );
}
