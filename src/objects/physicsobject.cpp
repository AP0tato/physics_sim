#include "objects/physicsobject.hpp"

// =============================================================================
// PhysicsObject (shared base)
// =============================================================================
PhysicsObject::PhysicsObject(const std::vector<std::vector<float>> &corners,
                             HitboxType hitbox_type, Orientation orientation)
    : Object(corners, hitbox_type, orientation)
{
    velocity             = {0.0f, 0.0f, 0.0f};
    acceleration         = {0.0f, 0.0f, 0.0f};
    mass                 = 1.0f;
    restitution          = 0.5f;
    friction             = 0.0f;
    affected_by_gravity  = true;
}

float PhysicsObject::speed() const
{
    return sqrtf(velocity[0]*velocity[0] +
                 velocity[1]*velocity[1] +
                 velocity[2]*velocity[2]);
}

void PhysicsObject::set_velocity(float x, float y, float z)
{
    velocity = {x, y, z};
}

void PhysicsObject::set_acceleration(float x, float y, float z)
{
    acceleration = {x, y, z};
}

void PhysicsObject::reset_motion()
{
    velocity     = {0.0f, 0.0f, 0.0f};
    acceleration = {0.0f, 0.0f, 0.0f};
}

// =============================================================================
// PhysicsObject2D
// =============================================================================
PhysicsObject2D::PhysicsObject2D(const std::vector<std::vector<float>> &corners,
                                 HitboxType hitbox_type, Orientation orientation)
    : PhysicsObject(corners, hitbox_type, orientation)
{}

// =============================================================================
// PhysicsObject3D
// =============================================================================
PhysicsObject3D::PhysicsObject3D(const std::vector<std::vector<float>> &corners,
                                 HitboxType hitbox_type, Orientation orientation)
    : PhysicsObject(corners, hitbox_type, orientation)
{}