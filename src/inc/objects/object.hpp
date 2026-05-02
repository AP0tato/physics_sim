#ifndef OBJECT_HPP

#define OBJECT_HPP
#include <vector>
#include <array>
#include <SDL3/SDL.h>
#include "color.hpp"
#include "themes.hpp"

enum class HitboxType { RECTANGLE, ELLIPSE };
enum class Orientation { UP, RIGHT, DOWN, LEFT, NONE };
enum class ObjectType { SPRING, MASS, BUTTON, PLANE, LIGHT_SOURCE, WALL, MIRROR };

class Object
{
    public:
    Object(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation);
    std::array<float,4> hitbox{};
    std::vector<std::array<float,2>> corners;
    std::vector<std::array<float,2>> base_shape;

    bool is_mouse_click(int x, int y, int w, int h);
    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h);
    void create_hitbox();

    void move_object_by_pixels(int dx, int dy, int w, int h);
    bool constrain_object_to_window(float &shift_x, float &shift_y);
    void get_rect_bounds(float &left, float &top, float &right, float &bottom) const;

    HitboxType hitbox_type;
    Orientation orientation;
    float velocity_x;
    float velocity_y;
    bool anchor;

    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
};

#endif