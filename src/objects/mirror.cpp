#include "objects/mirror.hpp"

// =============================================================================
// Mirror2D
// =============================================================================
Mirror2D::Mirror2D(MirrorType type, const std::vector<std::vector<float>> &corners)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->mirror_type = type;
    this->corners     = corners;
    this->hitbox_type = (mirror_type == MirrorType::FLAT) ? HitboxType::RECTANGLE
                                                           : HitboxType::SPECIAL;
    this->create_hitbox();
}

Mirror2D::Mirror2D(MirrorType type, int cx, int cy, unsigned int radius, unsigned int deg)
    : PhysicsObject2D({}, HitboxType::RECTANGLE, Orientation::NONE)
{
    (void)deg;
    this->mirror_type = type;

    if(mirror_type == MirrorType::FLAT)
    {
        this->hitbox_type = HitboxType::RECTANGLE;
        this->corners = {
            {(float)cx,       (float)(cy - radius/2)},
            {(float)(cx + 1), (float)(cy - radius/2)},
            {(float)(cx + 1), (float)(cy + radius/2)},
            {(float)cx,       (float)(cy + radius/2)}
        };
    }
    else
    {
        this->hitbox_type = HitboxType::SPECIAL;
    }

    this->create_hitbox();
    this->base_shape = this->corners;
}

void Mirror2D::create_hitbox()
{
    if(hitbox_type == HitboxType::SPECIAL)
    {
        // TODO: bounding box for curved mirrors.
    }
    else
        Object::create_hitbox();
}

void Mirror2D::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    if(this->mirror_type == MirrorType::FLAT)
        Object::draw_object(renderer, theme, w, h);
    else
    {
        // TODO: draw curved mirror geometry.
    }
}

// =============================================================================
// Mirror3D — skeleton
// =============================================================================
Mirror3D::Mirror3D(MirrorType type, const std::vector<std::vector<float>> &corners)
    : PhysicsObject3D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->mirror_type = type;
    // TODO: triangulate corners into faces, compute surface normal.
}