#include "FFmpegBufferIO.h"

#include <assert.h>

#include <iostream>
#include <fstream>

/**
    Copy from the user buffer to the av buffer.
    @return the number of bytes copied
*/
int buffer_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegBufferIO& io = *reinterpret_cast<FFMpegBufferIO*>( opaque );

    FFMpegBufferIO::Buffer tmp;
    tmp.size = size;
    tmp.data = (uint8_t*)av_malloc( tmp.size );
    memcpy( tmp.data, buffer, tmp.size );
    io.m_buffers.push_back( tmp );

    //std::cout << "write_packet: " << size << " bytes" << std::endl;
    return size;
}

/**
    Copy from the av buffer to the user buffer.
    @return the number of bytes copied
*/
int buffer_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegBufferIO& io = *reinterpret_cast<FFMpegBufferIO*>( opaque );

    if ( io.m_buffers.size() == 0 )
    {
        return -1;
    }

    // Lets just try returning contents of 1 buffer at a time and rely on libav to ask for more:
    FFMpegBufferIO::Buffer tmp = io.m_buffers.front();

    if ( size < tmp.size )
    {
        memcpy( buffer, tmp.data, size );

        size_t remainder = tmp.size - size;
        memmove( tmp.data, tmp.data+size, remainder );
        tmp.size = remainder;

        return size;
    }

    io.m_buffers.pop_front();
    memcpy( buffer, tmp.data, tmp.size );

    av_free( tmp.data );
    return tmp.size;
}

FFMpegBufferIO::FFMpegBufferIO( BufferType direction )
{
    // allocate io context and its buffer:
    AllocateBuffer();
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, (int)direction, this, buffer_read_packet, buffer_write_packet, 0 );
    m_io->seekable = 0;
}

FFMpegBufferIO::~FFMpegBufferIO()
{
    av_free( m_io );
//    av_free( m_buffer );
}

void FFMpegBufferIO::ChangeDirection( BufferType direction )
{
    av_free( m_buffer );
    av_free( m_io );

    AllocateBuffer();
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, (int)direction, this, buffer_read_packet, buffer_write_packet, 0 );
    m_io->seekable = 0;
    m_readIndex = 0;
}

AVIOContext* FFMpegBufferIO::GetAVIOContext()
{
    return m_io;
}

const char* FFMpegBufferIO::GetStreamName() const
{
    return "User-Buffer-IO";
}

/**
    @return true if there was a low level IO error.
*/
bool FFMpegBufferIO::IoError() const
{
    return m_io->error < 0;
}

void FFMpegBufferIO::AllocateBuffer()
{
    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE + 2*FF_INPUT_BUFFER_PADDING_SIZE );
    assert( m_buffer != 0 );
}

void FFMpegBufferIO::DumpToFile( const char* fileName ) const
{
    std::fstream file( fileName, std::fstream::out | std::fstream::binary );

    for ( std::deque<FFMpegBufferIO::Buffer>::const_iterator itr = m_buffers.begin(); itr != m_buffers.end(); ++itr )
    {
        file.write( reinterpret_cast<char*>(itr->data), itr->size );
    }
}

