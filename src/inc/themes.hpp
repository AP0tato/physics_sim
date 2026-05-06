#ifndef THEMES_H

#define THEMES_H
#include "color.hpp"

class Theme
{
    public:
    Color::Color background;
    Color::Color foreground;
    Color::Color border;
    Color::Color active;
};

class Light : public Theme
{
    public:
    Light() 
    { 
        background = Color::WHITE; 
        foreground = Color::BLACK;
        border = foreground;
        active = Color::LIGHT_GRAY;
    }
};

class Dark : public Theme
{
    public:
    Dark()
    {
        background = Color::BLACK;
        foreground = Color::WHITE; 
        border = foreground;
        active = Color::LIGHT_GRAY;
    }
};

#endif