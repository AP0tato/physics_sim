#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <SDL3/SDL.h>
#include <iostream>
#include "color.hpp"
#include "ntree.hpp"
#include "sim_enums.hpp"

class PhysicsObject;

namespace Engine
{
    // =========================================================================
    // Broad phase — called before any narrow check
    // =========================================================================
    bool aabb_overlap(float min_x1, float min_y1,
                      float max_x1, float max_y1,
                      float min_x2, float min_y2,
                      float max_x2, float max_y2);

    bool aabb_overlap(float min_x1, float min_y1, float min_z1,
                      float max_x1, float max_y1, float max_z1,
                      float min_x2, float min_y2, float min_z2,
                      float max_x2, float max_y2, float max_z2);

    // =========================================================================
    // Unified entry point.
    // Performs an AABB reject first, then dispatches to the correct narrow-phase
    // based on the objects' declared SimDimension and SimDomain.
    // Both objects must share the same dimension; mismatches return false.
    // =========================================================================
    bool is_collision(PhysicsObject *a, PhysicsObject *b);

    // =========================================================================
    // 2D narrow-phase checks (one per domain)
    // =========================================================================

    // Ray vs edge: parametric intersection, both params in [0,1].
    bool is_light_collision_2d(PhysicsObject *a, PhysicsObject *b);

    // Edge-edge contact for rigid polygon bodies.
    bool is_rigid_collision_2d(PhysicsObject *a, PhysicsObject *b);

    // Particle proximity / overlap for 2-D fluid regions.
    bool is_fluid_collision_2d(PhysicsObject *a, PhysicsObject *b);

    // =========================================================================
    // 3D narrow-phase checks (skeletons — implement when 3D sim is added)
    // =========================================================================

    // Ray vs triangulated face (Möller–Trumbore).
    bool is_light_collision_3d(PhysicsObject *a, PhysicsObject *b);

    // Edge-edge + vertex-face contact manifold for rigid bodies.
    bool is_rigid_collision_3d(PhysicsObject *a, PhysicsObject *b);

    // SPH particle or voxel-grid proximity for 3-D fluids.
    bool is_fluid_collision_3d(PhysicsObject *a, PhysicsObject *b);

    // =========================================================================
    // Rendering helpers
    // =========================================================================
    void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2,
                   Color::Color *color);
}

#endif