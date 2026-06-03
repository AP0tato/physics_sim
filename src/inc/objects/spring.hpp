#ifndef SPRING_HPP
#define SPRING_HPP

#include "objects/physicsobject.hpp"
#include "engine.hpp"

class Mass2D;
class Mass3D;

struct AttachedObject2D
{
    Mass2D              *mass   = nullptr;
    std::array<float, 2> offset = {0.0f, 0.0f};
};

struct AttachedObject3D
{
    Mass3D              *mass   = nullptr;
    std::array<float, 3> offset = {0.0f, 0.0f, 0.0f};
};

// -----------------------------------------------------------------------------
// Spring2D — massless or massed spring in a 2-D simulation.
// -----------------------------------------------------------------------------
class Spring2D : public PhysicsObject2D
{
    public:
    float  k_const;
    float  equilibrium_pos_px;
    float  deformation_px;
    bool   massless;
    std::vector<AttachedObject2D> attached_objects;

    Spring2D(const std::vector<std::vector<float>> &corners,
             float k_const = 1.0f, bool massless = false,
             float mass = 1.0f, Orientation orientation = Orientation::NONE);

    bool  is_mass_attached(const Mass2D *mass_obj) const;
    void  attach_mass(Mass2D *mass_obj, const std::array<float, 2> &offset);
    void  detach_mass(Mass2D *mass_obj);
    float attached_mass_total() const;

    SimDomain  domain()    const override { return SimDomain::RIGID; }
    ObjectType type()      const override { return ObjectType::SPRING; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

// -----------------------------------------------------------------------------
// Spring3D — skeleton, implement when 3-D sim is added.
// -----------------------------------------------------------------------------
class Spring3D : public PhysicsObject3D
{
    public:
    float  k_const;
    float  equilibrium_pos_px;
    float  deformation_px;
    bool   massless;
    std::vector<AttachedObject3D> attached_objects;

    Spring3D(const std::vector<std::vector<float>> &corners,
             float k_const = 1.0f, bool massless = false,
             float mass = 1.0f, Orientation orientation = Orientation::NONE);

    SimDomain  domain()    const override { return SimDomain::RIGID; }
    ObjectType type()      const override { return ObjectType::SPRING; }

    // TODO: attach_mass, detach_mass, draw_object, force application in 3D.
};

using Spring        = Spring2D;
using AttachedObject = AttachedObject2D;

#endif