#include "button.hpp"

Button::Button(int x, int y, int w, int h, std::function<void()> &on_click)
    : Object( std::vector<std::array<float,2>>{{(float)x, (float)y}, {(float)x+w, (float)y}, {(float)x+w, (float)y+h}, {(float)x, (float)y+h}}, RECTANGLE, NONE)
{
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->on_press = on_click;
}

void Button::draw_object(SDL_Renderer *renderer, Theme *theme)
{
    Color *fg = &theme->foreground;
    SDL_FRect btn = {(float)x, (float)y, (float)w, (float)h };
    SDL_SetRenderDrawColor(renderer, fg->r, fg->g, fg->b, fg->a);
    SDL_RenderFillRect(renderer, &btn);
}

void Button::press()
{
    if(on_press)
    {
        on_press();
    }
}