#ifndef COLOR_HPP

#define COLOR_HPP

#include <cstdint>

namespace Color
{

/*
 * R, G, B, A
*/
struct Color 
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

const Color GREEN      = {0, 255, 0, 255};
const Color RED        = {255, 0, 0, 255};
const Color BLUE       = {0, 0, 255, 255};
const Color WHITE      = {255, 255, 255, 255};
const Color BLACK      = {0, 0, 0, 255};
const Color LIGHT_GRAY = {120, 120, 120, 255};

} // namespace

#endif