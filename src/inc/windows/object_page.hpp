#ifndef OBJECT_PAGE_HPP

#define OBJECT_PAGE_HPP

#include "windows/window.hpp"
#include "button.hpp"
#include "spring.hpp"
#include "mass.hpp"

#define BTN_WIDTH 50
#define BTN_HEIGHT 30

class ObjectPage : public Window
{
    private:
    Window *ptr_main;
    std::vector<Button*> buttons;
    void create_object(ObjectType type);
    Spring* create_spring();
    Mass* create_mass();

    public:
    ObjectPage(Window *main_window_ptr);

    virtual void main_loop() override;
    virtual void event_handler(SDL_Event &event) override;
};

#endif
