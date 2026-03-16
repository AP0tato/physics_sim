// Standard C/C++ includes
#include <iostream>
#include <vector>

// SDL includes
#include <SDL3/SDL.h>

// User includes
#include "themes.hpp"
#include "windows/window.hpp"
#include "windows/main_window.hpp"

#define SDL_INIT (SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)
#define HEIGHT 720
#define WIDTH 1080

void open_objects_page();

int main(int argc, char const *argv[])
{
    if(!SDL_Init(SDL_INIT))
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    Theme *theme = new Light();
    // theme = new Dark();

    std::vector<Window*> windows;
    MainWindow *main_window_ptr = new MainWindow(theme);
    windows.push_back(main_window_ptr);
    size_t main_window = windows.size() - 1;

    float width = 30;
    float height = 30;

    std::vector<std::array<float,2>> spring_shape = {
        {(float)(WIDTH/2), (float)(HEIGHT/2)},
        {(float)(WIDTH/2+width), (float)(HEIGHT/2)},
        {(float)(WIDTH/2+width), (float)(HEIGHT/2+height)},
        {(float)(WIDTH/2), (float)(HEIGHT/2+height)}
    },
    mass_shape = {
        {5, 5},
        {5 + width, 5},
        {5 + width, 5 + height},
        {5, 5 + height}
    };

    main_window_ptr->add_object(new Spring(spring_shape, 25, false, 0, Orientation::UP));
    main_window_ptr->add_object(new Mass(mass_shape, HitboxType::RECTANGLE, 1));

    SDL_Event event;
    bool running = true;

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            for(size_t i = 0; i < windows.size(); i++)
            {
                windows[i]->event_handler(event);
                if(!windows[i]->running)
                {
                    if(i == main_window)
                        running = false;
                    else
                        windows[i]->destroy();
                }

            }
        }
        Color *bg = &theme->background;
        for(Window* window : windows)
        {
            window->clear_window(bg);
            window->main_loop();
        }

        for(Window* window : windows)
            window->render();
    }

    for(Window* window : windows)
        window->destroy();

    SDL_Quit();

    std::cout << "Quitting...\n";

    return 0;
}

void open_objects_page()
{
    printf("Button pressed\n");
}
