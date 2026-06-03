#ifndef TEXTFIELD_HPP

#define TEXTFIELD_HPP

#include "objects/object.hpp"
#include <SDL3/SDL.h>
#include <functional>
#include <string>

class TextField : public Object
{
    private:
    SDL_FRect   panel;
    std::string value;
    std::string label;
    bool        active;
    std::function<void(const std::string&)> on_commit;

    public:
    TextField(const std::vector<std::vector<float>> &corners, HitboxType hitbox_type, Orientation orientation, const std::string &value = "");

    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    void on_property_popup_load(float x, float y, float width, float height) override;
    ObjectType type() const override { return ObjectType::TEXTFIELD; }

    void set_label(const std::string &text) { label = text; }
    void set_value(const std::string &text) { value = text; }
    const std::string& get_value() const    { return value; }
    void set_on_commit(std::function<void(const std::string&)> cb) { on_commit = std::move(cb); }

    // Call when the user confirms input (Enter key or focus-loss)
    void commit() { if(on_commit) on_commit(value); }

    // Append a character (used by keyboard event handler)
    void append_char(char c) { value += c; }
    void backspace()         { if(!value.empty()) value.pop_back(); }

    bool is_active() const   { return active; }
    void set_active(bool a)  { active = a; }
    bool hit_test(int x, int y) const {
        return x >= (int)panel.x && x <= (int)(panel.x+panel.w) &&
               y >= (int)panel.y && y <= (int)(panel.y+panel.h);
    }
};

#endif