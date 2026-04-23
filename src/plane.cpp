#include "plane.hpp"

#include "engine.hpp"

#include <algorithm>

Plane::Plane(const std::vector<std::array<float,2>> &corners, bool vertical)
    : Object(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->vertical = vertical;
    this->absorb_enabled = false;
    this->absorb_strength = 1.0f;
    this->anchor = true;
}

void Plane::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    float left = corners[0][0];
    float right = corners[0][0];
    float top = corners[0][1];
    float bottom = corners[0][1];

    for(const auto &corner : corners)
    {
        left = std::min(left, corner[0]);
        right = std::max(right, corner[0]);
        top = std::min(top, corner[1]);
        bottom = std::max(bottom, corner[1]);
    }

    if(vertical)
    {
        const int x = (int)(((left + right) * 0.5f) * w);
        draw_line(renderer, x, (int)(top * h), x, (int)(bottom * h), &theme->foreground);
    }
    else
    {
        const int y = (int)(((top + bottom) * 0.5f) * h);
        draw_line(renderer, (int)(left * w), y, (int)(right * w), y, &theme->foreground);
    }
}
