#include "objects/textfield.hpp"

TextField::TextField(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation, const std::string &value)
    : Object(corners, hitbox_type, orientation), value(value), active(false), panel({0, 0, 0, 0})
{
}

void TextField::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    SDL_SetRenderDrawColor(renderer, theme->background.r, theme->background.g, theme->background.b, theme->background.a);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, active ? theme->active.r : 140, active ? theme->active.g : 140, active ? theme->active.b : 140, active ? theme->active.a : 255);
    SDL_RenderRect(renderer, &panel);
}

void TextField::on_property_popup_load(float x, float y, float width, float height) 
{
    panel = {x, y, width, height};
}