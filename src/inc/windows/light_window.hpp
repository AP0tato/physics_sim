#ifndef LIGHT_WINDOW_HPP
#define LIGHT_WINDOW_HPP

#include "windows/window.hpp"

class LightWindow : public Window
{
public:
    explicit LightWindow(Theme *theme);

    void main_loop()                  override;
    void event_handler(SDL_Event &ev) override;

    // Future scene state:
    // std::vector<Ray>     rays;
    // std::vector<Surface> surfaces;
    // BVH                  bvh;
};

#endif // LIGHT_WINDOW_HPP