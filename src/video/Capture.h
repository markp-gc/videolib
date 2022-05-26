/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include <time.h>

/**
    Abstract base for all types of capture (live or from files).
*/
class Capture
{
public:
    Capture() {};
    virtual ~Capture() {};

    virtual bool IsOpen() const = 0;

    virtual bool GetFrame() = 0;
    virtual void DoneFrame() = 0;
    virtual int32_t GetFrameWidth() const = 0;
    virtual int32_t GetFrameHeight() const = 0;
    virtual timespec GetFrameTimestamp() const = 0;

    virtual void ExtractLuminanceImage( uint8_t*, int stride ) = 0;
    virtual void ExtractRgbImage( uint8_t*, int stride ) = 0;
    virtual void ExtractBgrImage( uint8_t*, int stride ) = 0;
};

#endif /* __CAPTURE_H__ */
