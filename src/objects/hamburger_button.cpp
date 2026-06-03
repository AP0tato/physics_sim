#include "objects/hamburger_button.hpp"

HamburgerButton::HamburgerButton(int x, int y, int w, int h, std::function<void()> on_click)
    : Button(x, y, w, h, "", std::move(on_click))
{
}

void HamburgerButton::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    Button::draw_object(renderer, theme, w, h);

    const SDL_Color c = {50, 50, 50, 255};
    const float left = (float)x + (float)this->w * 0.2f;
    const float right = (float)x + (float)this->w * 0.8f;
    const float y1 = (float)y + (float)this->h * 0.28f;
    const float y2 = (float)y + (float)this->h * 0.50f;
    const float y3 = (float)y + (float)this->h * 0.72f;

    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderLine(renderer, left, y1, right, y1);
    SDL_RenderLine(renderer, left, y2, right, y2);
    SDL_RenderLine(renderer, left, y3, right, y3);
}