#include "windows/object_page.hpp"
#include "windows/main_window.hpp"

namespace {
constexpr int page_button_x   = 30;
constexpr int page_button_y   = 30;
constexpr int page_button_gap = 14;

int half_width(Window *w)
{
    int W = 0, H = 0;
    if(w) w->get_size(W, H);
    return W > 0 ? W / 2 : 640;
}
int half_height(Window *w)
{
    int W = 0, H = 0;
    if(w) w->get_size(W, H);
    return H > 0 ? H / 2 : 360;
}
} // namespace

// ── Helpers ───────────────────────────────────────────────────────────────────

void ObjectPage::normalize_button(Button *btn)
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

// ── Constructor ───────────────────────────────────────────────────────────────

ObjectPage::ObjectPage(Window *main_window_ptr)
    : Window("Object Page",
             half_width(main_window_ptr),
             half_height(main_window_ptr),
             main_window_ptr ? main_window_ptr->theme : nullptr)
    , ptr_main(dynamic_cast<MainWindow*>(main_window_ptr))
{
    auto row = [](int n) { return page_button_y + n * (BTN_HEIGHT + page_button_gap); };

    auto make = [&](int y, const char *label, ObjectType type) {
        auto *btn = new Button(page_button_x, y, BTN_WIDTH, BTN_HEIGHT,
                               label, [this, type]() { create_object(type); });
        buttons.push_back(btn);
        normalize_button(btn);
    };

    make(row(0), "Add Spring", ObjectType::SPRING);
    make(row(1), "Add Mass",   ObjectType::MASS);
    make(row(2), "Add Plane",  ObjectType::PLANE);
}

// ── main_loop ─────────────────────────────────────────────────────────────────

void ObjectPage::main_loop()
{
    int w, h;
    get_size(w, h);

    for(Button *btn : buttons)
        btn->draw_object(get_renderer(), theme, w, h);
}

// ── event_handler ─────────────────────────────────────────────────────────────

void ObjectPage::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    int w, h;
    get_size(w, h);

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        for(Button *btn : buttons)
            if(btn->is_mouse_click(event.button.x, event.button.y, w, h))
                btn->press();
    }
}

// ── Object factories ──────────────────────────────────────────────────────────

void ObjectPage::create_object(ObjectType type)
{
    Object *obj = nullptr;
    switch(type)
    {
        case ObjectType::SPRING: obj = create_spring(); break;
        case ObjectType::MASS:   obj = create_mass();   break;
        case ObjectType::PLANE:  obj = create_plane();  break;
        default: break;
    }
    if(obj && ptr_main)
        ptr_main->add_object(obj);
}

Spring* ObjectPage::create_spring()
{
    int w = 1920, h = 1080;
    if(ptr_main) ptr_main->get_size(w, h);

    const float bw = 120.0f, bh = 40.0f;
    const float x  = w * 0.5f - bw * 0.5f;
    const float y  = h * 0.5f - bh * 0.5f;

    return new Spring({{x,y},{x+bw,y},{x+bw,y+bh},{x,y+bh}},
                      25.0f, false, 1.0f, Orientation::UP);
}

Mass* ObjectPage::create_mass()
{
    int w = 1920, h = 1080;
    if(ptr_main) ptr_main->get_size(w, h);

    const float bw = 60.0f, bh = 60.0f;
    const float x  = w * 0.5f - bw * 0.5f;
    const float y  = h * 0.5f - bh * 0.5f;

    return new Mass({{x,y},{x+bw,y},{x+bw,y+bh},{x,y+bh}},
                    HitboxType::RECTANGLE, 1.0f);
}

Plane* ObjectPage::create_plane()
{
    int w = 1920, h = 1080;
    if(ptr_main) ptr_main->get_size(w, h);

    const float cx = w * 0.5f, cy = h * 0.5f;
    const float lw = 320.0f,   th = 6.0f;

    return new Plane({{cx - lw*0.5f, cy - th*0.5f},
                      {cx + lw*0.5f, cy - th*0.5f},
                      {cx + lw*0.5f, cy + th*0.5f},
                      {cx - lw*0.5f, cy + th*0.5f}}, false);
}