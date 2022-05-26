/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __VIDEO_CLIENT_H__
#define __VIDEO_CLIENT_H__

#include <VideoLib.h>

#include "../packetcomms/PacketComms.h"

#include <memory>
#include <cinttypes>

class PacketDemuxer;

/**
    Class for managing a video stream received over the muxer comms system.

    In the constructor a subscription is made to AvData packets which are
    simply enqued as they are received.


*/
class VideoClient
{
public:
    VideoClient( PacketDemuxer& demuxer );
    virtual ~VideoClient();

    bool InitialiseVideoStream( const std::chrono::seconds& videoTimeout );
    int GetFrameWidth() const { return m_streamer->GetFrameWidth(); };
    int GetFrameHeight() const { return m_streamer->GetFrameHeight(); };

    bool ReceiveVideoFrame( std::function< void(LibAvCapture&) > );

    double ComputeVideoBandwidthConsumed( double seconds );

protected:
    bool StreamerOk() const;
    bool StreamerIoError() const;
    int  ReadPacket( uint8_t* buffer, int size );

private:
    SimpleQueue m_avInfoPackets;
    SimpleQueue m_avDataPackets;
    int         m_packetOffset;
    uint64_t    m_lastTotalVideoBytes;
    uint64_t    m_totalVideoBytes;
    PacketSubscription m_avDataSubscription;

    std::unique_ptr<FFMpegStdFunctionIO> m_videoIO;
    std::unique_ptr<LibAvCapture> m_streamer;

    void ResetAvTimeout();
    bool AvHasTimedOut();
    std::chrono::steady_clock::time_point m_avDataTimeoutPoint;
    std::chrono::seconds m_avTimeout;
};

#endif /* __VIDEO_CLIENT_H__ */

