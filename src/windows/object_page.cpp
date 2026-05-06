#include "windows/object_page.hpp"
#include "windows/main_window.hpp"
#include "windows/light_window.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>

// ── Internal drawing helpers ──────────────────────────────────────────────────

namespace {

constexpr int   page_btn_x   = 30;
constexpr int   page_btn_y   = 30;
constexpr int   page_btn_gap = 14;

constexpr float POPUP_W      = 320.0f;
constexpr float POPUP_H_MASS = 200.0f;
constexpr float POPUP_H_MIRR = 210.0f;
constexpr float ROW_H        = 36.0f;
constexpr float SLIDER_H     = 12.0f;
constexpr float BTN_ROW_H    = 36.0f;

int half_w(Window *w) { int W=0,H=0; if(w) w->get_size(W,H); return W>0?W/2:640; }
int half_h(Window *w) { int W=0,H=0; if(w) w->get_size(W,H); return H>0?H/2:360; }

float clamp01(float t) { return t < 0.f ? 0.f : (t > 1.f ? 1.f : t); }

void fill_rect(SDL_Renderer *r, const SDL_FRect &rc,
               uint8_t red, uint8_t grn, uint8_t blu, uint8_t a=255)
{
    SDL_SetRenderDrawColor(r, red, grn, blu, a);
    SDL_RenderFillRect(r, &rc);
}
void stroke_rect(SDL_Renderer *r, const SDL_FRect &rc,
                 uint8_t red, uint8_t grn, uint8_t blu, uint8_t a=255)
{
    SDL_SetRenderDrawColor(r, red, grn, blu, a);
    SDL_RenderRect(r, &rc);
}

void draw_slider_bar(SDL_Renderer *r, const SDL_FRect &s, float t)
{
    fill_rect(r, s, 95, 95, 95);
    SDL_FRect filled = {s.x, s.y, s.w * clamp01(t), s.h};
    fill_rect(r, filled, 40, 200, 255);
    float tx = s.x + s.w * clamp01(t);
    SDL_FRect thumb = {tx - 5.f, s.y - 5.f, 10.f, s.h + 10.f};
    fill_rect(r, thumb, 240, 240, 240);
}

bool hit(const SDL_FRect &rc, int mx, int my)
{
    return mx >= (int)rc.x && mx <= (int)(rc.x+rc.w) &&
           my >= (int)rc.y && my <= (int)(rc.y+rc.h);
}

} // namespace

// ── Constructor / Destructor ──────────────────────────────────────────────────

ObjectPage::ObjectPage(Window *main_window_ptr)
    : Window("Object Page",
             half_w(main_window_ptr),
             half_h(main_window_ptr),
             main_window_ptr ? main_window_ptr->theme : nullptr)
    , ptr_main (dynamic_cast<MainWindow*> (main_window_ptr))
    , ptr_light(dynamic_cast<LightWindow*>(main_window_ptr))
{
    auto row = [](int n){ return page_btn_y + n*(BTN_HEIGHT + page_btn_gap); };

    auto add_btn = [&](int y, const char *label, std::function<void()> cb)
    {
        auto *btn = new Button(page_btn_x, y, BTN_WIDTH, BTN_HEIGHT, label, cb);
        normalize_button(btn);
        buttons.push_back(btn);
    };

    if(ptr_main)
    {
        add_btn(row(0), "Add Spring", [this]{ create_and_add(ObjectType::SPRING); });
        add_btn(row(1), "Add Mass",   [this]{ create_and_add(ObjectType::MASS);   });
        add_btn(row(2), "Add Plane",  [this]{ create_and_add(ObjectType::PLANE);  });
    }
    else if(ptr_light)
    {
        add_btn(row(0), "Add Wall",   [this]{ create_and_add(ObjectType::PLANE);  });
        add_btn(row(1), "Add Mass",   [this]{ open_popup(PendingType::MASS);      });
        add_btn(row(2), "Add Mirror", [this]{ open_popup(PendingType::MIRROR);    });
    }
}

ObjectPage::~ObjectPage()
{
    for(Button *b : buttons) delete b;
}

// ── Normalize button ──────────────────────────────────────────────────────────

void ObjectPage::normalize_button(Button *btn)
{
    int w, h;
    get_size(w, h);
    for(size_t i = 0; i < btn->corners.size(); i++)
    {
        btn->corners[i][0]    /= (float)w;   btn->corners[i][1]    /= (float)h;
        btn->base_shape[i][0] /= (float)w;   btn->base_shape[i][1] /= (float)h;
    }
    btn->create_hitbox();
}

