#include "windows/main_window.hpp"
#include "windows/object_page.hpp"

#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

// ── Anonymous-namespace helpers (internal to this TU) ────────────────────────
namespace {

constexpr float selection_padding_px        = 10.0f;
constexpr float selection_handle_size       = 9.0f;
constexpr int   selection_handle_pick_radius = 10;
constexpr float property_popup_width        = 280.0f;
constexpr float property_popup_height_mass  = 90.0f;
constexpr float property_popup_height_spring= 186.0f;
constexpr float property_popup_margin       = 10.0f;
constexpr float property_popup_lift_px      = 20.0f;
constexpr float property_slider_height      = 10.0f;
constexpr float property_input_height       = 24.0f;
constexpr float property_checkbox_size      = 18.0f;

struct PropertyPopupRects
{
    SDL_FRect panel{};
    SDL_FRect slider_1{};
    SDL_FRect input_1{};
    SDL_FRect slider_2{};
    SDL_FRect input_2{};
    SDL_FRect checkbox{};
    SDL_FRect orientation_button{};
    SDL_FRect anchor_checkbox{};
};

// ── Geometry helpers ──────────────────────────────────────────────────────────

void get_rect_bounds(const Object *obj,
                     float &left, float &top, float &right, float &bottom)
{
    left = right = obj->corners[0][0];
    top = bottom  = obj->corners[0][1];

    for(const auto &c : obj->corners)
    {
        if(c[0] < left)   left   = c[0];
        if(c[0] > right)  right  = c[0];
        if(c[1] < top)    top    = c[1];
        if(c[1] > bottom) bottom = c[1];
    }
}

bool constrain_object_to_window(Object *obj, float &shift_x, float &shift_y)
{
    float left, top, right, bottom;
    get_rect_bounds(obj, left, top, right, bottom);

    shift_x = shift_y = 0.0f;

    if(left  < 0.0f) shift_x = -left;
    else if(right  > 1.0f) shift_x = 1.0f - right;

    if(top   < 0.0f) shift_y = -top;
    else if(bottom > 1.0f) shift_y = 1.0f - bottom;

    if(std::fabs(shift_x) < 1e-6f && std::fabs(shift_y) < 1e-6f)
        return false;

    for(size_t i = 0; i < obj->corners.size(); i++)
    {
        obj->corners[i][0]    += shift_x;  obj->corners[i][1]    += shift_y;
        obj->base_shape[i][0] += shift_x;  obj->base_shape[i][1] += shift_y;
    }
    return true;
}

void move_object_by_pixels(Object *obj, int dx, int dy, int w, int h)
{
    const float ddx = (float)dx / (float)w;
    const float ddy = (float)dy / (float)h;

    for(size_t i = 0; i < obj->corners.size(); i++)
    {
        obj->corners[i][0]    += ddx;  obj->corners[i][1]    += ddy;
        obj->base_shape[i][0] += ddx;  obj->base_shape[i][1] += ddy;
    }

    float sx, sy;
    constrain_object_to_window(obj, sx, sy);
    obj->create_hitbox();
}

// ── Numeric / string helpers ──────────────────────────────────────────────────

float clamp_value(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

float slider_value_from_mouse(int mouse_x, const SDL_FRect &slider,
                               float min_val, float max_val)
{
    if(slider.w <= 1.0f) return min_val;
    const float t = clamp_value(((float)mouse_x - slider.x) / slider.w, 0.0f, 1.0f);
    return min_val + (max_val - min_val) * t;
}

float slider_x_from_value(float value, const SDL_FRect &slider,
                           float min_val, float max_val)
{
    const float span = max_val - min_val;
    if(span <= 0.0f) return slider.x;
    const float t = clamp_value((value - min_val) / span, 0.0f, 1.0f);
    return slider.x + t * slider.w;
}

std::string format_value(float value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    return buf;
}

bool point_in_rect(int x, int y, const SDL_FRect &r)
{
    return x >= (int)r.x && y >= (int)r.y &&
           x <= (int)(r.x + r.w) && y <= (int)(r.y + r.h);
}

// ── Orientation helpers ───────────────────────────────────────────────────────

size_t next_orientation_index(Orientation o)
{
    return (static_cast<size_t>(o) + 1) % 4;
}

const char *orientation_text(Orientation o)
{
    switch(o)
    {
        case Orientation::UP:    return "UP";
        case Orientation::RIGHT: return "RIGHT";
        case Orientation::DOWN:  return "DOWN";
        case Orientation::LEFT:  return "LEFT";
        default:                 return "NONE";
    }
}

// ── SDL drawing primitives ────────────────────────────────────────────────────

void draw_slider(SDL_Renderer *r, const SDL_FRect &slider,
                 float value, float min_v, float max_v)
{
    SDL_SetRenderDrawColor(r, 95, 95, 95, 255);
    SDL_RenderFillRect(r, &slider);

    SDL_SetRenderDrawColor(r, 40, 200, 255, 255);
    SDL_FRect filled = slider;
    filled.w = slider_x_from_value(value, slider, min_v, max_v) - slider.x;
    if(filled.w < 0.0f) filled.w = 0.0f;
    SDL_RenderFillRect(r, &filled);

    const float thumb_x = slider_x_from_value(value, slider, min_v, max_v);
    SDL_FRect thumb = {thumb_x - 4.0f, slider.y - 4.0f, 8.0f, slider.h + 8.0f};
    SDL_SetRenderDrawColor(r, 240, 240, 240, 255);
    SDL_RenderFillRect(r, &thumb);
}

void draw_input_box(SDL_Renderer *r, const SDL_FRect &box, bool active)
{
    SDL_SetRenderDrawColor(r, 28, 28, 28, 255);
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, active ? 120 : 140, active ? 220 : 140, active ? 255 : 140, 255);
    SDL_RenderRect(r, &box);
}

void draw_property_popup(SDL_Renderer *r, const PropertyPopupRects &rects, bool for_spring)
{
    const SDL_Color border = for_spring
        ? SDL_Color{255, 120, 120, 255}
        : SDL_Color{120, 255, 160, 255};

    SDL_SetRenderDrawColor(r, 35, 35, 35, 235);
    SDL_RenderFillRect(r, &rects.panel);
    SDL_SetRenderDrawColor(r, border.r, border.g, border.b, border.a);
    SDL_RenderRect(r, &rects.panel);
}

// ── Selection / resize handles ────────────────────────────────────────────────

std::array<SDL_FPoint, 8> get_resize_handles_px(const Object *obj, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(obj, left, top, right, bottom);

    const float l  = left  * w - selection_padding_px;
    const float t  = top   * h - selection_padding_px;
    const float r  = right * w + selection_padding_px;
    const float b  = bottom* h + selection_padding_px;
    const float mx = (l + r) * 0.5f;
    const float my = (t + b) * 0.5f;

    // Clockwise from top-left: TL, TM, TR, MR, BR, BM, BL, ML
    return { SDL_FPoint{l,t}, {mx,t}, {r,t}, {r,my},
             SDL_FPoint{r,b}, {mx,b}, {l,b}, {l,my} };
}

bool try_get_resize_handle(const Object *obj, int mouse_x, int mouse_y,
                            int w, int h, size_t &handle_idx)
{
    constexpr int r2 = selection_handle_pick_radius * selection_handle_pick_radius;
    auto handles = get_resize_handles_px(obj, w, h);
    for(size_t i = 0; i < handles.size(); i++)
    {
        const int dx = mouse_x - (int)handles[i].x;
        const int dy = mouse_y - (int)handles[i].y;
        if(dx*dx + dy*dy <= r2) { handle_idx = i; return true; }
    }
    return false;
}

void draw_selection_frame(SDL_Renderer *r, const Object *obj, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(obj, left, top, right, bottom);

    SDL_SetRenderDrawColor(r, 80, 170, 255, 255);
    SDL_FRect frame = {
        left  * w - selection_padding_px,
        top   * h - selection_padding_px,
        (right - left) * w + 2.0f * selection_padding_px,
        (bottom - top) * h + 2.0f * selection_padding_px
    };
    SDL_RenderRect(r, &frame);

    for(const auto &handle : get_resize_handles_px(obj, w, h))
    {
        SDL_FRect dot = {
            handle.x - selection_handle_size * 0.5f,
            handle.y - selection_handle_size * 0.5f,
            selection_handle_size, selection_handle_size
        };
        SDL_RenderFillRect(r, &dot);
    }
}

// ── Object mutation ───────────────────────────────────────────────────────────

void set_rect_from_bounds(Object *obj,
                           float left, float top, float right, float bottom)
{
    obj->corners[0] = obj->base_shape[0] = {left,  top};
    obj->corners[1] = obj->base_shape[1] = {right, top};
    obj->corners[2] = obj->base_shape[2] = {right, bottom};
    obj->corners[3] = obj->base_shape[3] = {left,  bottom};
    obj->create_hitbox();
}

void resize_rect_object_handle(Object *obj, size_t handle_idx,
                                int dx, int dy, int w, int h)
{
    if(obj->corners.size() != 4) return;

    float left, top, right, bottom;
    get_rect_bounds(obj, left, top, right, bottom);

    const float ndx = (float)dx / (float)w;
    const float ndy = (float)dy / (float)h;

    bool ml = false, mr = false, mt = false, mb = false;
    switch(handle_idx)
    {
        case 0: ml = mt = true; break;
        case 1: mt = true;      break;
        case 2: mr = mt = true; break;
        case 3: mr = true;      break;
        case 4: mr = mb = true; break;
        case 5: mb = true;      break;
        case 6: ml = mb = true; break;
        case 7: ml = true;      break;
        default: return;
    }

    if(ml) left   += ndx;
    if(mr) right  += ndx;
    if(mt) top    += ndy;
    if(mb) bottom += ndy;

    constexpr float min_size_px = 16.0f;
    const float min_w = min_size_px / (float)w;
    const float min_h = min_size_px / (float)h;

    if(right - left < min_w)
    {
        if(ml && !mr) left  = right - min_w;
        else          right = left  + min_w;
    }
    if(bottom - top < min_h)
    {
        if(mt && !mb) top    = bottom - min_h;
        else          bottom = top    + min_h;
    }

    set_rect_from_bounds(obj, left, top, right, bottom);
}

// ── Index-set helpers (for object deletion) ───────────────────────────────────

void erase_and_reindex(std::unordered_set<size_t> &set, size_t removed)
{
    std::unordered_set<size_t> rebuilt;
    rebuilt.reserve(set.size());
    for(size_t idx : set)
    {
        if(idx == removed) continue;
        rebuilt.insert(idx > removed ? idx - 1 : idx);
    }
    set.swap(rebuilt);
}

void erase_and_reindex_ulong(std::unordered_set<unsigned long> &set, size_t removed)
{
    std::unordered_set<unsigned long> rebuilt;
    rebuilt.reserve(set.size());
    for(unsigned long idx : set)
    {
        if(idx == removed) continue;
        rebuilt.insert(idx > removed ? idx - 1 : idx);
    }
    set.swap(rebuilt);
}

// ── Property-popup rect layout ────────────────────────────────────────────────

PropertyPopupRects get_property_popup_rects(const Object *obj,
                                             int w, int h, bool for_spring)
{
    PropertyPopupRects rects;

    float left, top, right, bottom;
    get_rect_bounds(obj, left, top, right, bottom);

    const float left_px   = left   * w;
    const float top_px    = top    * h;
    const float right_px  = right  * w;
    const float bottom_px = bottom * h;
    const float popup_h   = for_spring
        ? property_popup_height_spring
        : property_popup_height_mass;

    float panel_x = (left_px + right_px) * 0.5f - property_popup_width * 0.5f;
    panel_x = clamp_value(panel_x, 8.0f, (float)w - property_popup_width - 8.0f);

    float panel_y = top_px - property_popup_margin - popup_h - property_popup_lift_px;
    if(panel_y < 8.0f) panel_y = bottom_px + property_popup_margin;
    panel_y = clamp_value(panel_y, 8.0f, (float)h - popup_h - 8.0f);

    rects.panel   = {panel_x, panel_y, property_popup_width, popup_h};

    const float row1 = panel_y + 16.0f;
    const float row2 = panel_y + 56.0f;

    rects.slider_1 = {panel_x + 74.0f, row1 + 7.0f, 130.0f, property_slider_height};
    rects.input_1  = {panel_x + 212.0f, row1,        58.0f,  property_input_height};

    if(for_spring)
    {
        rects.slider_2          = {panel_x + 74.0f,  row2 + 7.0f,   130.0f, property_slider_height};
        rects.input_2           = {panel_x + 212.0f, row2,           58.0f,  property_input_height};
        rects.checkbox          = {panel_x + 16.0f,  panel_y + 98.0f,  property_checkbox_size, property_checkbox_size};
        rects.orientation_button= {panel_x + 144.0f, panel_y + 126.0f, 126.0f, property_input_height};
        rects.anchor_checkbox   = {panel_x + 16.0f,  panel_y + 162.0f, property_checkbox_size, property_checkbox_size};
    }
    else
    {
        rects.anchor_checkbox   = {panel_x + 16.0f,  panel_y + 56.0f,  property_checkbox_size, property_checkbox_size};
    }

    return rects;
}

} // namespace

