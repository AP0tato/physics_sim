#ifndef BUTTON_HPP

#define BUTTON_HPP

#include <functional>
#include "object.hpp"
#include "decorated_string.hpp"

class Button : public Object
{
    public:
    Button(int x, int y, int w, int h, std::string text, std::function<void()> on_click);
    std::function<void()> on_press;
    int x;
    int y;
    int w;
    int h;
    Uint64 last_press_ticks = 0;
    void press();
    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    DecoratedString label;
    
    ObjectType type() const override { return ObjectType::BUTTON; }
};

#endif