// ── Popup lifecycle ───────────────────────────────────────────────────────────

void ObjectPage::open_popup(PendingType type)
{
    pending_type       = type;
    mass_mantissa      = 1.0f;
    mass_exponent      = 0;
    active_mass_slider = ActiveMassSlider::NONE;
    mirror_shape       = MirrorType::FLAT;
}

void ObjectPage::close_popup()
{
    pending_type       = PendingType::NONE;
    active_mass_slider = ActiveMassSlider::NONE;
}

void ObjectPage::confirm_and_place()
{
    if(pending_type == PendingType::MASS)
    {
        const float value = mass_mantissa * std::pow(10.0f, (float)mass_exponent);
        Object *obj = make_mass(value);
        if(ptr_main)  ptr_main->add_object(obj);
        if(ptr_light) ptr_light->add_object(obj);
    }
    else if(pending_type == PendingType::MIRROR)
    {
        Object *obj = make_mirror(mirror_shape);
        if(ptr_light) ptr_light->add_object(obj);
        if(ptr_main)  ptr_main->add_object(obj);
    }
    close_popup();
}

// ── Popup layout ──────────────────────────────────────────────────────────────

ObjectPage::PopupLayout ObjectPage::build_layout(int w, int h) const
{
    PopupLayout L{};

    const float popup_h = (pending_type == PendingType::MIRROR) ? POPUP_H_MIRR : POPUP_H_MASS;
    const float px = (float)w * 0.5f - POPUP_W * 0.5f;
    const float py = (float)h * 0.5f - popup_h * 0.5f;

    L.panel = {px, py, POPUP_W, popup_h};

    const float sx = px + 12.f;
    const float sw = POPUP_W - 60.f; // leave room for value label

    // Row 0: mantissa slider (0.0 – 9.99), row starts after title
    L.mantissa_slider = {sx, py + 44.f, sw, SLIDER_H};
    // Row 1: exponent slider (0 – 50)
    L.exponent_slider = {sx, py + 44.f + ROW_H, sw, SLIDER_H};

    // Mirror option buttons — three equal columns
    const float mbw = (POPUP_W - 36.f) / 3.f;
    const float mby = py + 52.f;
    L.mirror_flat    = {px + 12.f,               mby, mbw, 40.f};
    L.mirror_concave = {px + 12.f + mbw + 6.f,   mby, mbw, 40.f};
    L.mirror_convex  = {px + 12.f + (mbw+6.f)*2.f, mby, mbw, 40.f};

    // Confirm / Cancel at the bottom
    const float btn_y = py + popup_h - BTN_ROW_H - 12.f;
    const float half  = (POPUP_W - 36.f) * 0.5f;
    L.confirm_btn = {px + 12.f,        btn_y, half, BTN_ROW_H - 4.f};
    L.cancel_btn  = {px + 24.f + half, btn_y, half, BTN_ROW_H - 4.f};

    return L;
}

// ── Popup drawing ─────────────────────────────────────────────────────────────

