#include "button.hpp"

#include <utility>

Button::Button(int x, int y, int w, int h, std::function<void()> on_click)
    : Object( std::vector<std::array<float,2>>{{(float)x, (float)y}, {(float)x+w, (float)y}, {(float)x+w, (float)y+h}, {(float)x, (float)y+h}}, HitboxType::RECTANGLE, Orientation::NONE)
{
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->on_press = std::move(on_click);
}

void Button::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    Color *fg = &theme->foreground;
    SDL_FRect btn = {(float)this->x, (float)this->y, (float)this->w, (float)this->h };
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