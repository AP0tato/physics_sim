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

    if(radial)
    {
        // ── Radial: draw a filled circle (approximated with lines from center)
        SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);  // warm yellow
        const int segs = 48;
        for(int i = 0; i < segs; ++i)
        {
            const float a0 = (float)i     / (float)segs * 2.0f * M_PI;
            const float a1 = (float)(i+1) / (float)segs * 2.0f * M_PI;
            SDL_RenderLine(renderer,
                (int)(cx + radius * cosf(a0)), (int)(cy + radius * sinf(a0)),
                (int)(cx + radius * cosf(a1)), (int)(cy + radius * sinf(a1)));
        }
        // Small bright centre dot
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect dot = {cx - 3.f, cy - 3.f, 6.f, 6.f};
        SDL_RenderFillRect(renderer, &dot);
    }
    else
    {
        // ── Linear (flat): two parallel lines across the object width
        //    White = front (top edge), Red = back (bottom edge)
        const float lx0 = left  * (float)w;
        const float lx1 = right * (float)w;
        const float ty  = top    * (float)h;
        const float by  = bottom * (float)h;
        const float mid = (ty + by) * 0.5f;

        // Back line (red)
        SDL_SetRenderDrawColor(renderer, 200, 40, 40, 255);
        SDL_RenderLine(renderer, (int)lx0, (int)by, (int)lx1, (int)by);

        // Body outline (dim)
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 200);
        SDL_RenderLine(renderer, (int)lx0, (int)ty, (int)lx0, (int)by);
        SDL_RenderLine(renderer, (int)lx1, (int)ty, (int)lx1, (int)by);
        SDL_RenderLine(renderer, (int)lx0, (int)mid, (int)lx1, (int)mid);

        // Front line (white)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderLine(renderer, (int)lx0, (int)ty, (int)lx1, (int)ty);
    }
}

// =============================================================================
// LightSource3D — skeleton
// =============================================================================
LightSource3D::LightSource3D(const std::vector<std::vector<float>> &corners)
    : PhysicsObject3D(corners, HitboxType::RECTANGLE, Orientation::NONE)
{
    // TODO: set ray direction, emission cone, intensity.
}