#ifndef MIRROR_HPP

#define MIRROR_HPP
#include "objects/plane.hpp"

enum MirrorType { FLAT, CONCAVE, CONVEX };

class Mirror : public Plane
{
    public:
    MirrorType type;
    Mirror(MirrorType type, const std::vector<std::array<float,2>> &corners);
};

#endif