void ObjectPage::draw_popup(int w, int h)
{
    auto L = build_layout(w, h);
    auto *r = get_renderer();
    const SDL_Color tc = {220, 220, 220, 255};

    fill_rect  (r, L.panel, 35, 35, 35, 235);
    stroke_rect(r, L.panel, 80, 170, 255);

    if(pending_type == PendingType::MASS)
    {
        draw_text("Configure Mass", L.panel.x + 12.f, L.panel.y + 10.f, tc, 14);

        // Mantissa
        draw_text("Mantissa (0 – 9.99)",
                  L.panel.x + 12.f, L.mantissa_slider.y - 18.f, tc, 12);
        draw_slider_bar(r, L.mantissa_slider, mass_mantissa / 9.99f);
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.2f", mass_mantissa);
        draw_text(buf, L.mantissa_slider.x + L.mantissa_slider.w + 8.f,
                  L.mantissa_slider.y - 2.f, tc, 13);

        // Exponent
        draw_text("× 10 ^  (0 – 50)",
                  L.panel.x + 12.f, L.exponent_slider.y - 18.f, tc, 12);
        draw_slider_bar(r, L.exponent_slider, (float)mass_exponent / 50.f);
        std::snprintf(buf, sizeof(buf), "%d", mass_exponent);
        draw_text(buf, L.exponent_slider.x + L.exponent_slider.w + 8.f,
                  L.exponent_slider.y - 2.f, tc, 13);

        // Result preview
        const float val = mass_mantissa * std::pow(10.f, (float)mass_exponent);
        std::snprintf(buf, sizeof(buf), "= %.3g kg", val);
        draw_text(buf, L.panel.x + 12.f,
                  L.exponent_slider.y + SLIDER_H + 10.f, {140,220,140,255}, 13);
    }
    else if(pending_type == PendingType::MIRROR)
    {
        draw_text("Choose Mirror Type", L.panel.x + 12.f, L.panel.y + 10.f, tc, 14);

        auto draw_opt = [&](const SDL_FRect &rc, const char *label, bool selected)
        {
            fill_rect  (r, rc, selected?55:28, selected?85:28, selected?115:28);
            stroke_rect(r, rc, selected?80:80, selected?170:100, selected?255:100);
            draw_text(label,
                      rc.x + rc.w*0.5f - (float)strlen(label)*3.5f,
                      rc.y + rc.h*0.5f - 7.f, tc, 13);
        };

        draw_opt(L.mirror_flat,    "Flat",    mirror_shape == MirrorType::FLAT);
        draw_opt(L.mirror_concave, "Concave", mirror_shape == MirrorType::CONCAVE);
        draw_opt(L.mirror_convex,  "Convex",  mirror_shape == MirrorType::CONVEX);

        const char *hint =
            mirror_shape == MirrorType::FLAT    ? "Reflects rays at equal angles" :
            mirror_shape == MirrorType::CONCAVE ? "Converges rays to a focal point" :
                                                   "Diverges rays outward";
        draw_text(hint, L.panel.x + 12.f,
                  L.mirror_flat.y + 48.f, {140, 180, 255, 255}, 12);
    }

    // Confirm / Cancel
    auto draw_action_btn = [&](const SDL_FRect &rc, const char *label,
                               uint8_t br, uint8_t bg, uint8_t bb)
    {
        fill_rect  (r, rc, 28, 28, 28);
        stroke_rect(r, rc, br, bg, bb);
        draw_text(label,
                  rc.x + rc.w*0.5f - (float)strlen(label)*3.5f,
                  rc.y + rc.h*0.5f - 7.f, tc, 13);
    };

    draw_action_btn(L.confirm_btn, "Add",    80,  200, 120);
    draw_action_btn(L.cancel_btn,  "Cancel", 200, 80,  80);
}

// ── Popup hit-testing ─────────────────────────────────────────────────────────

bool ObjectPage::handle_popup_click(int mx, int my, int w, int h)
{
    if(pending_type == PendingType::NONE) return false;
    auto L = build_layout(w, h);

    if(!hit(L.panel, mx, my)) { close_popup(); return true; }

    if(hit(L.confirm_btn, mx, my)) { confirm_and_place(); return true; }
    if(hit(L.cancel_btn,  mx, my)) { close_popup();       return true; }

    if(pending_type == PendingType::MASS)
    {
        if(hit(L.mantissa_slider, mx, my))
        {
            active_mass_slider = ActiveMassSlider::MANTISSA;
            mass_mantissa = clamp01(((float)mx - L.mantissa_slider.x) / L.mantissa_slider.w) * 9.99f;
            return true;
        }
        if(hit(L.exponent_slider, mx, my))
        {
            active_mass_slider = ActiveMassSlider::EXPONENT;
            mass_exponent = (int)std::round(
                clamp01(((float)mx - L.exponent_slider.x) / L.exponent_slider.w) * 50.f);
            return true;
        }
    }

    if(pending_type == PendingType::MIRROR)
    {
        if(hit(L.mirror_flat,    mx, my)) { mirror_shape = MirrorType::FLAT;    return true; }
        if(hit(L.mirror_concave, mx, my)) { mirror_shape = MirrorType::CONCAVE; return true; }
        if(hit(L.mirror_convex,  mx, my)) { mirror_shape = MirrorType::CONVEX;  return true; }
    }

    return true;
}

bool ObjectPage::handle_popup_motion(int mx, int my, int w, int h)
{
    if(pending_type != PendingType::MASS) return false;
    auto L = build_layout(w, h);

    if(active_mass_slider == ActiveMassSlider::MANTISSA)
    {
        mass_mantissa = clamp01(((float)mx - L.mantissa_slider.x) / L.mantissa_slider.w) * 9.99f;
        return true;
    }
    if(active_mass_slider == ActiveMassSlider::EXPONENT)
    {
        mass_exponent = (int)std::round(
            clamp01(((float)mx - L.exponent_slider.x) / L.exponent_slider.w) * 50.f);
        return true;
    }
    return false;
}

