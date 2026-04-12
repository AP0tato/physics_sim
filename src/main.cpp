// Standard C/C++ includes
#include <iostream>
#include <vector>

// SDL includes
#include <SDL3/SDL.h>
#if defined(PHYSICS_HAS_SDL_TTF) && __has_include(<SDL3_ttf/SDL_ttf.h>)
#include <SDL3_ttf/SDL_ttf.h>
#define PHYSICS_HAS_TTF_HEADER 1
#elif defined(PHYSICS_HAS_SDL_TTF) && __has_include(<SDL_ttf.h>)
#include <SDL_ttf.h>
#define PHYSICS_HAS_TTF_HEADER 1
#else
#define PHYSICS_HAS_TTF_HEADER 0
#endif

// User includes
#include "app_state.hpp"
#include "themes.hpp"
#include "windows/window.hpp"
#include "windows/main_window.hpp"

#define SDL_INIT (SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)

std::vector<Window*> windows;

void open_objects_page(Window *main_window_ptr);

int main(int argc, char const *argv[])
{
    if(!SDL_Init(SDL_INIT))
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << "\n";
        return 1;
    }

#if defined(PHYSICS_HAS_SDL_TTF) && PHYSICS_HAS_TTF_HEADER
    if(!TTF_Init())
    {
        std::cout << "Error initializing SDL_ttf: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
#endif

    Theme *theme = new Light();
    theme = new Dark();

    MainWindow *main_window_ptr = new MainWindow(theme);
    windows.push_back(main_window_ptr);
    size_t main_window = windows.size() - 1;

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
                }
            }
        }
        
        // Remove closed child windows, then focus main window if any were closed.
        bool closed_child_window = false;
        for(size_t i = windows.size(); i > 0; i--)
        {
            if(i - 1 != main_window && !windows[i-1]->running)
            {
                windows[i-1]->destroy();
                windows.erase(windows.begin() + (i - 1));
                closed_child_window = true;
                if(main_window > i - 1)
                    main_window--;
            }
        }

        if(closed_child_window && main_window < windows.size())
            SDL_RaiseWindow(windows[main_window]->get_window());
        
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

#if defined(PHYSICS_HAS_SDL_TTF) && PHYSICS_HAS_TTF_HEADER
    TTF_Quit();
#endif
    SDL_Quit();

    std::cout << "Quitting...\n";

    return 0;
}
