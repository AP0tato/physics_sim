#ifndef LIGHTRAY_HPP
#define LIGHTRAY_HPP

#include "objects/physicsobject.hpp"

class LightRay2D : public PhysicsObject2D
{
public:
    float angle_deg = 90.0f;   // emission direction in degrees (90 = straight down)
    float intensity = 1.0f;    // brightness multiplier

    LightRay2D(const std::vector<std::vector<float>> &corners = {});

    SimDomain  domain() const override { return SimDomain::LIGHT; }
    ObjectType type()   const override { return ObjectType::LIGHT_RAY; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

using LightRay = LightRay2D;

#endif // LIGHTRAY_HPP