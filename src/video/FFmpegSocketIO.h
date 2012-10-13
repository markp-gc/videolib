/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FFMPEG_SOCKET_IO_H__
#define __FFMPEG_SOCKET_IO_H__

#include "FFmpegCustomIO.h"

#include <string>

class TcpSocket;

extern "C" {

int socket_write_packet( void* opaque, uint8_t* buffer, int size );
int socket_read_packet( void* opaque, uint8_t* buffer, int size );

} // end extern "C"

/**
    Class which allows streaming of video packets
    over a TCP socket connection.
*/
class FFMpegSocketIO : public FFMpegCustomIO
{
friend int socket_write_packet( void* opaque, uint8_t* buffer, int size );
friend int socket_read_packet( void* opaque, uint8_t* buffer, int size );

public:
    FFMpegSocketIO( TcpSocket& socket, bool sender );
    virtual ~FFMpegSocketIO();
    virtual AVIOContext* GetAVIOContext();
    virtual const char* GetStreamName() const;
    virtual bool IoError() const;

    uint64_t BytesRead() const;
    uint64_t BytesWritten() const;

protected:
    static const int BUFFER_SIZE = 32*1024;

private:
    std::string RetrievePeerName() const;

    uint8_t* m_buffer;
    AVIOContext* m_io;
    TcpSocket& m_socket;
    std::string m_peerName;

    uint64_t m_bytesTx;
    uint64_t m_bytesRx;
};

#endif /* __FFMPEG_SOCKET_IO_H__ */

