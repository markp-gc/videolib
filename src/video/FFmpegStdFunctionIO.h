/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FFMPEG_STD_FUNCTION_IO_H__
#define __FFMPEG_STD_FUNCTION_IO_H__

#include "FFmpegCustomIO.h"

#include <functional>

/**
    This will call a std::function callback with data to be read or written.
*/
class FFMpegStdFunctionIO : public FFMpegCustomIO
{
friend int std_function_packet( void* opaque, uint8_t* buffer, int size );

public:
    typedef const std::function< int( uint8_t* buffer, int size ) > callback_t;

    FFMpegStdFunctionIO( FFMpegCustomIO::BufferType direction, callback_t&& callable );
    virtual ~FFMpegStdFunctionIO();

    virtual AVIOContext* GetAVIOContext();
    virtual const char* GetStreamName() const;
    virtual bool IoError() const;

    static constexpr int BUFFER_SIZE = (32*1024);// + AV_INPUT_BUFFER_PADDING_SIZE;

protected:
    void AllocateBuffer();

private:
    AVIOContext* m_io;
    uint8_t* m_buffer;

    callback_t m_callback;
};

#endif /* __FFMPEG_STD_FUNCTION_IO_H__ */

