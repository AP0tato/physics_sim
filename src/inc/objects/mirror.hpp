#ifndef MIRROR_HPP
#define MIRROR_HPP

#include "objects/physicsobject.hpp"

#define RESOLUTION 64

enum MirrorType { FLAT, CONCAVE, CONVEX };

// -----------------------------------------------------------------------------
// Mirror2D — flat / curved reflector in a 2-D light simulation.
// -----------------------------------------------------------------------------
class Mirror2D : public PhysicsObject2D
{
    public:
    MirrorType mirror_type;
    float      concavity = 0.5f;  // 0 = flat, 1 = maximally curved; used for CONCAVE/CONVEX

    Mirror2D(MirrorType type = MirrorType::FLAT,
             const std::vector<std::vector<float>> &corners = {});
    Mirror2D(MirrorType type, int cx, int cy, unsigned int radius, unsigned int deg = 0);

    SimDomain  domain()    const override { return SimDomain::LIGHT; }
    ObjectType type()      const override { return ObjectType::MIRROR; }

    void create_hitbox() override;
    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

// -----------------------------------------------------------------------------
// Mirror3D — skeleton, implement when 3-D light sim is added.
// -----------------------------------------------------------------------------
class Mirror3D : public PhysicsObject3D
{
    public:
    MirrorType mirror_type;

    Mirror3D(MirrorType type = MirrorType::FLAT,
             const std::vector<std::vector<float>> &corners = {});

    SimDomain  domain()    const override { return SimDomain::LIGHT; }
    ObjectType type()      const override { return ObjectType::MIRROR; }

    // TODO: create_hitbox (triangulate), draw_object, reflection normal.
};

using Mirror = Mirror2D;

#endif