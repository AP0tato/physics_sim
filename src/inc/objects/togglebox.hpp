#ifndef TOGGLEBOX_HPP
#define TOGGLEBOX_HPP

#include "objects/object.hpp"
#include <functional>
#include <string>
#include <vector>

class ToggleBox : public Object
{
    private:
    SDL_FRect panel;
    std::vector<std::string> values;
    size_t current_index = 0;
    std::string label;
    std::function<void(size_t, const std::string&)> on_change;

    public:
    ToggleBox(const std::vector<std::array<float,2>> &corners,
              HitboxType hitbox_type,
              Orientation orientation,
              const std::vector<std::string> &values,
              size_t initial_index = 0);

    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    void on_property_popup_load(float x, float y, float width, float height) override;

    void set_label(const std::string &text) { label = text; }
    void set_values(const std::vector<std::string> &new_values);
    void set_index(size_t index);
    size_t get_index() const { return current_index; }
    std::string get_value() const;

    void set_on_change(std::function<void(size_t, const std::string&)> callback)
    {
        on_change = std::move(callback);
    }

    bool handle_mouse_down(int mouse_x, int mouse_y);

    ObjectType type() const override { return ObjectType::TOGGLEBOX; }
};

#endif