#include "objects/object.hpp"
#include "engine.hpp"

namespace
{
    constexpr float selection_padding_px         = 10.0f;
    constexpr float selection_handle_size        = 9.0f;
    constexpr int   selection_handle_pick_radius = 10;
}

Object::Object(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation)
{
    this->hitbox_type = hitbox_type;
    this->corners = corners;
    this->base_shape = corners;
    this->orientation = orientation;
    this->velocity_x = 0.0f;
    this->velocity_y = 0.0f;
    this->anchor = false;
    create_hitbox();
}

void Object::create_hitbox()
{
    // SPECIAL hitbox types are not handled here
    if(hitbox_type == HitboxType::SPECIAL)
    {
        std::cout << "Could not create object hitbox, implementation is missing\n";
        return;
    }
    else if(hitbox_type == HitboxType::ELLIPSE)
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
        // Ellipse hitbox layout as 2D: single element {center_x, center_y, radius_x, radius_y}
        this->hitbox.clear();
        this->hitbox.push_back({c_x, c_y, r_x, r_y});
    }
    else
    {
        // Rectangle hitbox layout as 2D: four corner entries [{x,y,0,0}, ...]
        this->hitbox.clear();
        const size_t n = corners.size();
        // If we have at least four corners use the first four, otherwise use all available
        if(n >= 4)
        {
            for(size_t i = 0; i < 4; ++i)
                this->hitbox.push_back({corners[i][0], corners[i][1], 0.0f, 0.0f});
        }
        else
        {
            for(size_t i = 0; i < n; ++i)
                this->hitbox.push_back({corners[i][0], corners[i][1], 0.0f, 0.0f});
        }
    }
}

bool Object::is_mouse_click(int x, int y, int w, int h)
{
    if(this->hitbox_type == HitboxType::RECTANGLE)
    {
        if(this->hitbox.empty()) return false;
        float left = hitbox[0][0], top = hitbox[0][1], right = hitbox[0][0], bottom = hitbox[0][1];
        for(const auto &p : hitbox)
        {
            if(p[0] < left) left = p[0];
            if(p[0] > right) right = p[0];
            if(p[1] < top) top = p[1];
            if(p[1] > bottom) bottom = p[1];
        }
        return (x >= left * w && y >= top * h && x <= right * w && y <= bottom * h);
    }
    else if(this->hitbox_type == HitboxType::ELLIPSE)
    {
        if(this->hitbox.empty()) return false;
        const auto &e = this->hitbox[0];
        if(e[2] == 0.0f || e[3] == 0.0f) return false;

        const float cx = e[0] * w;
        const float cy = e[1] * h;
        const float rx = e[2] * w;
        const float ry = e[3] * h;

        return (
            ((x - cx) * (x - cx)) / (rx * rx) +
            ((y - cy) * (y - cy)) / (ry * ry)
            <= 1
        );
    }
    return false;
}

void Object::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    const int n = corners.size();
    for(int i = 0; i < n; i++)
    {
        int x1 = corners[i][0]*w;
        int y1 = corners[i][1]*h;
        int x2 = corners[(i+1)%n][0]*w;
        int y2 = corners[(i+1)%n][1]*h;

        Engine::draw_line(renderer, x1, y1, x2, y2, &theme->foreground);
    }
}

void Object::move_object_by_pixels(int dx, int dy, int w, int h)
{
    const float ddx = (float)dx / (float)w;
    const float ddy = (float)dy / (float)h;

    for(size_t i = 0; i < corners.size(); i++)
    {
        corners[i][0]    += ddx;  corners[i][1]    += ddy;
        base_shape[i][0] += ddx;  base_shape[i][1] += ddy;
    }

    float sx, sy;
    constrain_object_to_window(sx, sy);
    create_hitbox();
}

bool Object::constrain_object_to_window(float &shift_x, float &shift_y)
{
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);

    shift_x = shift_y = 0.0f;
    if(left  < 0.0f) shift_x = -left;
    else if(right  > 1.0f) shift_x = 1.0f - right;

    if(top   < 0.0f) shift_y = -top;
    else if(bottom > 1.0f) shift_y = 1.0f - bottom;

    if(std::fabs(shift_x) < 1e-6f && std::fabs(shift_y) < 1e-6f)
        return false;

    for(size_t i = 0; i < corners.size(); i++)
    {
        corners[i][0]    += shift_x;  corners[i][1]    += shift_y;
        base_shape[i][0] += shift_x;  base_shape[i][1] += shift_y;
    }
    return true;
}

