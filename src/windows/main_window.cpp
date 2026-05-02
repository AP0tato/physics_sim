#include "windows/main_window.hpp"
#include "windows/object_page.hpp"

#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

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

float clamp_value(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

float slider_value_from_mouse(int mouse_x, const SDL_FRect &slider, float min_val, float max_val)
{
    if(slider.w <= 1.0f) return min_val;
    const float t = clamp_value(((float)mouse_x - slider.x) / slider.w, 0.0f, 1.0f);
    return min_val + (max_val - min_val) * t;
}

float slider_x_from_value(float value, const SDL_FRect &slider, float min_val, float max_val)
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
    return x >= (int)r.x && y >= (int)r.y && x <= (int)(r.x + r.w) && y <= (int)(r.y + r.h);
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

void draw_slider(SDL_Renderer *r, const SDL_FRect &slider, float value, float min_v, float max_v)
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
    const SDL_Color border = for_spring ? SDL_Color{255, 120, 120, 255} : SDL_Color{120, 255, 160, 255};
    SDL_SetRenderDrawColor(r, 35, 35, 35, 235);
    SDL_RenderFillRect(r, &rects.panel);
    SDL_SetRenderDrawColor(r, border.r, border.g, border.b, border.a);
    SDL_RenderRect(r, &rects.panel);
}

std::array<SDL_FPoint, 8> get_resize_handles_px(const Object *obj, int w, int h)
{
    float left, top, right, bottom;
    obj->get_rect_bounds(left, top, right, bottom);
    const float l  = left  * w - selection_padding_px;
    const float t  = top   * h - selection_padding_px;
    const float r  = right * w + selection_padding_px;
    const float b  = bottom* h + selection_padding_px;
    const float mx = (l + r) * 0.5f;
    const float my = (t + b) * 0.5f;
    return { SDL_FPoint{l,t}, {mx,t}, {r,t}, {r,my}, SDL_FPoint{r,b}, {mx,b}, {l,b}, {l,my} };
}

bool try_get_resize_handle(const Object *obj, int mouse_x, int mouse_y, int w, int h, size_t &handle_idx)
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
    obj->get_rect_bounds(left, top, right, bottom);
    SDL_SetRenderDrawColor(r, 80, 170, 255, 255);
    SDL_FRect frame = { left*w-selection_padding_px, top*h-selection_padding_px, (right-left)*w+2*selection_padding_px, (bottom-top)*h+2*selection_padding_px };
    SDL_RenderRect(r, &frame);
    for(const auto &handle : get_resize_handles_px(obj, w, h))
    {
        SDL_FRect dot = { handle.x - selection_handle_size*0.5f, handle.y - selection_handle_size*0.5f, selection_handle_size, selection_handle_size };
        SDL_RenderFillRect(r, &dot);
    }
}

void set_rect_from_bounds(Object *obj, float left, float top, float right, float bottom)
{
    obj->corners[0] = obj->base_shape[0] = {left,  top};
    obj->corners[1] = obj->base_shape[1] = {right, top};
    obj->corners[2] = obj->base_shape[2] = {right, bottom};
    obj->corners[3] = obj->base_shape[3] = {left,  bottom};
    obj->create_hitbox();
}

void resize_rect_object_handle(Object *obj, size_t handle_idx, int dx, int dy, int w, int h)
{
    if(obj->corners.size() != 4) return;
    float left, top, right, bottom;
    obj->get_rect_bounds(left, top, right, bottom);
    const float ndx = (float)dx / (float)w, ndy = (float)dy / (float)h;
    bool ml = false, mr = false, mt = false, mb = false;
    switch(handle_idx) {
        case 0: ml = mt = true; break; case 1: mt = true; break; case 2: mr = mt = true; break;
        case 3: mr = true; break; case 4: mr = mb = true; break; case 5: mb = true; break;
        case 6: ml = mb = true; break; case 7: ml = true; break;
    }
    if(ml) left += ndx; if(mr) right += ndx; if(mt) top += ndy; if(mb) bottom += ndy;
    const float min_w = 16.0f/(float)w, min_h = 16.0f/(float)h;
    if(right-left < min_w) { if(ml && !mr) left = right-min_w; else right = left+min_w; }
    if(bottom-top < min_h) { if(mt && !mb) top = bottom-min_h; else bottom = top+min_h; }
    set_rect_from_bounds(obj, left, top, right, bottom);
}

