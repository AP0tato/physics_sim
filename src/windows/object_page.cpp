#include "windows/object_page.hpp"

ObjectPage::ObjectPage(Window *main_window_ptr) : Window("Object Page", 1920, 1080)
{
    this->ptr_main = main_window_ptr;

    Button *add_spring = new Button(30, 30, BTN_WIDTH, BTN_HEIGHT, [this](){ create_object(ObjectType::SPRING); });
    Button *add_mass = new Button(30, 30, BTN_WIDTH, BTN_HEIGHT, [this](){ create_object(ObjectType::MASS); });

    buttons.push_back(add_spring);
    buttons.push_back(add_mass);
}

void ObjectPage::main_loop()
{

}

void ObjectPage::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running)
        return;

    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
    {
        for(size_t i = 0; i < buttons.size(); i++)
        {
        if(buttons[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                Button *curr = dynamic_cast<Button*>(buttons[i]);
                curr->press();
            }
        }
    }
}

void ObjectPage::create_object(ObjectType type)
{
    Object *obj = nullptr;
    switch(type)
    {
        case ObjectType::SPRING:
            obj = create_spring();
            break;
        case ObjectType::MASS:
            obj = create_mass();
            break;
        default:
            break;
    }

    if(!obj)
        return;
}

Spring* ObjectPage::create_spring()
{

}

Mass* ObjectPage::create_mass()
{

}
