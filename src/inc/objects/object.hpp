#ifndef OBJECT_HPP

#define OBJECT_HPP
#include <vector>
#include <array>
#include <SDL3/SDL.h>
#include "color.hpp"
#include "themes.hpp"

enum class HitboxType { RECTANGLE, ELLIPSE };
enum class Orientation { UP, RIGHT, DOWN, LEFT, NONE };
enum class ObjectType { SPRING, MASS, BUTTON, PLANE, LIGHT_SOURCE, WALL, MIRROR, SLIDER, CHECKBOX, TEXTFIELD, TOGGLEBOX };

class Object
{
    public:
    std::vector<std::array<float,2>> corners;
    std::vector<std::array<float,2>> base_shape;
    std::array<float,4>              hitbox{};
    HitboxType                       hitbox_type;
    Orientation                      orientation;
    float                            velocity_x;
    float                            velocity_y;
    bool                             anchor;

    Object(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation);

    bool is_mouse_click(int x, int y, int w, int h);
    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h);
    void create_hitbox();

    void move_object_by_pixels(int dx, int dy, int w, int h);

    virtual void on_property_popup_load(float x, float y, float width, float height);

    bool constrain_object_to_window(float &shift_x, float &shift_y);

    void get_rect_bounds(float &left, float &top, float &right, float &bottom) const;
    void set_rect_from_bounds(float left, float top, float right, float bottom);

    void resize_rect_object_handle(size_t handle_idx, int dx, int dy, int w, int h);
    bool try_get_resize_handle(int mouse_x, int mouse_y, int w, int h, size_t &handle_idx) const;
    std::array<SDL_FPoint, 8> get_resize_handles_px(int w, int h) const;

    void draw_selection_frame(SDL_Renderer *r, int w, int h) const;

    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
};

#endif