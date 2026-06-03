#ifndef HAMBURGER_BUTTON_HPP
#define HAMBURGER_BUTTON_HPP

#include "button.hpp"

class HamburgerButton : public Button
{
public:
    HamburgerButton(int x, int y, int w, int h, std::function<void()> on_click = {});

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
};

#endif // HAMBURGER_BUTTON_HPP