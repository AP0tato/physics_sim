#include "objects/togglebox.hpp"

#include <SDL3_ttf/SDL_ttf.h>
#include <map>

namespace
{
    TTF_Font *get_font(int size)
    {
        static std::map<int, TTF_Font*> cache;
        auto it = cache.find(size);
        if(it != cache.end())
            return it->second;

        static const char *paths[] = {
            "assets/fonts/Roboto-Regular.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/System/Library/Fonts/Supplemental/Helvetica.ttc",
            "C:/Windows/Fonts/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            nullptr
        };

        TTF_Font *font = nullptr;
        for(int i = 0; paths[i]; ++i)
        {
            font = TTF_OpenFont(paths[i], size);
            if(font) break;
        }

        cache[size] = font;
        return font;
    }

    void draw_text(SDL_Renderer *renderer, const std::string &text, float x, float y, SDL_Color color, int font_size = 11)
    {
        TTF_Font *font = get_font(font_size);
        if(!font)
            return;

        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
        if(!surface)
            return;

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if(texture)
        {
            SDL_FRect dst = {x, y, (float)surface->w, (float)surface->h};
            SDL_RenderTexture(renderer, texture, nullptr, &dst);
            SDL_DestroyTexture(texture);
        }

        SDL_DestroySurface(surface);
    }
}

ToggleBox::ToggleBox(const std::vector<std::array<float,2>> &corners,
                     HitboxType hitbox_type,
                     Orientation orientation,
                     const std::vector<std::string> &values,
                     size_t initial_index)
    : Object(corners, hitbox_type, orientation), panel({0, 0, 0, 0}), values(values), current_index(0)
{
    set_index(initial_index);
}

void ToggleBox::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    (void)w;
    (void)h;
    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(renderer, 120, 220, 255, 255);
    SDL_RenderRect(renderer, &panel);

    const SDL_Color text_color = theme ? SDL_Color{theme->foreground.r, theme->foreground.g, theme->foreground.b, theme->foreground.a} : SDL_Color{220, 220, 220, 255};

    std::string text = label;
    if(!text.empty())
        text += ": ";
    text += get_value();

    draw_text(renderer, text, panel.x + 8.0f, panel.y + 7.0f, text_color, 11);
}

void ToggleBox::on_property_popup_load(float x, float y, float width, float height)
{
    panel = {x, y, width, height};
}

void ToggleBox::set_values(const std::vector<std::string> &new_values)
{
    values = new_values;
    if(values.empty())
        current_index = 0;
    else if(current_index >= values.size())
        current_index = values.size() - 1;
}

void ToggleBox::set_index(size_t index)
{
    if(values.empty())
    {
        current_index = 0;
        return;
    }
    current_index = index % values.size();
}

std::string ToggleBox::get_value() const
{
    if(values.empty())
        return "-";
    return values[current_index];
}

bool ToggleBox::handle_mouse_down(int mouse_x, int mouse_y)
{
    const bool hit = mouse_x >= (int)panel.x && mouse_x <= (int)(panel.x + panel.w) &&
                     mouse_y >= (int)panel.y && mouse_y <= (int)(panel.y + panel.h);
    if(!hit)
        return false;

    if(!values.empty())
    {
        current_index = (current_index + 1) % values.size();
        if(on_change)
            on_change(current_index, values[current_index]);
    }
    return true;
}
