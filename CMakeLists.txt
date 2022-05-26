cmake_minimum_required(VERSION 3.20)

project(videolib)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(AVCODEC_LIBRARIES swscale avformat avresample avutil avcodec)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -Werror -Wno-deprecated-declarations)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/src/video/*.hpp ${PROJECT_SOURCE_DIR}/src/video/*.cpp)
add_library(videolib STATIC ${SRC})
target_link_libraries(videolib ${AVCODEC_LIBRARIES})

set(VIDEOLIB_LIBRARIES videolib ${AVCODEC_LIBRARIES} PARENT_SCOPE)
set(VIDEOLIB_INCLUDES ${PROJECT_SOURCE_DIR}/include PARENT_SCOPE)

add_executable(test src/tests/VideoTests.cpp)
target_link_libraries(test videolib ${AVCODEC_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(test_videolib test)