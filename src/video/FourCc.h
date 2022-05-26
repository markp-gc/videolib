/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef FOURCC_H
#define FOURCC_H

#include <cstdint>
#include <cctype>

namespace video
{

/**
    @return an 32-bit integer representing the four character code (fourcc).

    Lower case characters are converted to upper case before computing the code.
*/
inline std::int32_t FourCc( char c1, char c2, char c3, char c4 )
{
    std::int32_t fourcc = std::toupper(c4);
    fourcc <<= 8;
    fourcc |= toupper(c3);
    fourcc <<= 8;
    fourcc |= toupper(c2);
    fourcc <<= 8;
    fourcc |= toupper(c1);
    return fourcc;
}

} // end namespace video

#endif // FOURCC_H
