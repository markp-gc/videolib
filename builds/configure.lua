-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '/usr/local/glk/include',
    '/usr/include/lua5.1',
    '/usr/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/usr/include/opencv'
}

LIB_DIRS = {
    '/usr/local/glk/lib'
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = { '-Wl,-rpath,/usr/local/glk/lib' }

OPENCV_LINKS = { 'ml', 'cvaux','highgui', 'cv', 'cxcore' }
OPENGL_LINKS = { 'Xrender', 'X11', 'GL' }
SYSTEM_LINKS = { 'pthread', 'rt' }
LINKS = { 'lua5.1', 'freetype', 'unicap' }
FFMPEG_LINKS = { 'avcodec', 'avformat', 'avutil', 'swscale' }

GLK_LINKS = { 'glk', 'glkcore' }

-- need these defines because ffmpeg libs use C99 standard macros which are not in any C++ standards.
DEFINES = { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' }

