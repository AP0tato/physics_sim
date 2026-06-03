#include "objects/wall.hpp"
#include "engine.hpp"

#include <algorithm>

Wall2D::Wall2D(const std::vector<std::vector<float>> &corners, bool vertical)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
    , vertical(vertical)
{
    this->anchor = true;
}

bool Wall2D::is_mouse_click(int x, int y, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);

    constexpr float pad = 8.0f;

    if(!vertical)
    {
        const float ly = ((top + bottom) * 0.5f) * (float)h;
        return ((float)x >= left  * (float)w - pad &&
                (float)x <= right * (float)w + pad &&
                (float)y >= ly - pad && (float)y <= ly + pad);
    }
    else
    {
        const float lx = ((left + right) * 0.5f) * (float)w;
        return ((float)y >= top    * (float)h - pad &&
                (float)y <= bottom * (float)h + pad &&
                (float)x >= lx - pad && (float)x <= lx + pad);
    }
}

void Wall2D::resize_rect_object_handle(size_t handle_idx, int dx, int dy, int w, int h)
{
    if(corners.size() != 4) return;
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);

    if(!vertical)
    {
        // Horizontal wall — only resize along the x axis
        const float ndx = (float)dx / (float)w;
        switch(handle_idx)
        {
            case 2: case 3: case 4: right += ndx; break;
            case 6: case 7: case 0: left  += ndx; break;
            default: return;
        }
        const float min_w = 16.0f / (float)w;
        if(right - left < min_w) right = left + min_w;
    }
    else
    {
        // Vertical wall — only resize along the y axis
        const float ndy = (float)dy / (float)h;
        switch(handle_idx)
        {
            case 0: case 1: case 2: top    += ndy; break;
            case 4: case 5: case 6: bottom += ndy; break;
            default: return;
        }
        const float min_h = 16.0f / (float)h;
        if(bottom - top < min_h) bottom = top + min_h;
    }
    set_rect_from_bounds(left, top, right, bottom);
}

void Wall2D::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    float left   = corners[0][0], right  = corners[0][0];
    float top    = corners[0][1], bottom = corners[0][1];

    for(const auto &corner : corners)
    {
        left   = std::min(left,   corner[0]);
        right  = std::max(right,  corner[0]);
        top    = std::min(top,    corner[1]);
        bottom = std::max(bottom, corner[1]);
    }

    if(vertical)
    {
        const int x = (int)(((left + right) * 0.5f) * w);
        Engine::draw_line(renderer, x, (int)(top * h), x, (int)(bottom * h), &theme->foreground);
    }
    else
    {
        const int y = (int)(((top + bottom) * 0.5f) * h);
        Engine::draw_line(renderer, (int)(left * w), y, (int)(right * w), y, &theme->foreground);
    }
}