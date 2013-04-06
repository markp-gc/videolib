// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#ifndef UNICAP_CAMERA_H
#define UNICAP_CAMERA_H

#include <stdint.h>
#include <string>

extern "C" {
#include <unicap.h>
}

#include "FrameConverter.h"

#include "CameraCapture.h"

/**
    Class for accessing USB cameras through libunicap.

**/
class UnicapCamera : public CameraCapture
{
public:
    UnicapCamera( unsigned long long guid = 0 );
    ~UnicapCamera();

    bool IsOpen() const;
    void StartCapture();
    void StopCapture();

    bool GetFrame();
    void DoneFrame();
    int32_t GetFrameWidth() const;
    int32_t GetFrameHeight() const;
    int64_t GetFrameTimestamp_us() const;

    uint64_t GetGuid() const;
    const char*   GetVendor() const;
    const char*   GetModel() const;

    void ExtractLuminanceImage( uint8_t*, int );
    void ExtractRgbImage( uint8_t*, int );
    void ExtractBgrImage( uint8_t* img, int stride );

    uint8_t* UnsafeBufferAccess() const;

protected:
    void FrameConversion( PixelFormat format, uint8_t* data, int stride );

private:
    bool OpenDevice();
    bool FindFormat( int width, int height, uint32_t fourcc, unicap_format_t& format );
    void EnumerateProperties();
    void SetDefaultProperties();

    static void NewFrame( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t* buffer, void *data );    

    unicap_handle_t m_handle;

    //GLK::ConditionVariable m_cond;
    //GLK::Mutex             m_mutex;
    uint32_t               m_frameCount;
    uint32_t               m_retrievedCount;

    uint8_t*        m_buffer;
    uint32_t        m_width;
    uint32_t        m_height;
    uint64_t        m_guid;
    std::string     m_vendor;
    std::string     m_model;
    volatile uint64_t m_time;

    FrameConverter m_converter;
};

#endif // UNICAP_CAMERA_H

