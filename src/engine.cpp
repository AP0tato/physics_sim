#include "engine.hpp"
#include "objects/object.hpp"

void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, Color::Color *color)
{
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
    SDL_RenderLine(renderer, x1, y1, x2, y2);
}