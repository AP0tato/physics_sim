#include "windows/object_list_window.hpp"
#include "windows/main_window.hpp"
#include "windows/object_page.hpp"

#include <string>

namespace {

const char *object_type_text(ObjectType type)
{
    switch(type)
    {
        case ObjectType::SPRING:      return "Spring";
        case ObjectType::MASS:        return "Mass";
        case ObjectType::BUTTON:      return "Button";
        case ObjectType::PLANE:       return "Plane";
        case ObjectType::LIGHT_SOURCE:return "Light Source";
        case ObjectType::WALL:        return "Wall";
        case ObjectType::MIRROR:      return "Mirror";
        case ObjectType::LASER:       return "Laser";
        case ObjectType::LIGHT_RAY:   return "Light Ray";
        case ObjectType::SLIDER:      return "Slider";
        case ObjectType::CHECKBOX:    return "Checkbox";
        case ObjectType::TEXTFIELD:   return "Textfield";
        case ObjectType::TOGGLEBOX:   return "Togglebox";
        default:                      return "Object";
    }
}

}

ObjectListWindow::ObjectListWindow(Window *source_window)
    : Window("Object List", 420, 600, source_window ? source_window->theme : nullptr)
    , source_window(source_window)
{
    const int btn_w = 180;
    const int btn_h = 42;
    const int btn_x = 18;

    auto add_control = [&](int y, const char *label, std::function<void()> cb)
    {
        auto *button = new Button(btn_x, y, btn_w, btn_h, label, cb);
        controls.push_back(button);
    };

    add_control(56, "Add Object", [source_window]() {
        if(source_window)
            ::windows.push_back(new ObjectPage(source_window));
    });

    play_button_control = new Button(btn_x, 108, btn_w, btn_h, "Play", [this, source_window]() {
        if(auto *main_window = dynamic_cast<MainWindow*>(source_window))
        {
            main_window->toggle_playing();
            if(play_button_control)
                play_button_control->label->set_text(main_window->is_playing() ? "Stop" : "Play");
            return;
        }

        local_playing = !local_playing;
        if(play_button_control)
            play_button_control->label->set_text(local_playing ? "Stop" : "Play");
    });
    controls.push_back(play_button_control);
}

ObjectListWindow::~ObjectListWindow()
{
    for(Button *button : controls)
        delete button;
}

void ObjectListWindow::main_loop()
{
    int w, h;
    get_size(w, h);

    draw_text("Objects in Scene", 18.f, 18.f, {230, 230, 230, 255}, 24);

    for(Button *button : controls)
        button->draw_object(get_renderer(), theme, w, h);

    float y = 180.f;
    const std::vector<Object*> &scene_objects = source_window ? source_window->objects : objects;
    for(size_t i = 0; i < scene_objects.size(); ++i)
    {
        Object *object = scene_objects[i];
        std::string label = std::to_string(i + 1) + ". " + object_type_text(object ? object->type() : ObjectType::BUTTON);
        draw_text(label, 18.f, y, {200, 200, 200, 255}, 16);
        y += 28.f;
    }

    if(scene_objects.empty())
        draw_text("No objects yet", 18.f, 180.f, {160, 160, 160, 255}, 16);
}

void ObjectListWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running)
        return;

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.window.windowID == SDL_GetWindowID(get_window()))
    {
        int w, h;
        get_size(w, h);
        for(Button *button : controls)
            if(button->is_mouse_click(event.button.x, event.button.y, w, h))
                { button->press(); break; }
    }

    if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        running = false;
}