void Object::get_rect_bounds(float &left, float &top, float &right, float &bottom) const
{
    left = right = corners[0][0];
    top = bottom  = corners[0][1];

    for(const auto &c : corners)
    {
        if(c[0] < left)   left   = c[0];
        if(c[0] > right)  right  = c[0];
        if(c[1] < top)    top    = c[1];
        if(c[1] > bottom) bottom = c[1];
    }
}

void Object::resize_rect_object_handle(size_t handle_idx, int dx, int dy, int w, int h)
{
    if(corners.size() != 4) return;
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);
    const float ndx = (float)dx / (float)w, ndy = (float)dy / (float)h;
    bool ml = false, mr = false, mt = false, mb = false;
    switch(handle_idx) {
        case 0: ml = mt = true; break; case 1: mt = true; break; case 2: mr = mt = true; break;
        case 3: mr = true; break; case 4: mr = mb = true; break; case 5: mb = true; break;
        case 6: ml = mb = true; break; case 7: ml = true; break;
    }
    if(ml) left += ndx; if(mr) right += ndx; if(mt) top += ndy; if(mb) bottom += ndy;
    const float min_w = 16.0f/(float)w, min_h = 16.0f/(float)h;
    if(right-left < min_w) { if(ml && !mr) left = right-min_w; else right = left+min_w; }
    if(bottom-top < min_h) { if(mt && !mb) top = bottom-min_h; else bottom = top+min_h; }
    set_rect_from_bounds(left, top, right, bottom);
}

void Object::draw_selection_frame(SDL_Renderer *r, int w, int h) const
{
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);
    SDL_SetRenderDrawColor(r, 80, 170, 255, 255);
    SDL_FRect frame = { left*w-selection_padding_px, top*h-selection_padding_px, (right-left)*w+2*selection_padding_px, (bottom-top)*h+2*selection_padding_px };
    SDL_RenderRect(r, &frame);
    for(const auto &handle : get_resize_handles_px(w, h))
    {
        SDL_FRect dot = { handle.x - selection_handle_size*0.5f, handle.y - selection_handle_size*0.5f, selection_handle_size, selection_handle_size };
        SDL_RenderFillRect(r, &dot);
    }
}

std::array<SDL_FPoint, 8> Object::get_resize_handles_px(int w, int h) const
{
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);
    const float l  = left  * w - selection_padding_px;
    const float t  = top   * h - selection_padding_px;
    const float r  = right * w + selection_padding_px;
    const float b  = bottom* h + selection_padding_px;
    const float mx = (l + r) * 0.5f;
    const float my = (t + b) * 0.5f;
    return { SDL_FPoint{l,t}, {mx,t}, {r,t}, {r,my}, SDL_FPoint{r,b}, {mx,b}, {l,b}, {l,my} };
}

bool Object::try_get_resize_handle(int mouse_x, int mouse_y, int w, int h, size_t &handle_idx) const
{
    constexpr int r2 = selection_handle_pick_radius * selection_handle_pick_radius;
    auto handles = get_resize_handles_px(w, h);
    for(size_t i = 0; i < handles.size(); i++)
    {
        const int dx = mouse_x - (int)handles[i].x;
        const int dy = mouse_y - (int)handles[i].y;
        if(dx*dx + dy*dy <= r2) { handle_idx = i; return true; }
    }
    return false;
}

void Object::set_rect_from_bounds(float left, float top, float right, float bottom)
{
    corners[0] = base_shape[0] = {left,  top};
    corners[1] = base_shape[1] = {right, top};
    corners[2] = base_shape[2] = {right, bottom};
    corners[3] = base_shape[3] = {left,  bottom};
    create_hitbox();
}

void Object::on_property_popup_load(float x, float y, float width, float height)
{
    // Base implementation (empty) - derived classes override as needed
}
