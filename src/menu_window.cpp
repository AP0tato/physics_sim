#include "windows/menu_window.hpp"
#include "windows/main_window.hpp"
#include "windows/light_window.hpp"

namespace {

struct SimEntry
{
    const char *label;
    // Called when the user clicks — receives the theme and the launching MenuWindow
    std::function<void(Theme*, MenuWindow*)> launch;
};

const SimEntry sim_entries[] = {
    {
        "Physics Sim",
        [](Theme *t, MenuWindow *) {
            windows.push_back(new MainWindow(t));
        }
    },
    {
        "Light Sim",
        [](Theme *t, MenuWindow *) {
            windows.push_back(new LightWindow(t));
        }
    },
};

} // namespace

// ── Constructor ───────────────────────────────────────────────────────────────

MenuWindow::MenuWindow(Theme *theme)
    : Window("Simulation Launcher", 1920, 1080, theme)
{
    int w, h;
    get_size(w, h);

    constexpr int btn_w = 300;
    constexpr int btn_h = 70;
    constexpr int gap   = 20;

    const int n       = (int)(sizeof(sim_entries) / sizeof(sim_entries[0]));
    const int total_h = n * btn_h + (n - 1) * gap;
    int       btn_y   = (h - total_h) / 2;
    const int btn_x   = (w - btn_w)   / 2;

    for(int i = 0; i < n; i++)
    {
        auto launch = sim_entries[i].launch;

        auto *btn = new Button(btn_x, btn_y, btn_w, btn_h,
                               sim_entries[i].label,
                               [this, launch, theme]() { launch(theme, this); });
        register_button(btn);
        btn_y += btn_h + gap;
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void MenuWindow::register_button(Button *btn)
{
    int w, h;
    get_size(w, h);

    for(size_t i = 0; i < btn->corners.size(); i++)
    {
        btn->corners[i][0]    /= (float)w;  btn->corners[i][1]    /= (float)h;
        btn->base_shape[i][0] /= (float)w;  btn->base_shape[i][1] /= (float)h;
    }
    btn->create_hitbox();
    buttons.push_back(btn);
}

// ── main_loop ─────────────────────────────────────────────────────────────────

void MenuWindow::main_loop()
{
    int w, h;
    get_size(w, h);

    const SDL_Color title_color = {220, 220, 220, 255};
    draw_text("Select a Simulation",
              (float)w * 0.5f - 160.0f, 80.0f,
              title_color, 32);

    for(Button *btn : buttons)
        btn->draw_object(get_renderer(), theme, w, h);
}

// ── event_handler ─────────────────────────────────────────────────────────────

void MenuWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
       event.window.windowID == SDL_GetWindowID(get_window()))
    {
        for(Window *w : windows)
            w->running = false;
        return;
    }

    if(windows.size() > 1)
        return;

    int w, h;
    get_size(w, h);

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        for(Button *btn : buttons)
        {
            if(btn->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                btn->press();
                break;
            }
        }
    }
}