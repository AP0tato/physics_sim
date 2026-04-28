#include "windows/light_window.hpp"

LightWindow::LightWindow(Theme *theme)
    : Window("Light Simulation", 1920, 1080, theme)
{
    // Scene initialisation will go here:
    // build BVH, seed rays, etc.
}

void LightWindow::main_loop()
{
    // 1. Step simulation  (trace rays, update scene)
    // 2. Draw results
    //
    // Placeholder: just clear to a dark background so the window is visible.
    Color bg{10, 10, 18, 255};
    clear_window(&bg);

    const SDL_Color white = {220, 220, 220, 255};
    draw_text("Light Simulation — coming soon", 40.0f, 40.0f, white, 24);
}

void LightWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    // Scene interaction (place emitters, mirrors, etc.) goes here.
}