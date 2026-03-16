#include "object.hpp"
#include "engine.hpp"

Object::Object(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation)
{
    this->hitbox_type = hitbox_type;
    this->corners = corners;
    this->base_shape = corners;
    this->orientation = orientation;
    create_hitbox();
}

void Object::create_hitbox()
{
    if(hitbox_type == HitboxType::ELLIPSE)
    {
        float c_x = 0, min_x = corners[0][0];
        float c_y = 0, min_y = corners[0][1];
        const int n = corners.size();
        for(int i = 0; i < n; i++)
        {
            c_x += corners[i][0];
            c_y += corners[i][1];
            min_x = min_x < corners[i][0] ? min_x : corners[i][0];
            min_y = min_y < corners[i][1] ? min_y : corners[i][1];
        }
        c_x /= n;
        c_y /= n;
        float r_x = c_x - min_x;
        float r_y = c_y - min_y;
        this->hitbox = {{c_x, c_y}, {r_x, r_y}};
    }
    else
    {
        this->hitbox = {
            {corners[0][0], corners[0][1]},
            {corners[2][0], corners[2][1]}
        };
    }
}

bool Object::is_mouse_click(int x, int y)
{
    if(this->hitbox_type == HitboxType::RECTANGLE)
    {
        return (x >= hitbox[0][0] && y >= hitbox[0][1] && x <= hitbox[1][0] && y <= hitbox[1][1]);
    }
    else if(this->hitbox_type == HitboxType::ELLIPSE)
    {
        if(hitbox.size() < 2 || hitbox[1][0] == 0 || hitbox[1][1] == 0)
            return false;
        return (
            ((x - hitbox[0][0]) * (x - hitbox[0][0])) / (hitbox[1][0] * hitbox[1][0]) +
            ((y - hitbox[0][1]) * (y - hitbox[0][1])) / (hitbox[1][1] * hitbox[1][1])
            <= 1
        );
    }
    return false;
}

void Object::draw_object(SDL_Renderer *renderer, Theme *theme)
{
    const int n = corners.size();
    for(int i = 0; i < n; i++)
    {
        int x1 = corners[i][0];
        int y1 = corners[i][1];
        int x2 = corners[(i+1)%n][0];
        int y2 = corners[(i+1)%n][1];

        draw_line(renderer, x1, y1, x2, y2, &theme->foreground);
    }
}