// ═════════════════════════════════════════════════════════════════════════════
// MainWindow
// ═════════════════════════════════════════════════════════════════════════════

MainWindow::MainWindow(Theme *theme)
    : Window("Physics Sim", 1920, 1080, theme)
{
    massless_checkbox = CheckBox(0, 0, (int)property_checkbox_size, false);
    anchor_checkbox   = CheckBox(0, 0, (int)property_checkbox_size, false);

    int w, h;
    get_size(w, h);

    constexpr int btn_w        = 150;
    constexpr int btn_h        = 50;
    constexpr int top_margin   = 10;
    constexpr int right_margin = 10;
    constexpr int button_gap   = 10;

    const int obj_page_x = w - btn_w - right_margin;
    const int obj_page_y = top_margin;
    const int play_x     = obj_page_x - btn_w - button_gap;

    play_button = new Button(play_x, top_margin, btn_w, btn_h,
                             "Play", [this]() { toggle_playing(); });
    add_object(play_button);

    add_object(new Button(obj_page_x, obj_page_y, btn_w, btn_h,
                          "Object Page",
                          [this]() { windows.push_back(new ObjectPage(this)); }));
}

// ── Physics ───────────────────────────────────────────────────────────────────

void MainWindow::step_gravity(Object *object, int w, int h)
{
    object->velocity_y += (float)(G * DELTA_T);
    const float displacement = object->velocity_y * DELTA_T;

    for(size_t j = 0; j < object->corners.size(); j++)
    {
        object->corners[j][1]    += displacement;
        object->base_shape[j][1] += displacement;
    }

    float sx, sy;
    if(constrain_object_to_window(object, sx, sy))
        if(std::fabs(sy) > 1e-6f)
            object->velocity_y = -object->velocity_y;

    object->create_hitbox();
}

