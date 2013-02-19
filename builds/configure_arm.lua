-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'beagle_build_g++4.6'

ARM_DEPLOYMENT = '/home/mark/custom_bb_kernel/deploy'
TOOLCHAIN = 'arm-linux-gnueabi-'
LIB_TYPE = 'SharedLib'

INCLUDE_DIRS = {
    '../../glk/include',
    '../../videolib/include',
    ARM_DEPLOYMENT .. '/include',
    ARM_DEPLOYMENT .. '/include/unicap',
    ARM_DEPLOYMENT .. '/include/freetype2',
    '/home/mark/custom_bb_kernel/packages/lua-5.1.4/src'
}

LIB_DIRS = {
    '../../glk/premake/beagle_build_g++4.6/debug',
    '../../glk/premake/beagle_build_g++4.6/release',
    ARM_DEPLOYMENT .. '/lib', '/home/mark/custom_bb_kernel/packages/lua-5.1.4/src'
}

DEFINES = { 'ARM_BUILD','__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }
BUILD_OPTIONS = { '-std=c++0x -static -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp --sysroot=' .. ARM_DEPLOYMENT }
LINK_OPTIONS = { '-Wl,--allow-shlib-undefined,-rpath=/usr/local/lib:' .. ARM_DEPLOYMENT }

OPENCV_LINKS = { 'opencv_core', 'opencv_contrib' }
OPENGL_LINKS = {}
SYSTEM_LINKS = { 'pthread', 'rt', 'dl' }
FFMPEG_LINKS = { 'avformat', 'avcodec', 'avutil', 'swscale' }
LINKS = {}

GLK_LINKS = { 'glkcore', 'lua', 'freetype' }
VIDEO_LINKS = { 'videolib','unicap' }

CONFIGURING_ARM = true
