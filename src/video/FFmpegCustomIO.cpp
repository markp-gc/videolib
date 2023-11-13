#include "FFmpegCustomIO.h"

//#include <iostream>
#include <assert.h>

int fd_write_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegFileIO* file = reinterpret_cast<FFMpegFileIO*>( opaque );
    int fd = fileno( file->m_fp );
    //std::cerr << "socket_write_packet: " << size << " bytes" << std::endl;
    return write( fd, buffer, size );
}

int fd_read_packet( void* opaque, uint8_t* buffer, int size )
{
    FFMpegFileIO* file = reinterpret_cast<FFMpegFileIO*>( opaque );
    int fd = fileno( file->m_fp );

    int numBytes = read( fd, buffer, size );

    if ( numBytes == 0 && feof(file->m_fp) )
    {
        numBytes =  -1;
    }

    //std::cerr << "fd_read_packet: bytes requested :=" << size << " bytes, received := " << numBytes << std::endl;
    return numBytes;
}

/**
    Construct an object for non-seekable file-IO.

    The file is opened, an io buffer is allocated, and an
    appropriate AVIOContext is allocated. You can retrieve
    the context using the GetAVIOContext() method.

    @param filename name of the file on disk.
    @param input true if this is an input (readable) context, false if it is an output (writeable) context.
*/
FFMpegFileIO::FFMpegFileIO( const char* filename, bool input )
:
    m_buffer (0),
    m_io     (0)
{
    int writable;
    if ( input )
    {
        writable = 0;
        m_fp = fopen( filename, "rb" );
    }
    else
    {
        writable = 1;
        m_fp = fopen( filename, "wb" );
    }

    m_fileName = filename;

    m_buffer = (uint8_t*)av_malloc( BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE );
    assert( m_buffer != 0 );
    m_io = avio_alloc_context( m_buffer, BUFFER_SIZE, writable, this, fd_read_packet, fd_write_packet, 0 );
    m_io->seekable = 0;
}

/**
    Frees the AVIOContext and io-buffer, and closes the file.
*/
FFMpegFileIO::~FFMpegFileIO()
{
    av_free( m_io );
    //@todo need to free this but it currently causes a crash (possibly due to ffmpeg bug)
    //av_free( m_buffer );
    fclose( m_fp );
}

const char* FFMpegFileIO::GetStreamName() const
{
    return m_fileName.c_str();
}

/**
    @return true if there was a low level IO error.
*/
bool FFMpegFileIO::IoError() const
{
    return m_io->error < 0;
}

