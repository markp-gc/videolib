-- You can change these variables so that
-- they are appropriate for your system:

TARGET_DIR   = 'linux_build'

INCLUDE_DIRS = {
    '/usr/local/glk/include',
    '../../include', -- Robolib includes.
    '/usr/include/lua5.1',
    '/usr/include/freetype2',
    '/usr/include/gtest',
    '/usr/include/',
    '/usr/include/unicap',
    '/usr/include/opencv',
    '/usr/include/dc1394',
}

LIB_DIRS = {
    '/usr/lib',
    '/usr/local/lib',
    '/usr/local/glk/lib',
    '../../builds/linux_build', -- robolib libraries built here
}

BUILD_OPTIONS = { '-msse -msse2' }
LINK_OPTIONS = { '-Wl,-rpath,/usr/local/glk/lib' }
LINKS = { 'lua5.1', 'freetype', 'pthread', 'rt', 'cxcore', 'cvaux', 'unicap' }
