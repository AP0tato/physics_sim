#ifndef BUTTON_HPP

#define BUTTON_HPP

#include <functional>
#include "object.hpp"

class Button : public Object
{
    public:
    Button(int x, int y, int w, int h, std::function<void()> on_click);
    std::function<void()> on_press;
    int x;
    int y;
    int w;
    int h;
    void press();
    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    
    ObjectType type() const override { return ObjectType::BUTTON; }
};

#endif