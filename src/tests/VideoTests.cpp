#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "../video/FourCc.h"
#include "../video/LibAvWriter.h"
#include "../video/LibAvCapture.h"
#include "../video/FFmpegCustomIO.h"
#include "../video/VideoFrame.h"
#include "../video/FFmpegStdFunctionIO.h"
#include "../../include/platform_memory.h"

#include <string>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

const int FRAME_WIDTH   = 640;
const int FRAME_HEIGHT  = 480;
const int STREAM_WIDTH  = 320;
const int STREAM_HEIGHT = 240;

/**
    Creates a LibAvWriter which writes a test video using the
    specified IO object and codec.
*/
void RunWriter( FFMpegCustomIO& videoIO, int32_t fourcc )
{
    // Write video into memory buffers:
    LibAvWriter writer( videoIO );
    BOOST_CHECK( writer.IsOpen() );

    bool streamCreated = writer.AddVideoStream( STREAM_WIDTH, STREAM_HEIGHT, 30, fourcc );
    BOOST_CHECK( streamCreated );

    uint8_t* buffer = static_cast<uint8_t*>(platform::aligned_alloc(FRAME_WIDTH*FRAME_HEIGHT, 16));
    BOOST_CHECK( buffer != nullptr );

    VideoFrame frame( buffer, AV_PIX_FMT_GRAY8, FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH );

    for ( int i=0;i<256;++i)
    {
        memset( buffer, i, FRAME_WIDTH*FRAME_HEIGHT );
        bool frameWritten = writer.PutVideoFrame( frame );
        BOOST_CHECK( frameWritten );
    }

    platform::aligned_free( buffer );
}

void RunReader( FFMpegCustomIO& videoIn )
{
    uint8_t* buffer = static_cast<uint8_t*>(platform::aligned_alloc(FRAME_WIDTH*FRAME_HEIGHT, 16));
    BOOST_CHECK( buffer != nullptr );

    // Now try to read back the same video
    LibAvCapture reader( videoIn );
    BOOST_CHECK( reader.IsOpen() );

    int decodedCount = 0;
    while ( reader.GetFrame() )
    {
        reader.ExtractLuminanceImage(buffer, FRAME_WIDTH);
        BOOST_CHECK_EQUAL(reader.GetFrameWidth(), STREAM_WIDTH);
        BOOST_CHECK_EQUAL(reader.GetFrameHeight(), STREAM_HEIGHT);

        /// timespec stamp = reader.GetFrameTimestamp();
        /// @todo Implement user settable timestamp and test it here.

        reader.DoneFrame();
        decodedCount += 1;
    }

    BOOST_CHECK_EQUAL( 256, decodedCount );

    platform::aligned_free( buffer );
}

/**
    Test video read/write using FFMpegFileIO.
*/
BOOST_AUTO_TEST_CASE(TestVideo)
{
    // test fourcc code generation:
    int32_t fourcc = video::FourCc( 'y','u','y','v' );
    BOOST_CHECK_EQUAL( fourcc, 0x56595559 );

    FFMpegFileIO videoOut( "test.avi", false );
    RunWriter( videoOut, video::FourCc( 'F','M','P','4' ) );

    FFMpegFileIO videoIn( "test.avi", true );
    RunReader( videoIn );

    // Check unused objects get cleaned up safely:
    char noFile[] = "this_file_should_not_be_created.avi";
    ::remove( noFile );
    {
        LibAvWriter unusedWriter( noFile ); // Construct with default IO otherwise file might be created
        LibAvCapture reader( "test.avi" );
    }

    // Check no file opened after writer was destroyed:
    struct stat info;
    int fileCreated = stat( noFile, &info );
    BOOST_CHECK_EQUAL( -1, fileCreated ); // should return -1 for non existent file
}

