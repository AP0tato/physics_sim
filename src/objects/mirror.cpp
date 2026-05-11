#include "objects/mirror.hpp"

namespace
{
    void draw_curve(SDL_Renderer *renderer);
}

Mirror::Mirror(MirrorType type, const std::vector<std::array<float,2>> &corners)
{
    this->mirror_type = type;
    this->corners = corners;

    if(mirror_type == MirrorType::FLAT)
        this->hitbox_type = HitboxType::RECTANGLE;
    else
        this->hitbox_type = HitboxType::SPECIAL;

    this->create_hitbox();
}

Mirror::Mirror(MirrorType type, int cx, int cy, unsigned int radius, unsigned int deg)
{
    this->mirror_type = type;

    if(mirror_type == MirrorType::FLAT)
    {
        this->hitbox_type = HitboxType::RECTANGLE;
        this->corners = {
            {(float)cx, (float)(cy - radius/2)},
            {(float)(cx + 1), (float)(cy - radius/2)},
            {(float)(cx + 1), (float)(cy + radius/2)},
            {(float)cx, (float)(cy + radius/2)}
        };
    }
    else
    {
        this->hitbox_type = HitboxType::SPECIAL;
    }

    this->create_hitbox();
}

void Mirror::create_hitbox()
{
    if(hitbox_type == HitboxType::SPECIAL)
    {

    }
    else
        Object::create_hitbox();
}

void Mirror::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    if(this->mirror_type == MirrorType::FLAT)
        Object::draw_object(renderer, theme, w, h);
    else
    {

    }
}