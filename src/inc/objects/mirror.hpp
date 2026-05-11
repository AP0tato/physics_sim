#ifndef MIRROR_HPP

#define MIRROR_HPP
#include "objects/plane.hpp"

#define RESOLUTION 64

enum MirrorType { FLAT, CONCAVE, CONVEX };

class Mirror : public Object
{
    public:
    MirrorType mirror_type;
    Mirror(MirrorType type, const std::vector<std::array<float,2>> &corners);
    Mirror(MirrorType type, int cx, int cy, unsigned int radius, unsigned int deg);
    ObjectType type() const override { return ObjectType::MIRROR; }

    virtual void create_hitbox() override;

    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

#endif