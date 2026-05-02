#include "engine.hpp"
#include "objects/object.hpp"

void translate_object(Object *object, float distance, float angle)
{
    
}

void rotate_object(Object *object, float angle)
{

}

void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, Color *color)
{
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
    SDL_RenderLine(renderer, x1, y1, x2, y2);
}