PropertyPopupRects get_property_popup_rects(const Object *obj, int w, int h, bool for_spring)
{
    PropertyPopupRects rects;
    float left, top, right, bottom;
    obj->get_rect_bounds(left, top, right, bottom);
    const float lpx = left*w, tpx = top*h, rpx = right*w, bpx = bottom*h;
    const float ph = for_spring ? property_popup_height_spring : property_popup_height_mass;
    float px = (lpx+rpx)*0.5f - property_popup_width*0.5f;
    px = clamp_value(px, 8.0f, (float)w-property_popup_width-8.0f);
    float py = tpx - property_popup_margin - ph - property_popup_lift_px;
    if(py < 8.0f) py = bpx + property_popup_margin;
    py = clamp_value(py, 8.0f, (float)h-ph-8.0f);
    rects.panel = {px, py, property_popup_width, ph};
    rects.slider_1 = {px+74, py+23, 130, property_slider_height};
    rects.input_1  = {px+212, py+16, 58, property_input_height};
    if(for_spring) {
        rects.slider_2 = {px+74, py+63, 130, property_slider_height};
        rects.input_2  = {px+212, py+56, 58, property_input_height};
        rects.checkbox = {px+16, py+98, property_checkbox_size, property_checkbox_size};
        rects.orientation_button = {px+144, py+126, 126, property_input_height};
        rects.anchor_checkbox = {px+16, py+162, property_checkbox_size, property_checkbox_size};
    } else {
        rects.anchor_checkbox = {px+16, py+56, property_checkbox_size, property_checkbox_size};
    }
    return rects;
}

} // namespace

MainWindow::MainWindow(Theme *theme) : Window("Physics Sim", 1920, 1080, theme)
{
    massless_checkbox = CheckBox(0,0,(int)property_checkbox_size, false);
    anchor_checkbox = CheckBox(0,0,(int)property_checkbox_size, false);
    int w, h; get_size(w, h);
    play_button = new Button(w-320, 10, 150, 50, "Play", [this](){ toggle_playing(); });
    add_object(play_button);
    add_object(new Button(w-160, 10, 150, 50, "Object Page", [this](){ child_windows.push_back(new ObjectPage(this)); }));
}

MainWindow::~MainWindow() { for(Window *w : child_windows) delete w; for(Object *o : objects) delete o; }

void MainWindow::step_gravity(Object *obj)
{
    obj->velocity_y += (float)(G * DELTA_T);
    const float disp = obj->velocity_y * DELTA_T;
    for(size_t j=0; j<obj->corners.size(); j++) { obj->corners[j][1] += disp; obj->base_shape[j][1] += disp; }
    float sx, sy; if(obj->constrain_object_to_window(sx, sy)) if(std::fabs(sy) > 1e-6f) obj->velocity_y = -obj->velocity_y;
    obj->create_hitbox();
}

void MainWindow::add_object(Object *object)
{
    const size_t idx = Window::add_object(object);
    if(object->type() == ObjectType::BUTTON) buttons.insert(idx);
    else if(object->type() == ObjectType::MASS) masses.insert(idx);
    else if(object->type() == ObjectType::PLANE) planes.insert(idx);
    
    objects.push_back(object);
    
    curr_object = idx;
}

void MainWindow::toggle_playing()
{
    playing = !playing; animating = playing;
    if(!playing) { dragging = resizing = has_selection = show_property_popup = false; }
    if(play_button) play_button->label.set_text(playing ? "Stop" : "Play");
}

