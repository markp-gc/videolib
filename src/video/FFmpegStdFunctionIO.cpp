#include "FFmpegStdFunctionIO.h"

#include <assert.h>

/**
    Copy from the user buffer to the av buffer.
    @return the number of bytes copied
*/
int std_function_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegStdFunctionIO& io = *reinterpret_cast<FFMpegStdFunctionIO*>( opaque );
    return io.m_readCallback( buffer, size );
}

int std_function_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegStdFunctionIO& io = *reinterpret_cast<FFMpegStdFunctionIO*>( opaque );
    return io.m_writeCallback( buffer, size );
}

FFMpegStdFunctionIO::FFMpegStdFunctionIO( ReadCallbackTag, read_callback_t&& callable )
:
    m_readCallback( callable )
{
    // allocate io context and its buffer:
    AllocateBuffer();
    int (*read_callback)(void*, uint8_t*, int) = nullptr;
    int (*write_callback)(void*, uint8_t*, int) = nullptr;
    read_callback = std_function_read_packet;

    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, 0, this, read_callback, write_callback, 0 );
    m_io->seekable = 0;
}

FFMpegStdFunctionIO::FFMpegStdFunctionIO( WriteCallbackTag, write_callback_t&& callable )
:
    m_writeCallback( callable )
{
    // allocate io context and its buffer:
    AllocateBuffer();
    int (*read_callback)(void*, uint8_t*, int) = nullptr;
    int (*write_callback)(void*, uint8_t*, int) = nullptr;
    write_callback = std_function_write_packet;

    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, 1, this, read_callback, write_callback, 0 );
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
