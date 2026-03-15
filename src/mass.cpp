#include "mass.hpp"

Mass::Mass(const std::vector<std::array<float,2>> &corners, HitboxType shape, float mass)
    : Object(corners, shape, NONE)
{
    this->mass = mass;
}