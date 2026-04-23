#ifndef MASS_HPP

#define MASS_HPP
#include "object.hpp"

class Mass : public Object
{
    public:
    Mass(const std::vector<std::array<float,2>> &corners, HitboxType shape, float mass);
    float mass;

    ObjectType type() const override { return ObjectType::MASS; }
};

#endif
