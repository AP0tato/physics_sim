#ifndef LIGHT_WINDOW_HPP
#define LIGHT_WINDOW_HPP

#include "windows/window.hpp"
#include "objects/object.hpp"
#include "objects/button.hpp"

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
    std::vector<Window*> windows;     // child windows (ObjectPage etc.)
    std::vector<Button*> ui_buttons;

    void normalize_button(Button *btn);
    void handle_collisions();
};

#endif // LIGHT_WINDOW_HPP