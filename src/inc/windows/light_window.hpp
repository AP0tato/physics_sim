#ifndef LIGHT_WINDOW_HPP
#define LIGHT_WINDOW_HPP

#include "windows/window.hpp"
#include "objects/object.hpp"
#include "objects/button.hpp"
#include "objects/mirror.hpp"
#include "objects/light.hpp"
#include "windows/property_popup.hpp"

#include <vector>

class ObjectPage;

class LightWindow : public Window
{
public:
    explicit LightWindow(Theme *theme);
    ~LightWindow();

    void main_loop()                  override;
    void event_handler(SDL_Event &ev) override;

private:
    Button               *play_button       = nullptr;
    PropertyPopup        *property_popup    = nullptr;

    std::vector<Window*> windows;     // child windows (ObjectPage etc.)
    std::vector<Button*> ui_buttons;
    std::vector<Object*> property_mass;
    std::vector<Object*> property_mirror;
    std::vector<Object*> property_source;

    void normalize_button(Button *btn);
    void handle_collisions();
};

#endif // LIGHT_WINDOW_HPP