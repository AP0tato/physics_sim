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
    bool  radial          = false;   // false = linear (fan), true = radial (bulb)
    float intensity       = 1.0f;
    float angle_deg       = 270.0f;  // facing direction (0=right, 90=down, 180=left, 270=up)
    float emission_angle  = 45.0f;   // spread in degrees (linear: fan width; radial: arc width)

    // Ray count — 0 = infinite (continuous)
    int   strength        = 8;

    LightSource2D(const std::vector<std::vector<float>> &corners);

    SimDomain  domain()    const override { return SimDomain::LIGHT; }
    ObjectType type()      const override { return ObjectType::LIGHT_SOURCE; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    void resize_rect_object_handle(size_t handle_idx, int dx, int dy, int w, int h) override;
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