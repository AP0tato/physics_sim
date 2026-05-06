#ifndef PROPERTY_POPUP_HPP

#define PROPERTY_POPUP_HPP

#include <vector>
#include <SDL3/SDL.h>
#include <random>
#include "objects/object.hpp"

class PropertyPopup
{
    private:
    SDL_FRect            panel;
    std::vector<Object*> options;
    Theme*               theme;
    int                  window_w = 0;
    int                  window_h = 0;

    void draw_panel(SDL_Renderer *renderer);

    std::string format_value(float value);

    public:
    PropertyPopup();
    PropertyPopup(Theme *theme);

    void draw(SDL_Renderer *renderer);
    void load(const Object* obj, std::vector<Object*> options, int w, int h);
    bool handle_event(SDL_Event &event);
    bool contains(int x, int y) const;
    void update();
};

#endif