#include "objects/laser.hpp"
#include "engine.hpp"

Laser2D::Laser2D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->anchor = true;
}

void Laser2D::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    Object::draw_object(renderer, theme, w, h);
}