#include "../../video/FourCc.h"
#include "../../video/LibAvWriter.h"
#include "../../video/LibAvCapture.h"
#include "../../video/FFmpegCustomIO.h"
#include "../../video/VideoFrame.h"
#include "../../video/FFmpegStdFunctionIO.h"

#include <gtest/gtest.h>

#include <string>
#include <iostream>
#include <fstream>

const int FRAME_WIDTH   = 640;
const int FRAME_HEIGHT  = 480;
const int STREAM_WIDTH  = 320;
const int STREAM_HEIGHT = 240;

/**
    Creates a LibAvWriter which writes a test video using the
    specified IO object.
*/
void RunWriter( FFMpegCustomIO& videoIO )
{
    // Write video into memory buffers:
    LibAvWriter writer( videoIO );
    ASSERT_TRUE( writer.IsOpen() );

    bool streamCreated = writer.AddVideoStream( STREAM_WIDTH, STREAM_HEIGHT, 30, video::FourCc( 'F','M','P','4' ) );
    ASSERT_TRUE( streamCreated );

    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, 640*480 );
    ASSERT_EQ( 0, err );

    VideoFrame frame( buffer, AV_PIX_FMT_GRAY8, FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH );

    for ( int i=0;i<256;++i)
    {
        memset( buffer, i, FRAME_WIDTH*FRAME_HEIGHT );
        bool frameWritten = writer.PutVideoFrame( frame );
        ASSERT_TRUE( frameWritten );
    }

    free( buffer );
}

void RunReader( FFMpegCustomIO& videoIn )
{
    uint8_t* buffer;
    int err = posix_memalign( (void**)&buffer, 16, FRAME_WIDTH*FRAME_HEIGHT );
    ASSERT_EQ( 0, err );

    // Now try to read back the same video
    LibAvCapture reader( videoIn );
    ASSERT_TRUE( reader.IsOpen() );

    int decodedCount = 0;
    while ( reader.GetFrame() )
    {
        reader.ExtractLuminanceImage( buffer, 640 );
        EXPECT_EQ( buffer[0], decodedCount );

        /// timespec stamp = reader.GetFrameTimestamp();
        /// @todo Implement user settable timestamp and test it here.

        reader.DoneFrame();
        decodedCount += 1;
    }

    EXPECT_EQ( 256, decodedCount );

    free( buffer );
}

/**
    Test video read/write using FFMpegFileIO.
*/
void TestVideo()
{
    // test fourcc code generation:
    int32_t fourcc = video::FourCc( 'y','u','y','v' );
    EXPECT_EQ( fourcc, 0x56595559 );

    FFMpegFileIO videoOut( "test.avi", false );
    RunWriter( videoOut );

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
    EXPECT_EQ( -1, fileCreated ); // should return -1 for non existent file
}

void TestStdFunctionIO()
{
    using namespace std;
    string testFileName( "ofstream.avi" );
    ofstream outFile( testFileName, ios_base::out | ios_base::binary );

    FFMpegStdFunctionIO videoOut( FFMpegCustomIO::WriteBuffer, [&](uint8_t* buffer, int size){
        outFile.write( reinterpret_cast<char*>(buffer), size );
        return size;
    });

    RunWriter( videoOut );

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

TEST( video, file )
{
    TestVideo();
}

TEST( video, stdfunction )
{
    TestStdFunctionIO();
}