void MainWindow::check_collisions()
{
    // Broad phase (AABB) + narrow phase (SAT/impulse) go here.
    // Left as a stub until collision response is implemented.
}

// ── Snapshot ──────────────────────────────────────────────────────────────────

void MainWindow::capture_runtime_snapshot()
{
    runtime_snapshot.clear();
    runtime_snapshot.reserve(objects.size());

    for(size_t i = 0; i < objects.size(); i++)
    {
        RuntimeSnapshot snap;
        snap.corners     = objects[i]->corners;
        snap.base_shape  = objects[i]->base_shape;
        snap.orientation = objects[i]->orientation;

        if(masses.count(i))
        {
            if(auto *m = dynamic_cast<Mass*>(objects[i]))
            {
                snap.mass_velocity_x = m->velocity_x;
                snap.mass_velocity_y = m->velocity_y;
            }
        }
        runtime_snapshot.push_back(std::move(snap));
    }

    has_runtime_snapshot = (runtime_snapshot.size() == objects.size());
}

void MainWindow::restore_runtime_snapshot()
{
    if(!has_runtime_snapshot || runtime_snapshot.size() != objects.size())
        return;

    for(size_t i = 0; i < objects.size(); i++)
    {
        objects[i]->corners     = runtime_snapshot[i].corners;
        objects[i]->base_shape  = runtime_snapshot[i].base_shape;
        objects[i]->orientation = runtime_snapshot[i].orientation;
        objects[i]->create_hitbox();

        if(masses.count(i))
        {
            if(auto *m = dynamic_cast<Mass*>(objects[i]))
            {
                m->velocity_x = runtime_snapshot[i].mass_velocity_x;
                m->velocity_y = runtime_snapshot[i].mass_velocity_y;
            }
        }
    }
}

