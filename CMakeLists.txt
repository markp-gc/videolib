cmake_minimum_required(VERSION 3.16)

project(videolib)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

message(STATUS "INC: ${LIBAV_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -Werror -Wno-deprecated-declarations)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(${LIBAV_INCLUDE_DIRS})

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/src/video/*.hpp ${PROJECT_SOURCE_DIR}/src/video/*.cpp)
add_library(videolib STATIC ${SRC})
target_link_libraries(videolib PkgConfig::LIBAV)

set(VIDEOLIB_LIBRARIES videolib PARENT_SCOPE)
set(VIDEOLIB_INCLUDES ${PROJECT_SOURCE_DIR}/include PARENT_SCOPE)

add_executable(video_test src/tests/VideoTests.cpp)
target_link_libraries(video_test videolib PkgConfig::LIBAV ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(test_videolib video_test)