BOOST_AUTO_TEST_CASE(TestStdFunctionIO)
{
    using namespace std;
    string testFileName( "ofstream.avi" );
    ofstream outFile( testFileName, ios_base::out | ios_base::binary );

    FFMpegStdFunctionIO videoOut( FFMpegCustomIO::WriteBuffer, [&](uint8_t* buffer, int size){
        outFile.write( reinterpret_cast<char*>(buffer), size );
        return size;
    });

    RunWriter( videoOut, video::FourCc( 'F','M','P','4' ) );

    ifstream inFile( testFileName, ios_base::in | ios_base::binary );
    FFMpegStdFunctionIO videoIn( FFMpegCustomIO::ReadBuffer, [&](uint8_t* buffer, int size){
        streamsize count = inFile.readsome( reinterpret_cast<char*>(buffer), size );
        if ( inFile.good() )
        {
            return static_cast<int>( count );
        }
        else
        {
            return -1;
        }
    });

    RunReader( videoIn );
}

/**
    Test H.264 encoding with fragmented MP4 container.
    This is the configuration used for browser-based MSE video streaming.
*/
BOOST_AUTO_TEST_CASE(TestH264FragmentedMp4)
{
    using namespace std;
    const int numFrames = 64;
    string testFileName( "h264_fmp4_test.mp4" );

    // Write H.264 video into fragmented MP4 via custom IO:
    {
        ofstream outFile( testFileName, ios_base::out | ios_base::binary );
        FFMpegStdFunctionIO videoOut( FFMpegCustomIO::WriteBuffer, [&](uint8_t* buffer, int size){
            outFile.write( reinterpret_cast<char*>(buffer), size );
            return size;
        });

        LibAvWriter writer( videoOut, "mp4", true );
        BOOST_CHECK( writer.IsOpen() );

        bool streamCreated = writer.AddVideoStream( STREAM_WIDTH, STREAM_HEIGHT, 30, video::FourCc( 'H','2','6','4' ) );
        BOOST_CHECK( streamCreated );

        uint8_t* buffer = static_cast<uint8_t*>(platform::aligned_alloc(FRAME_WIDTH*FRAME_HEIGHT, 16));
        BOOST_CHECK( buffer != nullptr );

        VideoFrame frame( buffer, AV_PIX_FMT_GRAY8, FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH );

        int framesWritten = 0;
        for ( int i = 0; i < numFrames; ++i )
        {
            memset( buffer, i, FRAME_WIDTH*FRAME_HEIGHT );
            if ( writer.PutVideoFrame( frame ) )
            {
                framesWritten += 1;
            }
        }

        // With libx264 lookahead some frames are delayed so not all
        // input frames produce immediate output:
        BOOST_CHECK( framesWritten > 0 );
        BOOST_TEST_MESSAGE( "H.264 frames written: " << framesWritten << " / " << numFrames );

        platform::aligned_free( buffer );
    }

    // Read back the video using LibAvCapture to verify it is valid:
    {
        ifstream inFile( testFileName, ios_base::in | ios_base::binary );
        FFMpegStdFunctionIO videoIn( FFMpegCustomIO::ReadBuffer, [&](uint8_t* buffer, int size){
            streamsize count = inFile.readsome( reinterpret_cast<char*>(buffer), size );
            if ( inFile.good() )
            {
                return static_cast<int>( count );
            }
            else
            {
                return -1;
            }
        });

        LibAvCapture reader( videoIn );
        BOOST_CHECK( reader.IsOpen() );

        int decodedCount = 0;
        while ( reader.GetFrame() )
        {
            BOOST_CHECK_EQUAL(reader.GetFrameWidth(), STREAM_WIDTH);
            BOOST_CHECK_EQUAL(reader.GetFrameHeight(), STREAM_HEIGHT);
            reader.DoneFrame();
            decodedCount += 1;
        }

        BOOST_CHECK( decodedCount > 0 );
        BOOST_TEST_MESSAGE( "H.264 frames decoded: " << decodedCount );
    }

    ::remove( testFileName.c_str() );
}