// ── Playback control ──────────────────────────────────────────────────────────

void MainWindow::toggle_playing()
{
    if(!playing)
    {
        capture_runtime_snapshot();
        playing = animating = true;
    }
    else
    {
        playing = animating = false;
        restore_runtime_snapshot();
        dragging = resizing = has_selection = show_property_popup = false;
    }

    active_slider  = ActiveSlider::NONE;
    active_input   = ActiveInput::NONE;
    property_input.clear();
    SDL_StopTextInput(get_window());

    if(play_button)
        play_button->label.set_text(playing ? "Stop" : "Play");
}

// ── add_object ────────────────────────────────────────────────────────────────

void MainWindow::add_object(Object *object)
{
    int w, h;
    get_size(w, h);

    for(size_t i = 0; i < object->corners.size(); i++)
    {
        object->corners[i][0]    /= (float)w;  object->corners[i][1]    /= (float)h;
        object->base_shape[i][0] /= (float)w;  object->base_shape[i][1] /= (float)h;
    }
    object->create_hitbox();

    const size_t idx = objects.size();
    switch(object->type())
    {
        case ObjectType::BUTTON: buttons.insert(idx); break;
        case ObjectType::MASS:   masses.insert(idx);  break;
        case ObjectType::PLANE:  planes.insert((unsigned long)idx);  break;
        default: printf("add_object: unknown type\n"); break;
    }
    objects.push_back(object);
}

