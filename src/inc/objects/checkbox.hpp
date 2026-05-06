#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include <SDL3/SDL.h>
#include <functional>
#include <iostream>
#include <string>
#include "objects/object.hpp"

class CheckBox : public Object
{
    public:
    CheckBox(int x, int y, int size, bool checked, std::function<void(bool)> on_toggle = {});
    CheckBox(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation, bool checked = false);

    void set_position(int x, int y);
    void set_size(int size);
    void set_checked(bool checked);
    bool is_checked() const;

    bool hit_test(int mouse_x, int mouse_y) const;
    void toggle();
    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    void on_property_popup_load(float x, float y, float width, float height) override;
    void set_label(const std::string &text);
    void set_on_toggle(std::function<void(bool)> callback) { on_toggle = std::move(callback); }
    ObjectType type() const override { return ObjectType::CHECKBOX; }

    private:
    int x = 0;
    int y = 0;
    int size = 16;
    bool checked = false;
    std::string label;
    std::function<void(bool)> on_toggle;
};

#endif
