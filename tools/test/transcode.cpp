// Copyright (c) 2012 Mark Pupilli, All Rights Reserved.

#include <sys/stat.h>
#include <iostream>
#include <assert.h>
#include <VideoLib.h>

/*
    Read in a video file and write it as an mpeg4 (fourcc=FMP4)
*/
int main( int argc, char** argv )
{
    if ( argc != 3 )
    {
        std::cerr << "Usage: " << argv[0] << " input-file output-file" << std::endl;
        return EXIT_FAILURE;
    }

    // Don't overwrite files:
    struct stat info;
    int err = stat( argv[2], &info );
    if ( err == 0 || errno != ENOENT )
    {
        std::cerr << "File '" << argv[2] << "' already exists - will not overwrite." << std::endl;
        return EXIT_FAILURE;
    }

    LibAvCapture video( argv[1] );
    if ( video.IsOpen() == false )
    {
        std::cerr << "Could not open video from file: " <<  argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    LibAvWriter out( argv[2] );
    out.AddVideoStream( 320, 240, 30, video::FourCc( 'F','M','P','4' ) );

    uint8_t* buffer;
    err = posix_memalign( (void**)&buffer, 16, video.GetFrameWidth() * video.GetFrameHeight() * 3 * sizeof(uint8_t) );
    assert( err == 0 );

    VideoFrame frame( buffer, PIX_FMT_RGB24, video.GetFrameWidth(), video.GetFrameHeight(), video.GetFrameWidth()*3 );

    while ( video.GetFrame() )
    {
        video.DoneFrame();
        video.ExtractRgbImage( buffer, video.GetFrameWidth()*3 );
        out.PutVideoFrame( frame );
    }

    free( buffer );

    return EXIT_SUCCESS;
}

