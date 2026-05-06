#include "objects/slider.hpp"

#include <map>
#include <cstdio>
#include <string>
#include <SDL3_ttf/SDL_ttf.h>

namespace
{
    float clamp_value(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

    std::string format_value(float value)
    {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f", value);
        return buf;
    }

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

Slider::Slider(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation)
    : Object(corners, hitbox_type, orientation), slider({0, 0, 0, 0}), min_v(0.0f), max_v(100.0f), value(50.0f)
{
}

Slider::Slider(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation, float min_v, float max_v, float value)
    : Object(corners, hitbox_type, orientation), slider({0, 0, 0, 0}), min_v(min_v), max_v(max_v), value(value)
{
}

void Slider::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    const SDL_Color text_color = theme ? SDL_Color{theme->foreground.r, theme->foreground.g, theme->foreground.b, theme->foreground.a} : SDL_Color{220, 220, 220, 255};
    const float current_x = slider_x_from_value(value, min_v, max_v);

    std::string title = label.empty() ? "slider" : label;
    title += "  ";
    title += format_value(value);
    title += " [";
    title += format_value(min_v);
    title += " - ";
    title += format_value(max_v);
    title += "]";
    draw_text(renderer, title, slider.x, slider.y - 24.0f, text_color, 13);

    SDL_SetRenderDrawColor(renderer, 95, 95, 95, 255);
    SDL_RenderFillRect(renderer, &slider);
    SDL_SetRenderDrawColor(renderer, 40, 200, 255, 255);
    SDL_FRect filled = slider;
    filled.w = current_x - slider.x;
    if(filled.w < 0.0f) filled.w = 0.0f;
    SDL_RenderFillRect(renderer, &filled);
    SDL_FRect thumb = {current_x - 4.0f, slider.y - 4.0f, 8.0f, slider.h + 8.0f};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderFillRect(renderer, &thumb);
}

float Slider::slider_x_from_value(float value, float min_val, float max_val)
{
    const float span = max_val - min_val;
    if(span <= 0.0f) return slider.x;
    const float t = clamp_value((value - min_val) / span, 0.0f, 1.0f);
    return slider.x + t * slider.w;
}

float Slider::slider_value_from_mouse(int mouse_x, float min_val, float max_val)
{
    if(slider.w <= 1.0f) return min_val;
    const float t = clamp_value(((float)mouse_x - slider.x) / slider.w, 0.0f, 1.0f);
    return min_val + (max_val - min_val) * t;
}

void Slider::on_property_popup_load(float x, float y, float width, float height)
{
    slider = {x, y + height * 0.5f, width, 8.0f};
}

bool Slider::handle_mouse_down(int mouse_x, int mouse_y)
{
    const SDL_FRect hit_rect = {slider.x, slider.y - 8.0f, slider.w, slider.h + 16.0f};
    if(mouse_x < (int)hit_rect.x || mouse_x > (int)(hit_rect.x + hit_rect.w) ||
       mouse_y < (int)hit_rect.y || mouse_y > (int)(hit_rect.y + hit_rect.h))
        return false;

    dragging = true;
    value = slider_value_from_mouse(mouse_x, min_v, max_v);
    if(on_change)
        on_change(value);
    return true;
}

bool Slider::handle_mouse_motion(int mouse_x)
{
    if(!dragging)
        return false;

    value = slider_value_from_mouse(mouse_x, min_v, max_v);
    if(on_change)
        on_change(value);
    return true;
}

void Slider::handle_mouse_up()
{
    dragging = false;
}