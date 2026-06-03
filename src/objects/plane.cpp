#include "objects/plane.hpp"
#include "engine.hpp"
#include <algorithm>

// =============================================================================
// Plane2D
// =============================================================================
Plane2D::Plane2D(const std::vector<std::vector<float>> &corners, bool vertical)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->vertical        = vertical;
    this->absorb_enabled  = false;
    this->absorb_strength = 1.0f;
    this->anchor          = true;
}

void Plane2D::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    float left   = corners[0][0], right  = corners[0][0];
    float top    = corners[0][1], bottom = corners[0][1];

    for(const auto &corner : corners)
    {
        left   = std::min(left,   corner[0]);
        right  = std::max(right,  corner[0]);
        top    = std::min(top,    corner[1]);
        bottom = std::max(bottom, corner[1]);
    }

    if(vertical)
    {
        const int x = (int)(((left + right) * 0.5f) * w);
        Engine::draw_line(renderer, x, (int)(top * h), x, (int)(bottom * h), &theme->foreground);
    }
    else
    {
        const int y = (int)(((top + bottom) * 0.5f) * h);
        Engine::draw_line(renderer, (int)(left * w), y, (int)(right * w), y, &theme->foreground);
    }
}

// =============================================================================
// Plane3D — skeleton
// =============================================================================
Plane3D::Plane3D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject3D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->anchor = true;
    // TODO: triangulate corners into faces.
}