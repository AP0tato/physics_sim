#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "objects/physicsobject.hpp"

// -----------------------------------------------------------------------------
// LightSource2D — emits rays in a 2-D light simulation.
// -----------------------------------------------------------------------------
class LightSource2D : public PhysicsObject2D
{
    public:
    // Emission mode
    bool  radial          = false;   // false = linear (flat), true = radial (bulb)
    float intensity       = 1.0f;
    float emission_angle  = 360.0f;  // degrees; used when radial = true

    // Ray count — 0 = infinite (continuous)
    int   strength        = 8;

    LightSource2D(const std::vector<std::vector<float>> &corners);

    SimDomain  domain()    const override { return SimDomain::LIGHT; }
    ObjectType type()      const override { return ObjectType::LIGHT_SOURCE; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

// -----------------------------------------------------------------------------
// LightSource3D — skeleton, implement when 3-D light sim is added.
// -----------------------------------------------------------------------------
class LightSource3D : public PhysicsObject3D
{
    public:
    bool  radial    = false;
    float intensity = 1.0f;

    LightSource3D(const std::vector<std::vector<float>> &corners);

    SimDomain  domain()    const override { return SimDomain::LIGHT; }
    ObjectType type()      const override { return ObjectType::LIGHT_SOURCE; }

    // TODO: solid-angle emission, wavelength.
};

using LightSource = LightSource2D;

#endif