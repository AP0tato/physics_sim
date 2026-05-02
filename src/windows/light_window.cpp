#include "windows/light_window.hpp"
#include "windows/object_page.hpp"

LightWindow::LightWindow(Theme *theme)
    : Window("Light Simulation", 1920, 1080, theme)
{
    int w, h;
    get_size(w, h);

    constexpr int btn_w = 150, btn_h = 50;
    constexpr int top_margin = 10, right_margin = 10;

    auto *btn = new Button(w - btn_w - right_margin, top_margin,
                           btn_w, btn_h, "Object Page",
                           [this]() { windows.push_back(new ObjectPage(this)); });
    normalize_button(btn);
    ui_buttons.push_back(btn);
}

LightWindow::~LightWindow()
{
    for(Window *w : windows)  delete w;
    for(Object *o : objects)  delete o;
    for(Button *b : ui_buttons) delete b;
}

void LightWindow::normalize_button(Button *btn)
{
    int w, h;
    get_size(w, h);
    for(size_t i = 0; i < btn->corners.size(); i++)
    {
        btn->corners[i][0]    /= (float)w;  btn->corners[i][1]    /= (float)h;
        btn->base_shape[i][0] /= (float)w;  btn->base_shape[i][1] /= (float)h;
    }
    btn->create_hitbox();
}

void LightWindow::main_loop()
{
    int w, h;
    get_size(w, h);

    // Draw scene objects
    for(Object *o : objects)
        if(o) o->draw_object(get_renderer(), theme, w, h);

    // Draw toolbar
    for(Button *b : ui_buttons)
        b->draw_object(get_renderer(), theme, w, h);

    // Placeholder label
    const SDL_Color c = {80, 80, 80, 255};
    draw_text("Light Simulation", 40.f, 40.f, c, 24);

    // Render child windows (ObjectPage etc.)
    for(Window *cw : windows)
    {
        cw->clear_window(&cw->theme->background);
        cw->main_loop();
        cw->render();
    }
}

void LightWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    // Forward events to child windows and prune any that have closed
    for(auto it = windows.begin(); it != windows.end(); )
    {
        (*it)->event_handler(event);
        if(!(*it)->running)
        {
            delete *it;
            it = windows.erase(it);
        }
        else ++it;
    }

    // Only process mouse events that belong to this window
    if((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.type == SDL_EVENT_MOUSE_BUTTON_UP   ||
        event.type == SDL_EVENT_MOUSE_MOTION)
       && event.window.windowID != SDL_GetWindowID(get_window()))
        return;

    int w, h;
    get_size(w, h);

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        for(Button *b : ui_buttons)
            if(b->is_mouse_click(event.button.x, event.button.y, w, h))
                b->press();
    }
}

void LightWindow::handle_collisions()
{

}
