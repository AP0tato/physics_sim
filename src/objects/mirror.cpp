#include "objects/mirror.hpp"

Mirror::Mirror(MirrorType type, const std::vector<std::array<float,2>> &corners) : Plane(corners, true)
{
    this->type = type;
}