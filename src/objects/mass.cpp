#include "objects/mass.hpp"

Mass::Mass(const std::vector<std::array<float,2>> &corners, HitboxType shape, float mass)
    : Object(corners, shape, Orientation::NONE)
{
    this->mass = mass;
    this->velocity_x = 0.0f;
    this->velocity_y = 0.0f;
}