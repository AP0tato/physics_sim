#ifndef LIGHTRAY_HPP
#define LIGHTRAY_HPP

#include "objects/physicsobject.hpp"

class LightRay2D : public PhysicsObject2D
{
public:
    LightRay2D(const std::vector<std::vector<float>> &corners = {});

    SimDomain  domain() const override { return SimDomain::LIGHT; }
    ObjectType type()   const override { return ObjectType::LIGHT_RAY; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

using LightRay = LightRay2D;

#endif // LIGHTRAY_HPP