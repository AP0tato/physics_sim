#include "objects/checkbox.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <SDL3_ttf/SDL_ttf.h>

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

    void draw_text(SDL_Renderer *renderer, const std::string &text, float x, float y, SDL_Color color, int font_size = 13)
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

CheckBox::CheckBox(int x, int y, int size, bool checked, std::function<void(bool)> on_toggle)
    : Object({{(float)x,(float)y},{(float)x+size,(float)y},{(float)x+size,(float)y+size},{(float)x,(float)y+size}}, HitboxType::RECTANGLE, Orientation::UP), x(x), y(y), size(size), checked(checked), on_toggle(std::move(on_toggle))
{
    
}

CheckBox::CheckBox(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation, bool checked)
    : Object(corners, hitbox_type, orientation), x(0), y(0), size(16), checked(checked), on_toggle(nullptr)
{
}

void CheckBox::set_position(int x_pos, int y_pos)
{
    x = x_pos;
    y = y_pos;
}

void CheckBox::set_size(int new_size)
{
    size = new_size;
}

void CheckBox::set_checked(bool state)
{
    checked = state;
}

void CheckBox::set_label(const std::string &text)
{
    label = text;
}

bool CheckBox::is_checked() const
{
    return checked;
}

bool CheckBox::hit_test(int mouse_x, int mouse_y) const
{
    return (
        mouse_x >= x &&
        mouse_y >= y &&
        mouse_x <= x + size &&
        mouse_y <= y + size
    );
}

void CheckBox::toggle()
{
    checked = !checked;
    if(on_toggle)
        on_toggle(checked);
}

void CheckBox::on_property_popup_load(float x, float y, float width, float height)
{
    (void)width;
    this->x = x;
    this->size = std::max(14, (int)(height * 0.6f));
    this->y = y + (int)((height - (float)this->size) * 0.5f);
}

void CheckBox::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    (void)w;
    (void)h;
    SDL_FRect outer = {(float)x, (float)y, (float)size, (float)size};

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderRect(renderer, &outer);

    if(!label.empty())
    {
        const SDL_Color text_color = theme ? SDL_Color{theme->foreground.r, theme->foreground.g, theme->foreground.b, theme->foreground.a} : SDL_Color{220, 220, 220, 255};

        TTF_Font *font = get_font(13);
        float text_y = outer.y;
        if(font)
        {
            int tw = 0, th = 0;
            if(TTF_GetStringSize(font, label.c_str(), label.length(), &tw, &th))
                text_y = outer.y + (outer.h - (float)th) * 0.5f;
        }

        draw_text(renderer, label, outer.x + outer.w + 8.0f, text_y, text_color, 13);
    }

    if(checked)
    {
        SDL_FRect inner = {
            (float)x + 4.0f,
            (float)y + 4.0f,
            (float)size - 8.0f,
            (float)size - 8.0f
        };
        SDL_RenderFillRect(renderer, &inner);
    }
}
