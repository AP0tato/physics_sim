#ifndef OBJECT_HPP

#define OBJECT_HPP
#include <vector>
#include <array>
#include <SDL3/SDL.h>
#include "color.hpp"
#include "themes.hpp"

enum class HitboxType { RECTANGLE, ELLIPSE };
enum class Orientation { UP, RIGHT, DOWN, LEFT, NONE };
enum class ObjectType { SPRING, MASS, BUTTON };

class Object
{
    public:
    Object(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation);
    std::vector<std::array<float,2>> hitbox;
    std::vector<std::array<float,2>> corners;
    std::vector<std::array<float,2>> base_shape;
    bool is_mouse_click(int x, int y);
    virtual void draw_object(SDL_Renderer *renderer, Theme *theme);
    void create_hitbox();
    HitboxType hitbox_type;
    Orientation orientation;

    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
};

#endif