// ── main_loop ─────────────────────────────────────────────────────────────────

void MainWindow::main_loop()
{
    int w, h;
    get_size(w, h);

    // Physics step (decoupled from draw)
    if(playing && animating)
    {
        for(size_t i = 0; i < objects.size(); i++)
            if(!buttons.count(i) && !objects[i]->anchor)
                step_gravity(objects[i], w, h);

        check_collisions();
    }

    // Draw all objects
    for(size_t i = 0; i < objects.size(); i++)
        objects[i]->draw_object(get_renderer(), theme, w, h);

    // Selection frame
    if(!playing && has_selection && curr_object < objects.size()
       && !buttons.count(curr_object))
    {
        draw_selection_frame(get_renderer(), objects[curr_object], w, h);
    }

    // Property popup
    if(!playing && show_property_popup && has_selection
       && curr_object < objects.size() && !buttons.count(curr_object))
    {
        auto *spring     = dynamic_cast<Spring*>(objects[curr_object]);
        const bool is_spring = (spring != nullptr);
        PropertyPopupRects popup = get_property_popup_rects(objects[curr_object], w, h, is_spring);

        draw_property_popup(get_renderer(), popup, is_spring);

        const SDL_Color tc = {230, 230, 230, 255};

        if(masses.count(curr_object))
        {
            if(auto *mass = dynamic_cast<Mass*>(objects[curr_object]))
            {
                draw_text("mass", popup.panel.x + 12.0f, popup.input_1.y + 4.0f, tc);
                draw_slider(get_renderer(), popup.slider_1, mass->mass, 0.1f, 100.0f);
                draw_input_box(get_renderer(), popup.input_1, active_input == ActiveInput::MASS);
                draw_text(active_input == ActiveInput::MASS ? property_input : format_value(mass->mass),
                          popup.input_1.x + 6.0f, popup.input_1.y + 4.0f, tc);
            }
        }
        else if(is_spring)
        {
            draw_text("k", popup.panel.x + 12.0f, popup.input_1.y + 4.0f, tc);
            draw_slider(get_renderer(), popup.slider_1, spring->k_const, 1.0f, 300.0f);
            draw_input_box(get_renderer(), popup.input_1, active_input == ActiveInput::SPRING_K);
            draw_text(active_input == ActiveInput::SPRING_K ? property_input : format_value(spring->k_const),
                      popup.input_1.x + 6.0f, popup.input_1.y + 4.0f, tc);

            draw_text("mass", popup.panel.x + 12.0f, popup.input_2.y + 4.0f, tc);
            draw_slider(get_renderer(), popup.slider_2, spring->mass, 0.0f, 100.0f);
            draw_input_box(get_renderer(), popup.input_2, active_input == ActiveInput::SPRING_MASS);
            draw_text(active_input == ActiveInput::SPRING_MASS ? property_input : format_value(spring->mass),
                      popup.input_2.x + 6.0f, popup.input_2.y + 4.0f, tc);

            massless_checkbox.set_position((int)popup.checkbox.x, (int)popup.checkbox.y);
            massless_checkbox.set_size((int)popup.checkbox.w);
            massless_checkbox.set_checked(spring->massless);
            massless_checkbox.draw(get_renderer());
            draw_text("massless",
                      popup.checkbox.x + popup.checkbox.w + 8.0f,
                      popup.checkbox.y - 1.0f, tc);

            draw_text("red edge", popup.panel.x + 16.0f, popup.orientation_button.y + 4.0f, tc);
            draw_input_box(get_renderer(), popup.orientation_button, false);
            draw_text(orientation_text(spring->orientation),
                      popup.orientation_button.x + 8.0f,
                      popup.orientation_button.y + 4.0f, tc);
        }

        anchor_checkbox.set_position((int)popup.anchor_checkbox.x, (int)popup.anchor_checkbox.y);
        anchor_checkbox.set_size((int)popup.anchor_checkbox.w);
        anchor_checkbox.set_checked(objects[curr_object]->anchor);
        anchor_checkbox.draw(get_renderer());
        draw_text("anchor",
                  popup.anchor_checkbox.x + popup.anchor_checkbox.w + 8.0f,
                  popup.anchor_checkbox.y - 1.0f, tc);
    }
}

