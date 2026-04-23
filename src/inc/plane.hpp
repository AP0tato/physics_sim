#ifndef PLANE_HPP

#define PLANE_HPP

#include "object.hpp"

class Plane : public Object
{
    public:
    Plane(const std::vector<std::array<float,2>> &corners, bool vertical);
    bool vertical;
    bool absorb_enabled;
    float absorb_strength;

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    ObjectType type() const override { return ObjectType::PLANE; }
};

#endif
