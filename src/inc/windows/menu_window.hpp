#ifndef MENU_WINDOW_HPP
#define MENU_WINDOW_HPP

#include "windows/window.hpp"
#include "objects/button.hpp"

#include <vector>

class MenuWindow : public Window
{
public:
    explicit MenuWindow(Theme *theme);
    ~MenuWindow();

    void main_loop()                  override;
    void event_handler(SDL_Event &ev) override;

private:
    std::vector<Button*> buttons;

    void register_button(Button *btn);
};

#endif // MENU_WINDOW_HPP