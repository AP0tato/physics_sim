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
#include "windows/menu_window.hpp"

#define SDL_INIT_FLAGS (SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)

std::vector<Window*> windows;

int main(int argc, char const *argv[])
{
    if(!SDL_Init(SDL_INIT_FLAGS))
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

    Theme *theme = new Dark();

    windows.push_back(new MenuWindow(theme));

    SDL_Event event;

    while(!windows.empty())
    {
        // ── Events ────────────────────────────────────────────────────────
        while(SDL_PollEvent(&event))
        {
            // Iterate a copy of the size — handlers may push new windows
            const size_t count = windows.size();
            for(size_t i = 0; i < count; i++)
                windows[i]->event_handler(event);
        }

        // ── Remove closed windows (back-to-front so indices stay valid) ──
        // Track which window, if any, should be raised after cleanup.
        Window *raise_target = nullptr;

        for(size_t i = windows.size(); i > 0; i--)
        {
            const size_t idx = i - 1;
            if(!windows[idx]->running)
            {
                // If a child window (not index 0) closed, raise its parent.
                // We treat the window at index 0 as the "active primary".
                if(idx > 0 && raise_target == nullptr && windows.size() > 1)
                    raise_target = windows[0];

                windows[idx]->destroy();
                delete windows[idx];
                windows.erase(windows.begin() + (long)idx);
            }
        }

        if(raise_target)
            SDL_RaiseWindow(raise_target->get_window());

        if(windows.empty())
            break;

        // ── Render ────────────────────────────────────────────────────────
        for(Window *w : windows)
        {
            w->clear_window(&w->theme->background);
            w->main_loop();
            w->render();
        }
    }

    for(Window *w : windows)
        w->destroy();

#if defined(PHYSICS_HAS_SDL_TTF) && PHYSICS_HAS_TTF_HEADER
    TTF_Quit();
#endif
    SDL_Quit();

    std::cout << "Quitting...\n";
    return 0;
}