#ifndef SLIDER_HPP

#define SLIDER_HPP
#include "objects/object.hpp"
#include <functional>
#include <string>

class Slider : public Object
{
    private:
    SDL_FRect slider;
    float     min_v;
    float     max_v;
    float     value;
    bool      dragging = false;
    std::string label;
    std::function<void(float)> on_change;

    public:
    Slider(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation);
    Slider(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation, float min_v, float max_v, float value);

    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    void on_property_popup_load(float x, float y, float width, float height) override;
    float slider_x_from_value(float value, float min_val, float max_val);
    float slider_value_from_mouse(int mouse_x, float min_val, float max_val);
    
    void set_label(const std::string &text) { label = text; }
    void set_range(float min_v, float max_v) { this->min_v = min_v; this->max_v = max_v; }
    void set_value(float v) { value = v; }
    void set_on_change(std::function<void(float)> callback) { on_change = std::move(callback); }
    float get_value() const { return value; }
    float get_min() const { return min_v; }
    float get_max() const { return max_v; }
    bool handle_mouse_down(int mouse_x, int mouse_y);
    bool handle_mouse_motion(int mouse_x);
    void handle_mouse_up();
    
    ObjectType type() const override { return ObjectType::SLIDER; }
};

#endif
