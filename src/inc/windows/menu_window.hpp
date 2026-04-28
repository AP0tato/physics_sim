#ifndef MENU_WINDOW_HPP
#define MENU_WINDOW_HPP

#include "windows/window.hpp"
#include "button.hpp"

#include <vector>

class MenuWindow : public Window
{
public:
    explicit MenuWindow(Theme *theme);

    void main_loop()                  override;
    void event_handler(SDL_Event &ev) override;

private:
    std::vector<Button*> buttons;

    // Normalises pixel coords → [0,1] after the window is created
    void register_button(Button *btn);
};

#endif // MENU_WINDOW_HPP