// ── event_handler ─────────────────────────────────────────────────────────────

void MainWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    int w, h;
    get_size(w, h);

    // Commit whatever is in the text input field
    auto commit_property_input = [&]()
    {
        if(active_input == ActiveInput::NONE || curr_object >= objects.size())
            return;

        if(property_input.empty() || property_input == "." || property_input == "-")
        {
            active_input = ActiveInput::NONE;
            property_input.clear();
            SDL_StopTextInput(get_window());
            return;
        }

        try
        {
            const float v = std::stof(property_input);
            if(active_input == ActiveInput::MASS && masses.count(curr_object))
            {
                if(auto *m = dynamic_cast<Mass*>(objects[curr_object]))
                    m->mass = clamp_value(v, 0.1f, 100.0f);
            }
            else if(active_input == ActiveInput::SPRING_K)
            {
                if(auto *s = dynamic_cast<Spring*>(objects[curr_object]))
                    s->k_const = clamp_value(v, 1.0f, 300.0f);
            }
            else if(active_input == ActiveInput::SPRING_MASS)
            {
                if(auto *s = dynamic_cast<Spring*>(objects[curr_object]))
                    s->mass = clamp_value(v, 0.0f, 100.0f);
            }
        }
        catch(...) {}

        active_input = ActiveInput::NONE;
        property_input.clear();
        SDL_StopTextInput(get_window());
    };

    // ── Keyboard: delete selected object ─────────────────────────────────
    if(!playing && has_selection && active_input == ActiveInput::NONE
       && curr_object < objects.size()
       && event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat
       && (event.key.key == SDLK_BACKSPACE || event.key.key == SDLK_DELETE))
    {
        delete objects[curr_object];
        objects.erase(objects.begin() + (long)curr_object);
        erase_and_reindex(masses,   curr_object);
        erase_and_reindex(buttons,  curr_object);
        erase_and_reindex_ulong(planes, curr_object);

        has_selection = show_property_popup = dragging = resizing = false;
        active_slider = ActiveSlider::NONE;
        property_input.clear();
        SDL_StopTextInput(get_window());
        return;
    }

    // ── Keyboard: text input into property field ──────────────────────────
    if(!playing && has_selection && active_input != ActiveInput::NONE
       && event.type == SDL_EVENT_TEXT_INPUT)
    {
        for(const char *p = event.text.text; *p; ++p)
        {
            const char c = *p;
            if(std::isdigit((unsigned char)c))
                property_input.push_back(c);
            else if(c == '.' && property_input.find('.') == std::string::npos)
                property_input.push_back(c);
            else if(c == '-' && property_input.empty())
                property_input.push_back(c);
        }
        return;
    }

    if(!playing && has_selection && active_input != ActiveInput::NONE
       && event.type == SDL_EVENT_KEY_DOWN)
    {
        switch(event.key.key)
        {
            case SDLK_BACKSPACE:
                if(!property_input.empty()) property_input.pop_back();
                return;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                commit_property_input();
                return;
            case SDLK_ESCAPE:
                active_input = ActiveInput::NONE;
                property_input.clear();
                SDL_StopTextInput(get_window());
                return;
            default: break;
        }
    }

    // ── Mouse button down ─────────────────────────────────────────────────
    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        bool hit_any = false;

        // Intercept popup clicks first
        if(!playing && has_selection && show_property_popup
           && curr_object < objects.size() && !buttons.count(curr_object))
        {
            auto *mass    = dynamic_cast<Mass*>(objects[curr_object]);
            auto *spring  = dynamic_cast<Spring*>(objects[curr_object]);
            const bool is_mass   = (mass   != nullptr);
            const bool is_spring = (spring != nullptr);
            PropertyPopupRects popup = get_property_popup_rects(objects[curr_object], w, h, is_spring);

            if(point_in_rect(event.button.x, event.button.y, popup.input_1))
            {
                active_slider = ActiveSlider::NONE;
                if(is_mass)   { active_input = ActiveInput::MASS;     property_input = format_value(mass->mass); }
                if(is_spring) { active_input = ActiveInput::SPRING_K; property_input = format_value(spring->k_const); }
                SDL_StartTextInput(get_window());
                return;
            }
            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.input_2))
            {
                active_slider = ActiveSlider::NONE;
                active_input  = ActiveInput::SPRING_MASS;
                property_input = format_value(spring->mass);
                SDL_StartTextInput(get_window());
                return;
            }
            if(point_in_rect(event.button.x, event.button.y, popup.slider_1))
            {
                active_input = ActiveInput::NONE; property_input.clear(); SDL_StopTextInput(get_window());
                if(is_mass)   { mass->mass      = slider_value_from_mouse(event.button.x, popup.slider_1, 0.1f, 100.0f);  active_slider = ActiveSlider::MASS; }
                if(is_spring) { spring->k_const  = slider_value_from_mouse(event.button.x, popup.slider_1, 1.0f, 300.0f); active_slider = ActiveSlider::SPRING_K; }
                return;
            }
            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.slider_2))
            {
                active_input = ActiveInput::NONE; property_input.clear(); SDL_StopTextInput(get_window());
                spring->mass  = slider_value_from_mouse(event.button.x, popup.slider_2, 0.0f, 100.0f);
                active_slider = ActiveSlider::SPRING_MASS;
                return;
            }
            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.checkbox))
            {
                massless_checkbox.set_position((int)popup.checkbox.x, (int)popup.checkbox.y);
                massless_checkbox.set_size((int)popup.checkbox.w);
                massless_checkbox.set_checked(spring->massless);
                if(massless_checkbox.hit_test(event.button.x, event.button.y))
                    massless_checkbox.toggle();
                spring->massless = massless_checkbox.is_checked();
                if(spring->massless) spring->mass = 0.0f;
                return;
            }
            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.orientation_button))
            {
                spring->orientation = static_cast<Orientation>(next_orientation_index(spring->orientation));
                return;
            }
            if(point_in_rect(event.button.x, event.button.y, popup.anchor_checkbox))
            {
                anchor_checkbox.set_position((int)popup.anchor_checkbox.x, (int)popup.anchor_checkbox.y);
                anchor_checkbox.set_size((int)popup.anchor_checkbox.w);
                anchor_checkbox.set_checked(objects[curr_object]->anchor);
                if(anchor_checkbox.hit_test(event.button.x, event.button.y))
                    anchor_checkbox.toggle();
                objects[curr_object]->anchor = anchor_checkbox.is_checked();
                return;
            }
            if(point_in_rect(event.button.x, event.button.y, popup.panel))
                return;
        }

        // Resize handles (can extend beyond object bounds in pause mode)
        if(!playing && has_selection && curr_object < objects.size()
           && !buttons.count(curr_object))
        {
            size_t hit_handle = 0;
            if(try_get_resize_handle(objects[curr_object], event.button.x, event.button.y, w, h, hit_handle))
            {
                hit_any = resizing = true;
                dragging = false;
                resize_handle = hit_handle;
                x_start = event.button.x; y_start = event.button.y;
                last_dx = last_dy = 0;
            }
        }

        if(hit_any) return;

        // Hit test all objects
        for(size_t i = 0; i < objects.size(); i++)
        {
            if(buttons.count(i) && objects[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                dynamic_cast<Button*>(objects[i])->press();
                hit_any = true;
                break;
            }
            if(!buttons.count(i) && objects[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                hit_any = true;
                x_start = event.button.x; y_start = event.button.y;
                curr_object = i;
                has_selection = true;
                last_dx = last_dy = 0;
                active_input = ActiveInput::NONE;
                property_input.clear();
                SDL_StopTextInput(get_window());

                float left, top, right, bottom;
                get_rect_bounds(objects[i], left, top, right, bottom);
                drag_anchor_x = event.button.x - (int)(left  * w);
                drag_anchor_y = event.button.y - (int)(top   * h);

                if(!playing)
                {
                    show_property_popup = (event.button.clicks >= 2);
                    size_t hh = 0;
                    resizing = try_get_resize_handle(objects[i], event.button.x, event.button.y, w, h, hh);
                    dragging = !resizing;
                    if(resizing) resize_handle = hh;
                }
                else
                {
                    dragging = true; resizing = false;
                }
                break;
            }
        }

        if(!hit_any)
        {
            has_selection = show_property_popup = false;
            active_slider = ActiveSlider::NONE;
            active_input  = ActiveInput::NONE;
            property_input.clear();
            SDL_StopTextInput(get_window());
        }
    }

    // ── Mouse button up ───────────────────────────────────────────────────
    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        dragging = resizing = false;
        active_slider = ActiveSlider::NONE;
        if(playing) animating = true;
    }

    // ── Mouse motion ──────────────────────────────────────────────────────
    if(event.type == SDL_EVENT_MOUSE_MOTION)
    {
        // Slider drag
        if(!playing && has_selection && curr_object < objects.size()
           && active_slider != ActiveSlider::NONE)
        {
            auto *spring   = dynamic_cast<Spring*>(objects[curr_object]);
            const bool is_spring = (spring != nullptr);
            PropertyPopupRects popup = get_property_popup_rects(objects[curr_object], w, h, is_spring);

            if(active_slider == ActiveSlider::MASS && masses.count(curr_object))
            {
                if(auto *m = dynamic_cast<Mass*>(objects[curr_object]))
                    m->mass = slider_value_from_mouse(event.motion.x, popup.slider_1, 0.1f, 100.0f);
            }
            else if(active_slider == ActiveSlider::SPRING_K && is_spring)
                spring->k_const = slider_value_from_mouse(event.motion.x, popup.slider_1, 1.0f, 300.0f);
            else if(active_slider == ActiveSlider::SPRING_MASS && is_spring)
                spring->mass = slider_value_from_mouse(event.motion.x, popup.slider_2, 0.0f, 100.0f);
            return;
        }

        // Drag / resize
        if((dragging || resizing) && curr_object < objects.size())
        {
            const int d_x = event.motion.x - (int)x_start;
            const int d_y = event.motion.y - (int)y_start;
            last_dx = d_x; last_dy = d_y;

            if(!playing && resizing)
            {
                resize_rect_object_handle(objects[curr_object], resize_handle, d_x, d_y, w, h);
            }
            else
            {
                if(!playing)
                {
                    float left, top, right, bottom;
                    get_rect_bounds(objects[curr_object], left, top, right, bottom);
                    move_object_by_pixels(objects[curr_object],
                        event.motion.x - drag_anchor_x - (int)(left * w),
                        event.motion.y - drag_anchor_y - (int)(top  * h),
                        w, h);
                }
                else
                {
                    move_object_by_pixels(objects[curr_object], d_x, d_y, w, h);
                }
            }

            x_start = event.motion.x;
            y_start = event.motion.y;
        }
    }
}