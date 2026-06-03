#include "objects/lightray.hpp"
#include "engine.hpp"
#include <cmath>

LightRay2D::LightRay2D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->anchor = true;
}

void LightRay2D::draw_object(SDL_Renderer *renderer, Theme * /*theme*/, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);

    const float cx  = ((left + right) * 0.5f) * (float)w;
    const float cy  = ((top + bottom) * 0.5f) * (float)h;
    const float rad = 6.0f;

    // Small diamond icon to mark the ray origin
    SDL_SetRenderDrawColor(renderer, 255, 255, 200, 220);
    SDL_RenderLine(renderer, (int)(cx),       (int)(cy - rad),
                             (int)(cx + rad), (int)(cy));
    SDL_RenderLine(renderer, (int)(cx + rad), (int)(cy),
                             (int)(cx),       (int)(cy + rad));
    SDL_RenderLine(renderer, (int)(cx),       (int)(cy + rad),
                             (int)(cx - rad), (int)(cy));
    SDL_RenderLine(renderer, (int)(cx - rad), (int)(cy),
                             (int)(cx),       (int)(cy - rad));
    // Arrow pointing down
    SDL_RenderLine(renderer, (int)cx, (int)(cy + rad), (int)cx, (int)(cy + rad + 10));
    SDL_RenderLine(renderer, (int)(cx - 3), (int)(cy + rad + 7), (int)cx, (int)(cy + rad + 10));
    SDL_RenderLine(renderer, (int)(cx + 3), (int)(cy + rad + 7), (int)cx, (int)(cy + rad + 10));
}