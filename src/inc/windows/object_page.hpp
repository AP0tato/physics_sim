#ifndef OBJECT_PAGE_HPP

#define OBJECT_PAGE_HPP

#include "windows/window.hpp"
#include "button.hpp"
#include "spring.hpp"
#include "mass.hpp"
#include "plane.hpp"

#define BTN_WIDTH 180
#define BTN_HEIGHT 56

class MainWindow;

class ObjectPage : public Window
{
    private:
    MainWindow *ptr_main;
    std::vector<Button*> buttons;
    void normalize_button(Button *button);
    void create_object(ObjectType type);
    Spring* create_spring();
    Mass* create_mass();
    Plane* create_plane();

    public:
    ObjectPage(Window *main_window_ptr);

    virtual void main_loop() override;
    virtual void event_handler(SDL_Event &event) override;
};

#endif
