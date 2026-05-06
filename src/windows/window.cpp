#include "windows/window.hpp"

#include <map>
#include <string>

// ── Constructor / Destructor ──────────────────────────────────────────────────

Window::Window(const char *title, int width, int height, Theme *theme)
    : theme(theme)
{
    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if(!window)
    {
        std::cerr << "Error creating window '" << title << "': " << SDL_GetError() << "\n";
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if(!renderer)
    {
        std::cerr << "Error creating renderer for '" << title << "': " << SDL_GetError() << "\n";
        exit(1);
    }

    SDL_SetRenderVSync(renderer, 1);
    running = true;
}

Window::~Window()
{
    destroy();
}

// ── Event handling ────────────────────────────────────────────────────────────

void Window::event_handler(SDL_Event &event)
{
    if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
       event.window.windowID == SDL_GetWindowID(window))
    {
        running = false;
    }
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void Window::clear_window(Color::Color *bg)
{
    // NOTE: original code passed bg->g twice (g instead of b). Fixed here.
    SDL_SetRenderDrawColor(renderer, bg->r, bg->g, bg->b, bg->a);
    SDL_RenderClear(renderer);
}

void Window::render()
{
    SDL_RenderPresent(renderer);
}

void Window::destroy()
{
    if(renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if(window)   { SDL_DestroyWindow(window);     window   = nullptr; }
}

size_t Window::add_object(Object *object)
{
    
    if(object)
    {
        int w, h; get_size(w, h);
        for(size_t i=0; i<object->corners.size(); i++) {
            object->corners[i][0] /= (float)w; object->corners[i][1] /= (float)h;
            object->base_shape[i][0] /= (float)w; object->base_shape[i][1] /= (float)h;
        }
        object->create_hitbox();
        objects.push_back(object);
        return objects.size();
    }
    return 0;
}

// ── Font / Text ───────────────────────────────────────────────────────────────

TTF_Font *Window::get_font(int size)
{
    // One cached font per size
    static std::map<int, TTF_Font*> cache;

    auto it = cache.find(size);
    if(it != cache.end())
        return it->second;

    static const char *paths[] = {
        "assets/fonts/Roboto-Regular.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        nullptr
    };

    TTF_Font *font = nullptr;
    for(int i = 0; paths[i]; ++i)
    {
        font = TTF_OpenFont(paths[i], size);
        if(font) break;
    }

    cache[size] = font;  // cache even if null so we don't retry every frame
    return font;
}

void Window::draw_text(const std::string &text, float x, float y,
                       SDL_Color color, int font_size)
{
    TTF_Font *font = get_font(font_size);
    if(!font)
        return;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if(!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture)
    {
        SDL_FRect dst = {x, y, (float)surface->w, (float)surface->h};
        SDL_RenderTexture(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }

    SDL_DestroySurface(surface);
}