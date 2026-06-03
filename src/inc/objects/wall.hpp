#ifndef WALL_HPP
#define WALL_HPP

#include "objects/physicsobject.hpp"

class Wall2D : public PhysicsObject2D
{
public:
    float opacity        = 1.0f;   // 0 = transparent (pass-through), 1 = fully opaque
    float absorb_strength = 1.0f;  // fraction of ray intensity absorbed on hit

    Wall2D(const std::vector<std::vector<float>> &corners = {}, bool vertical = false);

    SimDomain  domain() const override { return SimDomain::LIGHT; }
    ObjectType type()   const override { return ObjectType::WALL; }

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    bool is_mouse_click(int x, int y, int w, int h) override;
    void resize_rect_object_handle(size_t handle_idx, int dx, int dy, int w, int h) override;

    bool is_vertical() const { return vertical; }
    void set_vertical(bool v) { vertical = v; }

private:
    bool vertical = false;
};

using Wall = Wall2D;

#endif // WALL_HPP