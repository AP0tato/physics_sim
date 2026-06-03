#ifndef LASER_HPP
#define LASER_HPP

#include "objects/physicsobject.hpp"
#include <string>

class Laser2D : public PhysicsObject2D
{
public:
    SDL_Color   ray_color     = {255, 50, 50, 255};
    std::string ray_color_hex = "FF3232";   // uppercase, no '#'
    float       angle_deg     = 90.0f;      // emission direction (90 = straight down)
    float       intensity     = 1.0f;       // brightness multiplier

    Laser2D(const std::vector<std::vector<float>> &corners = {});

    SimDomain  domain() const override { return SimDomain::LIGHT; }
    ObjectType type()   const override { return ObjectType::LASER; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

using Laser = Laser2D;

#endif // LASER_HPP