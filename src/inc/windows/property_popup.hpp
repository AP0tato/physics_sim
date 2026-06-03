#ifndef PROPERTY_POPUP_HPP
#define PROPERTY_POPUP_HPP

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include "objects/object.hpp"

class PropertyPopup
{
public:
    PropertyPopup();
    explicit PropertyPopup(Theme *theme);
    ~PropertyPopup();

    // Auto-build widgets from the object's type and properties, then show.
    void load_for_object(Object *obj, int w, int h);

    // Legacy: caller supplies pre-built widget list.
    void load(const Object *obj, std::vector<Object*> options, int w, int h);

    void draw(SDL_Renderer *renderer);
    bool handle_event(SDL_Event &event);
    bool contains(int x, int y) const;

private:
    SDL_FRect            panel  = {0,0,0,0};
    std::vector<Object*> options;          // widgets currently shown
    bool                 owns_options = false; // true when we built them ourselves
    Theme               *theme  = nullptr;
    int                  window_w = 0;
    int                  window_h = 0;
    std::string          title;

    void layout_options(float px, float py);
    void draw_panel(SDL_Renderer *renderer);
    void draw_title(SDL_Renderer *renderer);
    void clear_owned();
    std::string format_value(float value);
};

#endif