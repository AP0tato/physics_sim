#include "windows/property_popup.hpp"
#include "objects/slider.hpp"
#include "objects/checkbox.hpp"
#include "objects/textfield.hpp"
#include "objects/togglebox.hpp"
#include "objects/button.hpp"
#include "objects/light.hpp"
#include "objects/laser.hpp"
#include "objects/lightray.hpp"
#include "objects/mirror.hpp"
#include "objects/wall.hpp"
#include "objects/mass.hpp"
#include "objects/spring.hpp"

#include <map>
#include <cmath>
#include <cstdio>
#include <SDL3_ttf/SDL_ttf.h>

// =============================================================================
// Local helpers
// =============================================================================
namespace
{
    const float POP_WIDTH    = 300.0f;
    const float ROW_H        = 32.0f;
    const float PAD          = 8.0f;
    const float TITLE_H      = 28.0f;

    float clamp_val(float v, float lo, float hi){ return v<lo?lo:(v>hi?hi:v); }

    TTF_Font *get_font(int size)
    {
        static std::map<int,TTF_Font*> cache;
        auto it = cache.find(size);
        if(it != cache.end()) return it->second;
        static const char *paths[] = {
            "assets/fonts/Roboto-Regular.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/System/Library/Fonts/Supplemental/Helvetica.ttc",
            "C:/Windows/Fonts/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            nullptr
        };
        TTF_Font *f = nullptr;
        for(int i = 0; paths[i]; ++i){ f = TTF_OpenFont(paths[i], size); if(f) break; }
        cache[size] = f;
        return f;
    }

