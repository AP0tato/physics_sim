#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include <SDL3/SDL.h>
#include <functional>

class CheckBox
{
    public:
    CheckBox();
    CheckBox(int x, int y, int size, bool checked, std::function<void(bool)> on_toggle = {});

    void set_position(int x, int y);
    void set_size(int size);
    void set_checked(bool checked);
    bool is_checked() const;

    bool hit_test(int mouse_x, int mouse_y) const;
    void toggle();
    void draw(SDL_Renderer *renderer) const;

    private:
    int x = 0;
    int y = 0;
    int size = 16;
    bool checked = false;
    std::function<void(bool)> on_toggle;
};

#endif
