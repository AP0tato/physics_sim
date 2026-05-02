#include "objects/checkbox.hpp"

CheckBox::CheckBox() = default;

CheckBox::CheckBox(int x, int y, int size, bool checked, std::function<void(bool)> on_toggle)
    : x(x), y(y), size(size), checked(checked), on_toggle(std::move(on_toggle))
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

void CheckBox::draw(SDL_Renderer *renderer) const
{
    SDL_FRect outer = {(float)x, (float)y, (float)size, (float)size};

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderRect(renderer, &outer);

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
