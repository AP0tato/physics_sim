#ifndef THEMES_H

#define THEMES_H
#include "color.hpp"

class Theme
{
    public:
    Color background;
    Color foreground;
};

class Light : public Theme
{
    public:
    Light() 
    { 
        background = {.r = 255, .g = 255, .b = 255, .a = 255}; 
        foreground = {.r = 0, .g = 0, .b = 0, .a = 255};
    }
};

class Dark : public Theme
{
    public:
    Dark()
    {
        background = {.r = 0, .g = 0, .b = 0, .a = 255};
        foreground = {.r = 255, .g = 255, .b = 255, .a = 255}; 
    }
};

#endif