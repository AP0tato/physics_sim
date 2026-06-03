#ifndef OBJECT_HPP

#define OBJECT_HPP
#include <vector>
#include <array>
#include <SDL3/SDL.h>
#include <iostream>
#include "color.hpp"
#include "themes.hpp"

enum class HitboxType { RECTANGLE, ELLIPSE, SPECIAL };
enum class Orientation { UP, RIGHT, DOWN, LEFT, NONE };
enum class ObjectType { SPRING, MASS, BUTTON, PLANE, LIGHT_SOURCE, WALL, MIRROR, LASER, LIGHT_RAY, SLIDER, CHECKBOX, TEXTFIELD, TOGGLEBOX };

class Object
{
    public:
    std::vector<std::vector<float>>           corners;
    std::vector<std::vector<float>>           base_shape;

    // For RECTANGLE / SPECIAL: hitbox mirrors corners — each entry is a vertex
    // {x, y, z} so edge iteration works directly on hitbox.
    // For ELLIPSE: exactly 2 entries — hitbox[0] = {cx, cy, 0},
    //                                   hitbox[1] = {rx, ry, 0}.
    std::vector<std::array<float, 3>>         hitbox{{std::array<float, 3>{0.0f, 0.0f, 0.0f}, std::array<float, 3>{0.0f, 0.0f, 0.0f}}};

    HitboxType                       hitbox_type;
    Orientation                      orientation;
    bool                             anchor;

    Object() = default;
    Object(const std::vector<std::vector<float>> &corners, HitboxType hitbox_type, Orientation orientation);

    virtual bool is_mouse_click(int x, int y, int w, int h);
    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h);
    virtual void create_hitbox();

    void move_object_by_pixels(int dx, int dy, int w, int h);

    virtual void on_property_popup_load(float x, float y, float width, float height);

    bool constrain_object_to_window(float &shift_x, float &shift_y);

    void get_rect_bounds(float &left, float &top, float &right, float &bottom) const;
    void set_rect_from_bounds(float left, float top, float right, float bottom);

    virtual void resize_rect_object_handle(size_t handle_idx, int dx, int dy, int w, int h);
    bool try_get_resize_handle(int mouse_x, int mouse_y, int w, int h, size_t &handle_idx) const;
    std::array<SDL_FPoint, 8> get_resize_handles_px(int w, int h) const;

    void draw_selection_frame(SDL_Renderer *r, int w, int h) const;

    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
};

#endif