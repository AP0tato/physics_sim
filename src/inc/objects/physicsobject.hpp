#ifndef PHYSICSOBJECT_HPP
#define PHYSICSOBJECT_HPP

#include <array>
#include "objects/object.hpp"
#include "sim_enums.hpp"
#include <math.h>

// -----------------------------------------------------------------------------
// PhysicsObject — shared base for all physics-capable objects regardless of
// dimension or domain.  Stores motion state and material properties only.
// -----------------------------------------------------------------------------
class PhysicsObject : public Object
{
    public:
    std::array<float, 3> velocity;
    std::array<float, 3> acceleration;
    float                mass;
    float                restitution;
    float                friction;
    bool                 affected_by_gravity;

    // Every concrete physics object declares which domain it belongs to so the
    // engine can dispatch to the right narrow-phase check.
    virtual SimDomain    domain()    const = 0;
    virtual SimDimension dimension() const = 0;

    PhysicsObject(const std::vector<std::vector<float>> &corners,
                  HitboxType  hitbox_type = HitboxType::RECTANGLE,
                  Orientation orientation = Orientation::NONE);

    void  set_velocity(float x = 0.0f, float y = 0.0f, float z = 0.0f);
    void  set_acceleration(float x = 0.0f, float y = 0.0f, float z = 0.0f);
    void  reset_motion();
    float speed() const;
};

// -----------------------------------------------------------------------------
// PhysicsObject2D — base for all objects that live in a 2-D simulation.
// Concrete 2-D types (Mass, Plane, Mirror, LightSource …) inherit from this.
// The z-components of velocity / acceleration are always 0 and unused.
// -----------------------------------------------------------------------------
class PhysicsObject2D : public PhysicsObject
{
    public:
    PhysicsObject2D(const std::vector<std::vector<float>> &corners,
                    HitboxType  hitbox_type = HitboxType::RECTANGLE,
                    Orientation orientation = Orientation::NONE);

    SimDimension dimension() const override { return SimDimension::DIM_2D; }
    // domain() is still pure — each concrete 2-D type implements it.
};

// -----------------------------------------------------------------------------
// PhysicsObject3D — skeleton base for future 3-D objects.
// Adds face topology needed for 3-D collision (ray-vs-face, rigid manifold).
// -----------------------------------------------------------------------------
class PhysicsObject3D : public PhysicsObject
{
    public:
    // Each face is a list of indices into corners (triangulated or arbitrary).
    // Populate this in the constructor of each concrete 3-D subclass.
    std::vector<std::array<size_t, 3>> faces; // triangles only for now

    PhysicsObject3D(const std::vector<std::vector<float>> &corners,
                    HitboxType  hitbox_type = HitboxType::RECTANGLE,
                    Orientation orientation = Orientation::NONE);

    SimDimension dimension() const override { return SimDimension::DIM_3D; }
    // domain() is still pure — each concrete 3-D type implements it.
};

#endif