    void render_text(SDL_Renderer *r, const std::string &text,
                     float x, float y, SDL_Color col, int size = 12)
    {
        TTF_Font *f = get_font(size);
        if(!f) return;
        SDL_Surface *sur = TTF_RenderText_Blended(f, text.c_str(), 0, col);
        if(!sur) return;
        SDL_Texture *tex = SDL_CreateTextureFromSurface(r, sur);
        if(tex){
            SDL_FRect dst = {x, y, (float)sur->w, (float)sur->h};
            SDL_RenderTexture(r, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(sur);
    }

    // Dummy corners — widgets are positioned by on_property_popup_load, not by corners
    static const std::vector<std::vector<float>> DC = {{0,0},{1,0},{1,1},{0,1}};

    Slider *make_slider(const std::string &lbl, float mn, float mx, float val)
    {
        auto *s = new Slider(DC, HitboxType::RECTANGLE, Orientation::NONE, mn, mx, val);
        s->set_label(lbl);
        return s;
    }
    ToggleBox *make_toggle(const std::string &lbl,
                           const std::vector<std::string> &opts, size_t idx)
    {
        auto *t = new ToggleBox(DC, HitboxType::RECTANGLE, Orientation::NONE, opts, idx);
        t->set_label(lbl);
        return t;
    }
    CheckBox *make_check(const std::string &lbl, bool checked)
    {
        auto *c = new CheckBox(DC, HitboxType::RECTANGLE, Orientation::NONE, checked);
        c->set_label(lbl);
        return c;
    }
    TextField *make_field(const std::string &lbl, const std::string &val)
    {
        auto *tf = new TextField(DC, HitboxType::RECTANGLE, Orientation::NONE, val);
        tf->set_label(lbl);
        return tf;
    }
}

// =============================================================================
// Construction / destruction
// =============================================================================
PropertyPopup::PropertyPopup()
{
    theme = new Theme();
    theme->background = Color::Color{30, 30, 30, 245};
    theme->foreground = Color::WHITE;
    theme->border     = Color::Color{80, 160, 255, 255};
    theme->active     = Color::LIGHT_GRAY;
}

PropertyPopup::PropertyPopup(Theme *t) : theme(t) {}

PropertyPopup::~PropertyPopup() { clear_owned(); }

void PropertyPopup::clear_owned()
{
    if(owns_options)
        for(Object *o : options) delete o;
    options.clear();
    owns_options = false;
}

// =============================================================================
// load_for_object — auto-builds widgets for every known object type
// =============================================================================
void PropertyPopup::load_for_object(Object *obj, int w, int h)
{
    clear_owned();
    owns_options = true;
    window_w = w;
    window_h = h;

    switch(obj->type())
    {
    // ── LightSource ───────────────────────────────────────────────────────
    case ObjectType::LIGHT_SOURCE:
    {
        title = "Light Source";
        auto *src = static_cast<LightSource*>(obj);

        auto *tb = make_toggle("emission", {"Linear","Radial"}, src->radial ? 1 : 0);
        tb->set_on_change([src](size_t i, const std::string &)
            { src->radial = (i == 1); });
        options.push_back(tb);

        auto *sl_dir = make_slider("direction°", 0, 360, src->angle_deg);
        sl_dir->set_on_change([src](float v){ src->angle_deg = v; });
        options.push_back(sl_dir);

        auto *sl_strength = make_slider("rays (0=∞)", 0, 256, (float)src->strength);
        sl_strength->set_integer_mode(true);
        sl_strength->set_on_change([src](float v){ src->strength = (int)std::round(v); });
        options.push_back(sl_strength);

        auto *sl_angle = make_slider("spread", 0, 360, src->emission_angle);
        sl_angle->set_on_change([src](float v){ src->emission_angle = v; });
        options.push_back(sl_angle);
        break;
    }

    // ── Laser ─────────────────────────────────────────────────────────────
    case ObjectType::LASER:
    {
        title = "Laser";
        auto *laser = static_cast<Laser*>(obj);

        auto *sl_angle = make_slider("angle°", 0, 360, laser->angle_deg);
        sl_angle->set_on_change([laser](float v){ laser->angle_deg = v; });
        options.push_back(sl_angle);

        // Color hex text field — commit parses RRGGBB
        auto *tf_col = make_field("color (RRGGBB)", laser->ray_color_hex);
        tf_col->set_on_commit([laser](const std::string &s)
        {
            if(s.size() < 6) return;
            unsigned int r=0,g=0,b=0;
            std::sscanf(s.c_str(), "%02x%02x%02x", &r, &g, &b);
            laser->ray_color     = {(Uint8)r,(Uint8)g,(Uint8)b,255};
            laser->ray_color_hex = s.substr(0,6);
        });
        options.push_back(tf_col);
        break;
    }

    // ── LightRay ──────────────────────────────────────────────────────────
    case ObjectType::LIGHT_RAY:
    {
        title = "Light Ray";
        auto *ray = static_cast<LightRay*>(obj);

        auto *sl_angle = make_slider("angle°", 0, 360, ray->angle_deg);
        sl_angle->set_on_change([ray](float v){ ray->angle_deg = v; });
        options.push_back(sl_angle);

        break;
    }

    // ── Mirror ────────────────────────────────────────────────────────────
    case ObjectType::MIRROR:
    {
        title = "Mirror";
        auto *m = static_cast<Mirror*>(obj);

        auto *tb = make_toggle("type", {"Flat","Concave","Convex"},
                               m->mirror_type == FLAT    ? 0 :
                               m->mirror_type == CONCAVE ? 1 : 2);
        tb->set_on_change([m](size_t i, const std::string &)
        {
            m->mirror_type = (i==0) ? FLAT : (i==1) ? CONCAVE : CONVEX;
        });
        options.push_back(tb);

        auto *sl_curv = make_slider("concavity", 0, 1, m->concavity);
        sl_curv->set_on_change([m](float v){ m->concavity = v; });
        options.push_back(sl_curv);
        break;
    }

    // ── Wall ──────────────────────────────────────────────────────────────
    case ObjectType::WALL:
    {
        title = "Wall";
        auto *wall = static_cast<Wall*>(obj);

        auto *tb = make_toggle("orientation", {"Horizontal","Vertical"},
                               wall->is_vertical() ? 1 : 0);
        tb->set_on_change([wall](size_t i, const std::string &)
            { wall->set_vertical(i == 1); });
        options.push_back(tb);
        break;
    }

    // ── Mass ─────────────────────────────────────────────────────────────
    case ObjectType::MASS:
    {
        title = "Mass";
        auto *po = static_cast<PhysicsObject*>(obj);

        auto *sl_mass = make_slider("mass (kg)", 0.1f, 100, po->mass);
        sl_mass->set_on_change([po](float v){ po->mass = v; });
        options.push_back(sl_mass);

        auto *sl_rest = make_slider("restitution", 0, 1, po->restitution);
        sl_rest->set_on_change([po](float v){ po->restitution = v; });
        options.push_back(sl_rest);

        auto *sl_fric = make_slider("friction", 0, 1, po->friction);
        sl_fric->set_on_change([po](float v){ po->friction = v; });
        options.push_back(sl_fric);

        auto *cb_grav = make_check("affected by gravity", po->affected_by_gravity);
        cb_grav->set_on_toggle([po](bool b){ po->affected_by_gravity = b; });
        options.push_back(cb_grav);
        break;
    }

    // ── Spring ────────────────────────────────────────────────────────────
    case ObjectType::SPRING:
    {
        title = "Spring";
        auto *sp = static_cast<Spring*>(obj);

        auto *sl_k = make_slider("stiffness k", 0.1f, 200, sp->k_const);
        sl_k->set_on_change([sp](float v){ sp->k_const = v; });
        options.push_back(sl_k);

        auto *cb_ml = make_check("massless", sp->massless);
        cb_ml->set_on_toggle([sp](bool b){ sp->massless = b; });
        options.push_back(cb_ml);
        break;
    }

    default:
        title = "Object";
        break;
    }

    // ── Position popup relative to object ─────────────────────────────────
    float left, top, right, bottom;
    obj->get_rect_bounds(left, top, right, bottom);
    const float lpx = left*w, tpx = top*h, rpx = right*w, bpx = bottom*h;

    const float ph = TITLE_H + PAD +
                     (float)options.size() * (ROW_H + PAD) + PAD;

    float px = (lpx + rpx) * 0.5f - POP_WIDTH * 0.5f;
    px = clamp_val(px, 8.0f, (float)w - POP_WIDTH - 8.0f);
    float py = tpx - PAD - ph;
    if(py < 8.0f) py = bpx + PAD;
    py = clamp_val(py, 8.0f, (float)h - ph - 8.0f);

    panel = {px, py, POP_WIDTH, ph};
    layout_options(px, py);
}

// =============================================================================
// Legacy load — caller owns the widgets
// =============================================================================
void PropertyPopup::load(const Object *obj, std::vector<Object*> opts, int w, int h)
{
    clear_owned();
    owns_options = false;
    options  = opts;
    window_w = w;
    window_h = h;
    title    = "Properties";

    float left, top, right, bottom;
    obj->get_rect_bounds(left, top, right, bottom);
    const float lpx = left*w, tpx = top*h, rpx = right*w, bpx = bottom*h;

    const float ph = TITLE_H + PAD +
                     (float)options.size() * (ROW_H + PAD) + PAD;
    float px = (lpx + rpx) * 0.5f - POP_WIDTH * 0.5f;
    px = clamp_val(px, 8.0f, (float)w - POP_WIDTH - 8.0f);
    float py = tpx - PAD - ph;
    if(py < 8.0f) py = bpx + PAD;
    py = clamp_val(py, 8.0f, (float)h - ph - 8.0f);

    panel = {px, py, POP_WIDTH, ph};
    layout_options(px, py);
}

// =============================================================================
// Layout
// =============================================================================
void PropertyPopup::layout_options(float px, float py)
{
    float cur_y = py + TITLE_H + PAD;
    const float cx  = px + PAD;
    const float cw  = POP_WIDTH - 2.0f * PAD;

    for(Object *o : options)
    {
        if(Slider    *s  = dynamic_cast<Slider*>(o))
            s->on_property_popup_load(cx, cur_y, cw, ROW_H);
        else if(CheckBox  *cb = dynamic_cast<CheckBox*>(o))
            cb->on_property_popup_load(cx, cur_y, cw, ROW_H);
        else if(TextField *tf = dynamic_cast<TextField*>(o))
            tf->on_property_popup_load(cx, cur_y, cw, ROW_H);
        else if(ToggleBox *tb = dynamic_cast<ToggleBox*>(o))
            tb->on_property_popup_load(cx, cur_y, cw, ROW_H);

        cur_y += ROW_H + PAD;
    }
}

// =============================================================================
// Draw
// =============================================================================
void PropertyPopup::draw(SDL_Renderer *renderer)
{
    draw_panel(renderer);
    draw_title(renderer);
    for(Object *o : options)
        if(o) o->draw_object(renderer, theme, window_w, window_h);
}

void PropertyPopup::draw_panel(SDL_Renderer *renderer)
{
    // Drop shadow
    SDL_FRect shadow = {panel.x+3, panel.y+3, panel.w, panel.h};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(renderer, &shadow);

    // Body
    SDL_SetRenderDrawColor(renderer,
        theme->background.r, theme->background.g,
        theme->background.b, theme->background.a);
    SDL_RenderFillRect(renderer, &panel);

    // Title bar
    SDL_FRect title_bar = {panel.x, panel.y, panel.w, TITLE_H};
    SDL_SetRenderDrawColor(renderer,
        theme->border.r, theme->border.g, theme->border.b, 180);
    SDL_RenderFillRect(renderer, &title_bar);

    // Border
    SDL_SetRenderDrawColor(renderer,
        theme->border.r, theme->border.g, theme->border.b, 255);
    SDL_RenderRect(renderer, &panel);
}

void PropertyPopup::draw_title(SDL_Renderer *renderer)
{
    SDL_Color col = {0, 0, 0, 255};
    render_text(renderer, title, panel.x + PAD, panel.y + 7.0f, col, 13);
}

// =============================================================================
// Event handling
// =============================================================================
bool PropertyPopup::contains(int x, int y) const
{
    return x >= (int)panel.x && y >= (int)panel.y &&
           x <= (int)(panel.x + panel.w) &&
           y <= (int)(panel.y + panel.h);
}

bool PropertyPopup::handle_event(SDL_Event &event)
{
    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if(!contains(event.button.x, event.button.y))
            return false;

        for(Object *o : options)
        {
            if(Slider    *s  = dynamic_cast<Slider*>(o))
                { if(s->handle_mouse_down(event.button.x, event.button.y)) return true; }
            else if(CheckBox  *cb = dynamic_cast<CheckBox*>(o))
                { if(cb->hit_test(event.button.x, event.button.y)){ cb->toggle(); return true; } }
            else if(ToggleBox *tb = dynamic_cast<ToggleBox*>(o))
                { if(tb->handle_mouse_down(event.button.x, event.button.y)) return true; }
            else if(TextField *tf = dynamic_cast<TextField*>(o))
            {
                bool hit = tf->hit_test(event.button.x, event.button.y);
                tf->set_active(hit);
                if(hit) return true;
            }
            else if(Button *btn = dynamic_cast<Button*>(o))
            {
                // Buttons store pixel coords in x/y/w/h — check manually
                // (Button doesn't have a public hit_test but has press())
                // We rely on the object's is_mouse_click via the panel check above.
                (void)btn;
            }
        }
        return true; // swallow clicks inside panel
    }

    if(event.type == SDL_EVENT_MOUSE_MOTION)
    {
        bool handled = false;
        for(Object *o : options)
            if(Slider *s = dynamic_cast<Slider*>(o))
                handled = s->handle_mouse_motion(event.motion.x) || handled;
        return handled;
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        for(Object *o : options)
            if(Slider *s = dynamic_cast<Slider*>(o))
                s->handle_mouse_up();
        return false;
    }

    if(event.type == SDL_EVENT_TEXT_INPUT)
    {
        for(Object *o : options)
            if(TextField *tf = dynamic_cast<TextField*>(o))
                if(tf->is_active())
                    { for(const char *c = event.text.text; *c; ++c) tf->append_char(*c); return true; }
    }

    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        for(Object *o : options)
        {
            if(TextField *tf = dynamic_cast<TextField*>(o))
            {
                if(tf->is_active())
                {
                    if(event.key.key == SDLK_BACKSPACE)                            { tf->backspace(); return true; }
                    if(event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER) { tf->commit(); tf->set_active(false); return true; }
                    if(event.key.key == SDLK_ESCAPE)                               { tf->set_active(false); return true; }
                }
            }
        }
    }

    return false;
}

std::string PropertyPopup::format_value(float value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    return buf;
}