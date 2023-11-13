/*
    Copyright (C) Mark Pupilli 2012, All rights reserved

    Classes that enable custom IO with FFmpeg/LibAv.
*/
#ifndef __FFMPEG_CUSTOM_IO_H__
#define __FFMPEG_CUSTOM_IO_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <string>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int fd_write_packet( void* opaque, uint8_t* buffer, int size );
int fd_read_packet( void* opaque, uint8_t* buffer, int size );

} // end extern "C"

/**
    Abstract base class for custom IO objects.
*/
class FFMpegCustomIO
{
public:

    enum BufferType {
        ReadBuffer  = 0,
        WriteBuffer = 1
    };

    FFMpegCustomIO() {};
    virtual ~FFMpegCustomIO() {};
    virtual AVIOContext* GetAVIOContext() = 0;
    virtual const char* GetStreamName() const = 0;
    virtual bool IoError() const = 0; // should return true if there was an IO error
};

/**
    This class implements non-seekable file-io.

    This is only really used for debugging the custom I/O sub-system,
    for regular file I/O you can use standard ffmpeg I/O (e.g. avio_open).
*/
class FFMpegFileIO : public FFMpegCustomIO
{
friend int fd_write_packet( void* opaque, uint8_t* buffer, int size );
friend int fd_read_packet( void* opaque, uint8_t* buffer, int size );

public:
    FFMpegFileIO( const char* filename, bool input );
    virtual ~FFMpegFileIO();
    virtual AVIOContext* GetAVIOContext() { return m_io; };
    virtual const char* GetStreamName() const;
    virtual bool IoError() const;

    static const int BUFFER_SIZE = 32*1024;

private:
    FILE* m_fp;
    uint8_t* m_buffer;
    AVIOContext* m_io;
    std::string m_fileName;
};

#endif /* __FFMPEG_CUSTOM_IO_H__ */

