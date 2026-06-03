#include "objects/lightray.hpp"
#include "engine.hpp"

LightRay2D::LightRay2D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->anchor = true;
}

void LightRay2D::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    Object::draw_object(renderer, theme, w, h);
}