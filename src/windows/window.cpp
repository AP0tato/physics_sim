#include "windows/window.hpp"

Window::Window(const char* title, const int WIDTH, const int HEIGHT)
{
    window = SDL_CreateWindow(title, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    if(!window)
    {
        std::cout << "Error creating  " << title << " window: " << SDL_GetError() << "\n";
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if(!renderer)
    {
        std::cout << "Error creating  " << title << " renderer: " << SDL_GetError() << "\n";
        exit(1);
    }

    SDL_SetRenderVSync(renderer, 1);

    this->running = true;
}

void Window::event_handler(SDL_Event &event)
{
    if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
       event.window.windowID == SDL_GetWindowID(window))
    {
        running = false;
    }
}

void Window::main_loop() {}

void Window::clear_window(Color *bg)
{
    SDL_SetRenderDrawColor(this->renderer, bg->r, bg->g, bg->g, bg->a);
    SDL_RenderClear(this->renderer);
}

void Window::render()
{
    SDL_RenderPresent(this->renderer);
}

SDL_Window* Window::get_window()
{
    return this->window;
}

SDL_Renderer* Window::get_renderer()
{
    return this->renderer;
}

void Window::destroy()
{
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
}
