#include "objects/mass.hpp"

// =============================================================================
// Mass2D
// =============================================================================
Mass2D::Mass2D(const std::vector<std::vector<float>> &corners,
               HitboxType shape, float mass)
    : PhysicsObject2D(corners, shape, Orientation::NONE)
{
    this->mass = mass;
}

// =============================================================================
// Mass3D — skeleton
// =============================================================================
Mass3D::Mass3D(const std::vector<std::vector<float>> &corners,
               HitboxType shape, float mass)
    : PhysicsObject3D(corners, shape, Orientation::NONE)
{
    this->mass = mass;
    // TODO: triangulate corners into faces for 3-D collision.
}