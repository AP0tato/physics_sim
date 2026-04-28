#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "color.hpp"
#include "themes.hpp"

// Forward declaration so Window can hold the global window list
// without a circular include. Defined in app_state.hpp / main.cpp.
extern std::vector<class Window*> windows;

class Window
{
public:
    Window(const char *title, int width, int height, Theme *theme);
    virtual ~Window();

    // Core loop hooks — override in subclasses
    virtual void main_loop() {}
    virtual void event_handler(SDL_Event &event);

    // SDL accessors
    SDL_Window   *get_window()   const { return window; }
    SDL_Renderer *get_renderer() const { return renderer; }

    // Convenience: fills w/h without callers reaching into SDL directly
    void get_size(int &w, int &h) const { SDL_GetWindowSize(window, &w, &h); }

    // Rendering helpers available to all subclasses
    void clear_window(Color *bg);
    void render();

    // Text rendering — shared so MenuWindow, LightWindow etc. all have it
    void draw_text(const std::string &text, float x, float y,
                   SDL_Color color, int font_size = 14);

    void destroy();

    Theme *theme = nullptr;  // owned by whoever created the window (e.g. main)
    bool   running = false;

protected:
    // Returns a cached font at the requested size.
    // Searches the standard platform font paths.
    static TTF_Font *get_font(int size);

private:
    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;
};

#endif // WINDOW_HPP