#include "objects/decorated_string.hpp"

#include <utility>

namespace {
TTF_Font* open_font_with_fallbacks(const std::string &font_path, int font_size)
{
    const char* fallback_paths[] = {
        font_path.c_str(),
        "assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/Roboto-Bold.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    for(const char* path : fallback_paths)
    {
        if(!path || !*path)
            continue;

        TTF_Font* candidate = TTF_OpenFont(path, font_size);
        if(candidate)
            return candidate;
    }

    return nullptr;
}
}

DecoratedString::DecoratedString(std::string text, std::string font_path, int font_size, SDL_Color color)
    : text(std::move(text)), font_path(std::move(font_path)), font_size(font_size), color(color), font(nullptr)
{
    reload_font();
}

DecoratedString::~DecoratedString()
{
    if(font)
    {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

DecoratedString::DecoratedString(DecoratedString&& other) noexcept
    : text(std::move(other.text)),
      font_path(std::move(other.font_path)),
      font_size(other.font_size),
      color(other.color),
      font(other.font)
{
    other.font = nullptr;
}

DecoratedString& DecoratedString::operator=(DecoratedString&& other) noexcept
{
    if(this == &other)
        return *this;

    if(font)
        TTF_CloseFont(font);

    text = std::move(other.text);
    font_path = std::move(other.font_path);
    font_size = other.font_size;
    color = other.color;
    font = other.font;
    other.font = nullptr;

    return *this;
}

void DecoratedString::reload_font()
{
    if(font)
    {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if(font_path.empty() || font_size <= 0)
        return;

    font = open_font_with_fallbacks(font_path, font_size);
    if(font)
    {
        TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
        TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    }
}

void DecoratedString::set_text(const std::string &new_text)
{
    text = new_text;
}

void DecoratedString::set_font_path(const std::string &new_font_path)
{
    font_path = new_font_path;
    reload_font();
}

void DecoratedString::set_font_size(int new_font_size)
{
    font_size = new_font_size;
    reload_font();
}

void DecoratedString::set_color(SDL_Color new_color)
{
    color = new_color;
}

const std::string& DecoratedString::get_text() const
{
    return text;
}

const std::string& DecoratedString::get_font_path() const
{
    return font_path;
}

int DecoratedString::get_font_size() const
{
    return font_size;
}

SDL_Color DecoratedString::get_color() const
{
    return color;
}

void DecoratedString::draw(SDL_Renderer *renderer, int x, int y, int w, int h) const
{
    if(!font || text.empty())
        return;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if(!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(!texture)
    {
        SDL_DestroySurface(surface);
        return;
    }

    const int text_x = x + (w - surface->w) / 2;
    const int text_y = y + (h - surface->h) / 2;

    SDL_FRect dst = {
        (float)text_x,
        (float)text_y,
        (float)surface->w,
        (float)surface->h
    };

    SDL_RenderTexture(renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}