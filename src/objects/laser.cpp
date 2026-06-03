#include "objects/laser.hpp"
#include "engine.hpp"
#include <cmath>

Laser2D::Laser2D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->anchor = true;
}

void Laser2D::draw_object(SDL_Renderer *renderer, Theme * /*theme*/, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);

    const float lx0 = left  * (float)w;
    const float lx1 = right * (float)w;
    const float ty  = top   * (float)h;
    const float by  = bottom * (float)h;
    const float mid = (ty + by) * 0.5f;

    // Body outline
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 230);
    SDL_FRect body = { lx0, ty, lx1 - lx0, by - ty };
    SDL_RenderFillRect(renderer, &body);

    // Emitter aperture (front, top edge) in ray color
    SDL_SetRenderDrawColor(renderer, ray_color.r, ray_color.g, ray_color.b, 255);
    SDL_RenderLine(renderer, (int)lx0, (int)ty, (int)lx1, (int)ty);

    // Centre stripe
    SDL_SetRenderDrawColor(renderer, ray_color.r, ray_color.g, ray_color.b, 140);
    SDL_RenderLine(renderer, (int)lx0, (int)mid, (int)lx1, (int)mid);

    // Outline
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderLine(renderer, (int)lx0, (int)ty, (int)lx0, (int)by);
    SDL_RenderLine(renderer, (int)lx1, (int)ty, (int)lx1, (int)by);
    SDL_RenderLine(renderer, (int)lx0, (int)by, (int)lx1, (int)by);
}