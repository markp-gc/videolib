#ifndef VIDEO_LIB_H
#define VIDEO_LIB_H

#include "../src/video/CameraCapture.h"

#include "../src/video/FourCc.h"
#include "../src/video/video_conversion.h"
#include "../src/video/VideoFrame.h"
#include "../src/video/LibAvCapture.h"
#include "../src/video/LibAvWriter.h"
#include "../src/video/FFmpegCustomIO.h"
#include "../src/video/FFmpegStdFunctionIO.h"

#ifndef ANDROID

#include "../src/video/UnicapCamera.h"
#include "../src/video/UnicapCapture.h"

#ifndef ARM_BUILD
// These components are not ready to be used on
// ARM systems yet:

#include "../src/video/Dc1394Camera.h"

#endif // ARM_BUILD

#endif // ANDROID

#endif // VIDEO_LIB_H

