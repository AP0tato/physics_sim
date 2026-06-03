#include "objects/light.hpp"
#include "engine.hpp"
#include <cmath>

// =============================================================================
// LightSource2D
// =============================================================================
LightSource2D::LightSource2D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{}

void LightSource2D::draw_object(SDL_Renderer *renderer, Theme * /*theme*/, int w, int h)
{
    // Compute center in pixels from the hitbox
    const float cx = ((hitbox[0][0] + hitbox[hitbox.size()/2][0]) * 0.5f) * (float)w;
    const float cy = ((hitbox[0][1] + hitbox[hitbox.size()/2][1]) * 0.5f) * (float)h;
    // Use the smaller half-extent as radius, minimum 8px
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);
    const float radius = std::max(8.0f,
        std::min((right - left) * (float)w, (bottom - top) * (float)h) * 0.5f);

    const float dir_rad = angle_deg * (M_PI / 180.0f);
    const float ddx     = std::cos(dir_rad);
    const float ddy     = std::sin(dir_rad);

    if(radial)
    {
        // ── Radial: circle + direction arrow
        SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
        const int segs = 48;
        for(int i = 0; i < segs; ++i)
        {
            const float a0 = (float)i     / (float)segs * 2.0f * M_PI;
            const float a1 = (float)(i+1) / (float)segs * 2.0f * M_PI;
            SDL_RenderLine(renderer,
                (int)(cx + radius * cosf(a0)), (int)(cy + radius * sinf(a0)),
                (int)(cx + radius * cosf(a1)), (int)(cy + radius * sinf(a1)));
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderLine(renderer, (int)cx, (int)cy,
            (int)(cx + radius * ddx), (int)(cy + radius * ddy));
        SDL_FRect dot = {cx - 3.f, cy - 3.f, 6.f, 6.f};
        SDL_RenderFillRect(renderer, &dot);
    }
    else
    {
        // ── Linear: rectangle body + highlighted front edge
        const float lx0 = left  * (float)w;
        const float lx1 = right * (float)w;
        const float ty  = top   * (float)h;
        const float by  = bottom * (float)h;

        // Body outline (dim)
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 200);
        SDL_RenderLine(renderer, (int)lx0, (int)ty, (int)lx1, (int)ty);
        SDL_RenderLine(renderer, (int)lx1, (int)ty, (int)lx1, (int)by);
        SDL_RenderLine(renderer, (int)lx1, (int)by, (int)lx0, (int)by);
        SDL_RenderLine(renderer, (int)lx0, (int)by, (int)lx0, (int)ty);

        // Highlight the front face (same edge logic as simulate_rays)
        SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
        if(std::abs(ddx) >= std::abs(ddy))
        {
            const float ex = ((ddx >= 0.0f) ? right : left) * (float)w;
            SDL_RenderLine(renderer, (int)ex, (int)ty, (int)ex, (int)by);
        }
        else
        {
            const float ey = ((ddy >= 0.0f) ? bottom : top) * (float)h;
            SDL_RenderLine(renderer, (int)lx0, (int)ey, (int)lx1, (int)ey);
        }

        // Direction tick from centre toward front edge
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 160);
        SDL_RenderLine(renderer, (int)cx, (int)cy,
            (int)(cx + radius * ddx), (int)(cy + radius * ddy));

        // Centre dot
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect dot = {cx - 3.f, cy - 3.f, 6.f, 6.f};
        SDL_RenderFillRect(renderer, &dot);
    }
}

// Only allow resizing along the length (horizontal handles: left/right edges).
// Handles 3 (right-mid) and 7 (left-mid) change width; all others are blocked.
void LightSource2D::resize_rect_object_handle(size_t handle_idx, int dx, int /*dy*/, int w, int h)
{
    if(corners.size() != 4) return;
    float left, top, right, bottom;
    get_rect_bounds(left, top, right, bottom);
    const float ndx = (float)dx / (float)w;
    // handle layout (same as base class):
    //  0=TL 1=TM 2=TR  3=MR  4=BR 5=BM 6=BL  7=ML
    // Only move left/right edges; top/bottom are fixed.
    switch(handle_idx)
    {
        case 2: case 3: case 4: right += ndx; break;  // right side
        case 6: case 7: case 0: left  += ndx; break;  // left side
        default: return;  // top/bottom handles — block
    }
    const float min_w = 16.0f / (float)w;
    if(right - left < min_w) right = left + min_w;
    set_rect_from_bounds(left, top, right, bottom);
}


// =============================================================================
LightSource3D::LightSource3D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject3D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    // TODO: set ray direction, emission cone, intensity.
}