#ifndef LASER_HPP
#define LASER_HPP

#include "objects/physicsobject.hpp"
#include <string>

class Laser2D : public PhysicsObject2D
{
public:
    // Ray color — stored as RGBA and as a hex string for the text field
    SDL_Color   ray_color     = {255, 50, 50, 255};   // default: red
    std::string ray_color_hex = "FF3232";              // uppercase, no '#'

    Laser2D(const std::vector<std::vector<float>> &corners = {});

    SimDomain  domain() const override { return SimDomain::LIGHT; }
    ObjectType type()   const override { return ObjectType::LASER; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

using Laser = Laser2D;

#endif // LASER_HPP