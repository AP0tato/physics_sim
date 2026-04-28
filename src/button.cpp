#include "button.hpp"

#include <utility>

Button::Button(int x, int y, int w, int h, std::string text, std::function<void()> on_click)
    : Object( std::vector<std::array<float,2>>{{(float)x, (float)y}, {(float)x+w, (float)y}, {(float)x+w, (float)y+h}, {(float)x, (float)y+h}}, HitboxType::RECTANGLE, Orientation::NONE)
    , label(
        text,
        "assets/fonts/Roboto-Regular.ttf",
        [&]() {
            int font_size = h > 0 ? (h * 3) / 7 : 14;
            if(font_size < 14)
                font_size = 14;
            if(font_size > 18)
                font_size = 18;
            return font_size;
        }(),
        SDL_Color{0, 0, 0, 255}
    )
{
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->anchor = true;
    this->on_press = std::move(on_click);
}

void Button::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    (void)w;
    (void)h;
    (void)theme;

    const SDL_Color base_color = {225, 225, 225, 255};
    const SDL_Color dark_color = {190, 190, 190, 255};
    const Uint64 now = SDL_GetTicks();
    constexpr Uint64 darken_ms = 90;
    constexpr Uint64 return_ms = 130;
    constexpr Uint64 anim_duration_ms = darken_ms + return_ms;

    float blend = 0.0f;
    if(last_press_ticks > 0)
    {
        const Uint64 elapsed = now - last_press_ticks;
        if(elapsed >= anim_duration_ms)
        {
            last_press_ticks = 0;
        }
        else if(elapsed < darken_ms)
        {
            blend = (float)elapsed / (float)darken_ms;
        }
        else
        {
            const Uint64 back_elapsed = elapsed - darken_ms;
            blend = 1.0f - ((float)back_elapsed / (float)return_ms);
        }
    }

    const Uint8 r = (Uint8)(base_color.r + (dark_color.r - base_color.r) * blend);
    const Uint8 g = (Uint8)(base_color.g + (dark_color.g - base_color.g) * blend);
    const Uint8 b = (Uint8)(base_color.b + (dark_color.b - base_color.b) * blend);

    SDL_FRect btn = {(float)this->x, (float)this->y, (float)this->w, (float)this->h };
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &btn);

    label.draw(renderer, this->x, this->y, this->w, this->h);
}

void Button::press()
{
    last_press_ticks = SDL_GetTicks();

    if(on_press)
    {
        on_press();
    }
}