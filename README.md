# Videolib

This is a fork of only the video processing components from: https://github.com/mpups/development.

The library provides wrappers for encoding and decoding video using `libavcodec`. The key additional
functionality provided by this library, not commonly possible in other libraries, is the ability to give
fine grained control of video streaming so that individual video codec packets can be interleaved with
other data on the same link without the video data consuming all the bandwidth and/or increasing latency
of other smaller packets.

This solves a critical problem with TCP/IP that is not easy to work around. For example, even if the video
was provided on a separate socket (e.g video on dedicated TCP socket, smaller packets on separate TCP socket
or via UDP) it is the nature of the TCP protocol to consume as much bandwith as it can, only backing off when
the link becomes unreliable. Effectively the TCP protocol was not designed to play nice and share this is why,
for example, game stores will pause or throttle downloads while you are gaming. However, in the use case this
library was intended for we care equally about low or deterministic latency of small packets (e.g. real-time
control data or telemetry) and maintaining the throughput of bulk transfers (video). For details of the
conflicting requirements between small packets and bulk data transmission over standard protocols see:
- [Analyzing the effect of TCP and server population on massively multiplayer games](https://dl.acm.org/doi/abs/10.1155/2014/602403)
- [Redundant bundling in TCP to reduce perceived latency for time-dependent thin streams](https://ieeexplore.ieee.org/abstract/document/4489685)

## Building

Install dependencies (described below) and then use CMake:

```
mkdir build
cd build
cmake -G Ninja ..
ninja -j32
```

## Installing Dependencies

The only library dependency is a compatible version of FFmpeg version.

### Ubuntu 18/20

On these versions of Ubuntu the standard apt packaged version of libavformat (fork of FFmpeg 3.6) is compatible:

```
sudo apt install libavformat-dev
```

### Mac OSX

The only tested configuration is FFmpeg 4.0, Mac OSX 11.6.5, Xcode 13.2.

The following should be enough to build and install a compatible version of FFmpeg (note that you will need brew):

```
git clone https://github.com/FFmpeg/FFmpeg.git
cd FFmpeg
git checkout release/4.0
brew install nasm
./configure --enable-shared --enable-small --extra-cflags='-O3'  --enable-gpl
make -j16
make install
```

### Other Platforms

If you can build and install the correct version of FFMpeg in a place that CMake can find it then it might work...