void MainWindow::main_loop()
{
    int w, h; get_size(w, h);
    if(playing && animating) 
    {
        for(size_t i=0; i<objects.size(); i++) if(!buttons.count(i) && !objects[i]->anchor) step_gravity(objects[i]);
    }
    for(size_t i=0; i<objects.size(); i++) 
        objects[i]->draw_object(get_renderer(), theme, w, h);
    if(!playing && has_selection && curr_object < objects.size() && !buttons.count(curr_object)) 
    {
        draw_selection_frame(get_renderer(), objects[curr_object], w, h);
        if(show_property_popup) {
            auto *s = dynamic_cast<Spring*>(objects[curr_object]);
            const bool is_s = (s != nullptr);
            PropertyPopupRects p = get_property_popup_rects(objects[curr_object], w, h, is_s);
            draw_property_popup(get_renderer(), p, is_s);
            const SDL_Color tc = {230, 230, 230, 255};
            if(masses.count(curr_object)) {
                auto *m = dynamic_cast<Mass*>(objects[curr_object]);
                draw_text("mass", p.panel.x+12, p.input_1.y+4, tc);
                draw_slider(get_renderer(), p.slider_1, m->mass, 0.1f, 100.0f);
                draw_input_box(get_renderer(), p.input_1, active_input == ActiveInput::MASS);
                draw_text(active_input == ActiveInput::MASS ? property_input : format_value(m->mass), p.input_1.x+6, p.input_1.y+4, tc);
            }
            anchor_checkbox.set_position((int)p.anchor_checkbox.x, (int)p.anchor_checkbox.y);
            anchor_checkbox.set_checked(objects[curr_object]->anchor);
            anchor_checkbox.draw(get_renderer());
            draw_text("anchor", p.anchor_checkbox.x+26, p.anchor_checkbox.y-1, tc);
        }
    }
    for(Window *cw : child_windows) { cw->clear_window(&cw->theme->background); cw->main_loop(); cw->render(); }
}

void MainWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    for(auto it = child_windows.begin(); it != child_windows.end(); ) {
        (*it)->event_handler(event);
        if(!(*it)->running) { delete *it; it = child_windows.erase(it); } else ++it;
    }
    int w, h; get_size(w, h);

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        bool hit_any = false;
        if(!playing && has_selection && show_property_popup && curr_object < objects.size()) {
            PropertyPopupRects p = get_property_popup_rects(objects[curr_object], w, h, dynamic_cast<Spring*>(objects[curr_object]) != nullptr);
            if(point_in_rect(event.button.x, event.button.y, p.panel)) return;
        }
        for(size_t i=0; i<objects.size(); i++) {
            if(objects[i]->is_mouse_click(event.button.x, event.button.y, w, h)) {
                if(buttons.count(i)) { dynamic_cast<Button*>(objects[i])->press(); hit_any = true; break; }
                hit_any = true; curr_object = i; has_selection = true;
                
                // CRITICAL FIX: Only show popup if it's a double-click (clicks >= 2)
                show_property_popup = (event.button.clicks >= 2); 
                
                x_start = event.button.x; y_start = event.button.y;
                float l, t, r, b; objects[i]->get_rect_bounds(l, t, r, b);
                drag_anchor_x = event.button.x - (int)(l*w); drag_anchor_y = event.button.y - (int)(t*h);
                size_t hh; resizing = try_get_resize_handle(objects[i], event.button.x, event.button.y, w, h, hh);
                if(resizing) resize_handle = hh; dragging = !resizing;
                break;
            }
        }
        if(!hit_any) { has_selection = show_property_popup = false; }
    }
    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP) dragging = resizing = false;
    if(event.type == SDL_EVENT_MOUSE_MOTION && (dragging || resizing)) {
        if(resizing) resize_rect_object_handle(objects[curr_object], resize_handle, event.motion.x-(int)x_start, event.motion.y-(int)y_start, w, h);
        else objects[curr_object]->move_object_by_pixels(event.motion.x-(int)x_start, event.motion.y-(int)y_start, w, h);
        x_start = event.motion.x; y_start = event.motion.y;
    }
}