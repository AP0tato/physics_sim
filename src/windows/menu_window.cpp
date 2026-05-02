#include "windows/menu_window.hpp"
#include "windows/main_window.hpp"
#include "windows/light_window.hpp"
#include "app_state.hpp"

// ── Constructor / Destructor ──────────────────────────────────────────────────

MenuWindow::MenuWindow(Theme *theme)
    : Window("Simulation Launcher", 1920, 1080, theme)
{
    int w, h;
    get_size(w, h);

    constexpr int btn_w = 300, btn_h = 70, gap = 20;

    struct Entry { const char *label; std::function<void()> cb; };
    const Entry entries[] = {
        { "Physics Sim", [theme]{ ::windows.push_back(new MainWindow(theme));  } },
        { "Light Sim",   [theme]{ ::windows.push_back(new LightWindow(theme)); } },
    };

    const int n       = (int)(sizeof(entries) / sizeof(entries[0]));
    const int total_h = n * btn_h + (n - 1) * gap;
    int       btn_y   = (h - total_h) / 2;
    const int btn_x   = (w - btn_w)   / 2;

    for(int i = 0; i < n; i++)
    {
        auto *btn = new Button(btn_x, btn_y, btn_w, btn_h,
                               entries[i].label, entries[i].cb);
        register_button(btn);
        btn_y += btn_h + gap;
    }
}

MenuWindow::~MenuWindow()
{
    for(Button *b : buttons) delete b;
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

    draw_text("Select a Simulation",
              (float)w * 0.5f - 160.f, 80.f,
              {220, 220, 220, 255}, 32);

    for(Button *btn : buttons)
        btn->draw_object(get_renderer(), theme, w, h);
}

// ── event_handler ─────────────────────────────────────────────────────────────

void MenuWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
       event.window.windowID == SDL_GetWindowID(get_window()))
    {
        int w, h;
        get_size(w, h);
        for(Button *btn : buttons)
            if(btn->is_mouse_click(event.button.x, event.button.y, w, h))
                { btn->press(); break; }
    }
}