// ── main_loop ─────────────────────────────────────────────────────────────────

void ObjectPage::main_loop()
{
    int w, h;
    get_size(w, h);

    for(Button *btn : buttons)
        btn->draw_object(get_renderer(), theme, w, h);

    if(pending_type != PendingType::NONE)
        draw_popup(w, h);
}

// ── event_handler ─────────────────────────────────────────────────────────────

void ObjectPage::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    // Ignore mouse events that belong to a different window
    if((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.type == SDL_EVENT_MOUSE_BUTTON_UP   ||
        event.type == SDL_EVENT_MOUSE_MOTION)
       && event.window.windowID != SDL_GetWindowID(get_window()))
        return;

    int w, h;
    get_size(w, h);

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if(pending_type != PendingType::NONE)
        {
            handle_popup_click(event.button.x, event.button.y, w, h);
            return;
        }
        for(Button *btn : buttons)
            if(btn->is_mouse_click(event.button.x, event.button.y, w, h))
                btn->press();
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        active_mass_slider = ActiveMassSlider::NONE;

    if(event.type == SDL_EVENT_MOUSE_MOTION)
        handle_popup_motion(event.motion.x, event.motion.y, w, h);
}

// ── Object factories ──────────────────────────────────────────────────────────

void ObjectPage::create_and_add(ObjectType type)
{
    Object *obj = nullptr;
    if(type == ObjectType::SPRING) obj = make_spring();
    if(type == ObjectType::MASS)   obj = make_mass(1.0f);
    if(type == ObjectType::PLANE)  obj = make_plane();
    if(!obj) return;
    if(ptr_main)  ptr_main->add_object(obj);
    if(ptr_light) ptr_light->add_object(obj);
}

Spring* ObjectPage::make_spring()
{
    int w=1920, h=1080;
    if(ptr_main) ptr_main->get_size(w,h);
    const float bw=120.f, bh=40.f;
    const float x=w*0.5f-bw*0.5f, y=h*0.5f-bh*0.5f;
    return new Spring({{x,y},{x+bw,y},{x+bw,y+bh},{x,y+bh}},
                      25.f, false, 1.f, Orientation::UP);
}

Mass* ObjectPage::make_mass(float value)
{
    int w=1920, h=1080;
    if(ptr_main)  ptr_main->get_size(w,h);
    if(ptr_light) ptr_light->get_size(w,h);
    const float bw=60.f, bh=60.f;
    const float x=w*0.5f-bw*0.5f, y=h*0.5f-bh*0.5f;
    return new Mass({{x,y},{x+bw,y},{x+bw,y+bh},{x,y+bh}},
                    HitboxType::RECTANGLE, value);
}

Plane* ObjectPage::make_plane()
{
    int w=1920, h=1080;
    if(ptr_main) ptr_main->get_size(w,h);
    const float cx=w*0.5f, cy=h*0.5f, lw=320.f, th=6.f;
    return new Plane({{cx-lw*0.5f,cy-th*0.5f},{cx+lw*0.5f,cy-th*0.5f},
                      {cx+lw*0.5f,cy+th*0.5f},{cx-lw*0.5f,cy+th*0.5f}}, false);
}

Plane* ObjectPage::make_mirror(MirrorType shape)
{
    int w=1920, h=1080;
    if(ptr_light) ptr_light->get_size(w,h);
    const float cx=w*0.5f, cy=h*0.5f, lw=240.f, th=6.f;
    // All three are Plane placeholders for now.
    // Actual curved geometry will be added when the ray engine is built.
    (void)shape;
    return new Mirror(MirrorType::FLAT, {{cx-lw*0.5f,cy-th*0.5f},{cx+lw*0.5f,cy-th*0.5f},
                      {cx+lw*0.5f,cy+th*0.5f},{cx-lw*0.5f,cy+th*0.5f}});
}

Plane* ObjectPage::make_wall()
{
    int w=1920, h=1080;
    if(ptr_light) ptr_light->get_size(w,h);
    const float cx=w*0.5f, cy=h*0.5f, lw=200.f, th=20.f;
    return new Plane({{cx-lw*0.5f,cy-th*0.5f},{cx+lw*0.5f,cy-th*0.5f},
                      {cx+lw*0.5f,cy+th*0.5f},{cx-lw*0.5f,cy+th*0.5f}}, true);
}