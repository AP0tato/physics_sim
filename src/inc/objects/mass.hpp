#ifndef MASS_HPP
#define MASS_HPP

#include "objects/physicsobject.hpp"

// -----------------------------------------------------------------------------
// Mass2D — rigid point mass in a 2-D simulation.
// -----------------------------------------------------------------------------
class Mass2D : public PhysicsObject2D
{
    public:
    Mass2D(const std::vector<std::vector<float>> &corners,
           HitboxType shape = HitboxType::RECTANGLE,
           float mass = 1.0f);

    SimDomain  domain()    const override { return SimDomain::RIGID; }
    ObjectType type()      const override { return ObjectType::MASS; }
};

// -----------------------------------------------------------------------------
// Mass3D — skeleton, implement when 3-D sim is added.
// -----------------------------------------------------------------------------
class Mass3D : public PhysicsObject3D
{
    public:
    Mass3D(const std::vector<std::vector<float>> &corners,
           HitboxType shape = HitboxType::RECTANGLE,
           float mass = 1.0f);

    SimDomain  domain()    const override { return SimDomain::RIGID; }
    ObjectType type()      const override { return ObjectType::MASS; }
};

// Alias so existing code that says "Mass" still compiles during migration.
using Mass = Mass2D;

#endif