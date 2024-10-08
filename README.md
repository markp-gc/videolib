# Videolib

This is a fork of only the video processing components from: https://github.com/mpups/development.

The library provides wrappers for encoding and decoding video using `libavcodec`. The key additional
functionality provided by this library, not commonly possible in other libraries, is the ability to give
fine grained control of video streaming so that individual video codec packets can be read/written from
FFMpeg using its custom IO API. Combined with the appropriate communications protocol this allows video
packets to be interleaved with other data on the same link without the video data consuming all the
bandwidth and/or increasing latency of other smaller packets.

## Building

Install dependencies (described below) and then use CMake:

```
git clone https://github.com/markp-gc/videolib
mkdir videolib/build
cd videolib/build
cmake -G Ninja ..
ninja -j32
```

## Installing Dependencies

The key external dependency is a compatible version of FFmpeg (see below). Other dependencies should work with package manager provided versions.

### Ubuntu 18/20/22 Instructions

On these versions of Ubuntu the standard apt packaged version of libavformat (a fork of FFmpeg 3.6) is compatible:

```
sudo apt install libavformat-dev libswscale-dev pkg-config libboost-dev libboost-test-dev
```

However, if you want to use a more recent version of FFMpeg then first install nasm:

```
sudo apt install nasm
```

and then follow the build from source instructions below.


### Mac OSX Instructions

The only tested configuration is FFmpeg 4.4, Mac OSX 11.6.5, Xcode 13.2. First use brew to install pkg-config, nasm, and libx264:

```
brew install pkg-config nasm x264
```

and then follow the build from source instructions below.


### Other Platforms

If you can build and install the correct version of FFMpeg in a place that CMake can find it then it might work...


## FFMpeg Build from Source Instructions

The following should be enough to build and install a compatible version of FFmpeg (note that you will need to have installed
nasm using your system's package manager):

```
git clone https://github.com/FFmpeg/FFmpeg.git
cd FFmpeg
git checkout release/4.4
./configure --enable-shared --enable-libx264 --enable-gpl --disable-programs --enable-rpath
make -j32
make install
```

## Motivation

The library is intended to solve a critical problem with TCP/IP that is not easy to work around. A naive use of FFMpeg
would encode the video directly to a file stream which is then sent over TCP. However, even if this stream is provided
on a separate socket to other data (i.e. video on dedicated TCP socket, smaller packets on separate TCP socket or via UDP)
it is the nature of the TCP protocol to consume as much bandwith as it can only backing off when the link becomes unreliable.
In the use cases this library was intended for we care equally about low or deterministic latency of small packets
(e.g. real-time control input or telemetry to/from a robot) and the throughput of bulk transfers (video). This library
exposes and encodes FFMpeg video packets via a callback mechanism that allows the packets to be easily muxed/demuxed with
other packets from the same byte stream whilst minimising copies of the video data.

For more details of the conflicting requirements between small packets and bulk data transmission over standard protocols see:
- [Analyzing the effect of TCP and server population on massively multiplayer games](https://dl.acm.org/doi/abs/10.1155/2014/602403)
- [Redundant bundling in TCP to reduce perceived latency for time-dependent thin streams](https://ieeexplore.ieee.org/abstract/document/4489685)
