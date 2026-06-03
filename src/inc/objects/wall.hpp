#ifndef WALL_HPP
#define WALL_HPP

#include "objects/physicsobject.hpp"

class Wall2D : public PhysicsObject2D
{
public:
    Wall2D(const std::vector<std::vector<float>> &corners = {}, bool vertical = false);

    SimDomain  domain() const override { return SimDomain::LIGHT; }
    ObjectType type()   const override { return ObjectType::WALL; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;

private:
    bool vertical = false;
};

using Wall = Wall2D;

#endif // WALL_HPP