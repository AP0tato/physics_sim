#ifndef WINDOW_HPP

#define WINDOW_HPP

#include <SDL3/SDL.h>
#include <iostream>
#include "color.hpp"

class Window
{
    private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    public:
    Window(const char* title, const int WIDTH, const int HEIGHT);
    void clear_window(Color *bg);
    void render();
    SDL_Window* get_window();
    SDL_Renderer* get_renderer();
    void destroy();
    
    virtual void event_handler(SDL_Event &event);
    virtual void main_loop();

    bool running;
};

#endif
