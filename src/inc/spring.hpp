#ifndef SPRING_HPP

#define SPRING_HPP
#include "object.hpp"
#include "engine.hpp"

class Spring : public Object
{
    public:
    Spring(const std::vector<std::array<float,2>> &corners, float k_const, bool massless, float mass, Orientation orientation);
    float k_const;
    float mass;
    float velocity;
    bool massless;
    void draw_object(SDL_Renderer *renderer, Theme *theme) override;
};

#endif