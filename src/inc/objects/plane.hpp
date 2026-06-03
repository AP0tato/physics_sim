#ifndef PLANE_HPP
#define PLANE_HPP

#include "objects/physicsobject.hpp"

// -----------------------------------------------------------------------------
// Plane2D — infinite thin wall / boundary in a 2-D simulation.
// -----------------------------------------------------------------------------
class Plane2D : public PhysicsObject2D
{
    public:
    bool  vertical;
    bool  absorb_enabled;
    float absorb_strength;

    Plane2D(const std::vector<std::vector<float>> &corners, bool vertical = false);

    SimDomain  domain()    const override { return SimDomain::RIGID; }
    ObjectType type()      const override { return ObjectType::PLANE; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

// -----------------------------------------------------------------------------
// Plane3D — skeleton, implement when 3-D sim is added.
// -----------------------------------------------------------------------------
class Plane3D : public PhysicsObject3D
{
    public:
    Plane3D(const std::vector<std::vector<float>> &corners);

    SimDomain  domain()    const override { return SimDomain::RIGID; }
    ObjectType type()      const override { return ObjectType::PLANE; }

    // TODO: draw_object, face normal, collision response.
};

using Plane = Plane2D;

#endif