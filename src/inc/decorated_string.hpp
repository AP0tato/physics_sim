#ifndef DECORATED_STRING_HPP

#define DECORATED_STRING_HPP

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL.h>
#include <string>
#include <utility>

class DecoratedString
{
    public:
    DecoratedString(std::string text, std::string font_path, int font_size, SDL_Color color);
    ~DecoratedString();

    DecoratedString(const DecoratedString&) = delete;
    DecoratedString& operator=(const DecoratedString&) = delete;
    DecoratedString(DecoratedString&& other) noexcept;
    DecoratedString& operator=(DecoratedString&& other) noexcept;

    void set_text(const std::string &text);
    void set_font_path(const std::string &font_path);
    void set_font_size(int font_size);
    void set_color(SDL_Color color);

    const std::string& get_text() const;
    const std::string& get_font_path() const;
    int get_font_size() const;
    SDL_Color get_color() const;

    void draw(SDL_Renderer *renderer, int x, int y, int w, int h) const;

    private:
    void reload_font();

    std::string text;
    std::string font_path;
    int font_size;
    SDL_Color color;
    TTF_Font *font;
};

#endif