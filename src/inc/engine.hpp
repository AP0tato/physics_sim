#ifndef ENGINE_HPP

#define ENGINE_HPP

#include <SDL3/SDL.h>
#include "color.hpp"

// Forward declaration to avoid circular includes
class Object;

void translate_object(Object *object, float distance, float angle);

void rotate_object(Object *object, float angle);

void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, Color::Color *color);

#endif
