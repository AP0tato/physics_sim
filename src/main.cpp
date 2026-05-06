#include <iostream>
#include <vector>

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

    // windows[0] is ALWAYS the MenuWindow for the lifetime of the app.
    // Simulations are pushed on top (index 1+).
    // Closing a simulation (index > 0) reveals the menu again.
    // Closing the menu (index 0) exits the app.
    windows.push_back(new MenuWindow(theme));

    SDL_Event event;

    while(!windows.empty())
    {
        // ── Events ────────────────────────────────────────────────────────
        while(SDL_PollEvent(&event))
        {
            const size_t count = windows.size();
            for(size_t i = 0; i < count; i++)
                windows[i]->event_handler(event);
        }

        // ── Remove closed windows ─────────────────────────────────────────
        // Scan back-to-front so erasing doesn't invalidate forward indices.
        bool menu_closed   = false;
        bool sim_closed    = false;

        for(size_t i = windows.size(); i > 0; i--)
        {
            const size_t idx = i - 1;
            if(windows[idx]->running) continue;

            if(idx == 0)
                menu_closed = true;   // MenuWindow itself was closed → quit
            else
                sim_closed = true;    // a sim or child closed → show menu

            windows[idx]->destroy();
            delete windows[idx];
            windows.erase(windows.begin() + (long)idx);
        }

        // If the menu was closed, we're done
        if(menu_closed) break;

        // If a sim closed and the menu is still alive, bring it forward
        if(sim_closed && !windows.empty())
            SDL_RaiseWindow(windows[0]->get_window());

        if(windows.empty()) break;

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
    (void) argc;
    (void) argv;
    return 0;
}