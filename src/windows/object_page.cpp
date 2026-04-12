#include "windows/object_page.hpp"
#include "windows/main_window.hpp"

namespace {
constexpr int page_button_x = 30;
constexpr int page_button_y = 30;
constexpr int page_button_gap = 14;

int object_page_width(Window *main_window_ptr)
{
    if(!main_window_ptr)
        return 640;

    int w = 0;
    int h = 0;
    SDL_GetWindowSize(main_window_ptr->get_window(), &w, &h);
    (void)h;
    return w > 0 ? w / 2 : 640;
}

int object_page_height(Window *main_window_ptr)
{
    if(!main_window_ptr)
        return 360;

    int w = 0;
    int h = 0;
    SDL_GetWindowSize(main_window_ptr->get_window(), &w, &h);
    (void)w;
    return h > 0 ? h / 2 : 360;
}
}

void ObjectPage::normalize_button(Button *button)
{
    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);

    for(size_t i = 0; i < button->corners.size(); i++)
    {
        button->corners[i][0] /= (float)w;
        button->corners[i][1] /= (float)h;
        button->base_shape[i][0] /= (float)w;
        button->base_shape[i][1] /= (float)h;
    }

    button->create_hitbox();
}

ObjectPage::ObjectPage(Window *main_window_ptr)
    : Window("Object Page", object_page_width(main_window_ptr), object_page_height(main_window_ptr))
{
    this->ptr_main = dynamic_cast<MainWindow*>(main_window_ptr);

    Button *add_spring = new Button(page_button_x, page_button_y, BTN_WIDTH, BTN_HEIGHT, "Add Spring", [this](){ create_object(ObjectType::SPRING); });
    Button *add_mass = new Button(page_button_x, page_button_y + BTN_HEIGHT + page_button_gap, BTN_WIDTH, BTN_HEIGHT, "Add Mass", [this](){ create_object(ObjectType::MASS); });

    buttons.push_back(add_spring);
    buttons.push_back(add_mass);

    normalize_button(add_spring);
    normalize_button(add_mass);
}

void ObjectPage::main_loop()
{
    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);

    Theme *draw_theme = ptr_main ? ptr_main->theme : nullptr;
    Dark fallback_theme;
    if(!draw_theme)
        draw_theme = &fallback_theme;

    for(Button *button : buttons)
        button->draw_object(get_renderer(), draw_theme, w, h);
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

    if(ptr_main)
        ptr_main->add_object(obj);
}

Spring* ObjectPage::create_spring()
{
    int w = 1920;
    int h = 1080;
    if(ptr_main)
        SDL_GetWindowSize(ptr_main->get_window(), &w, &h);

    const float box_w = 120.0f;
    const float box_h = 40.0f;
    const float x = (w * 0.5f) - (box_w * 0.5f);
    const float y = (h * 0.5f) - (box_h * 0.5f);

    std::vector<std::array<float,2>> corners = {
        {x, y},
        {x + box_w, y},
        {x + box_w, y + box_h},
        {x, y + box_h}
    };

    return new Spring(corners, 25.0f, false, 1.0f, Orientation::UP);
}

Mass* ObjectPage::create_mass()
{
    int w = 1920;
    int h = 1080;
    if(ptr_main)
        SDL_GetWindowSize(ptr_main->get_window(), &w, &h);

    const float box_w = 60.0f;
    const float box_h = 60.0f;
    const float x = (w * 0.5f) - (box_w * 0.5f);
    const float y = (h * 0.5f) - (box_h * 0.5f);

    std::vector<std::array<float,2>> corners = {
        {x, y},
        {x + box_w, y},
        {x + box_w, y + box_h},
        {x, y + box_h}
    };

    return new Mass(corners, HitboxType::RECTANGLE, 1.0f);
}
