#include "windows/main_window.hpp"
#include "windows/object_page.hpp"
#include "objects/slider.hpp"
#include "objects/checkbox.hpp"
#include "objects/togglebox.hpp"

#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

namespace {

float clamp_value(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

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

} // namespace

MainWindow::MainWindow(Theme *theme) : Window("Physics Sim", 1920, 1080, theme)
{
    int w, h; get_size(w, h);
    play_button = new Button(w-320, 10, 150, 50, "Play", [this](){ toggle_playing(); });
    property_popup = new PropertyPopup(theme);
    
    // Initialize property_spring: k(mantissa+exp), mass(mantissa+exp), massless, anchor, rededge
    std::vector<std::array<float,2>> dummy_corners = {{0,0}, {1,0}, {1,1}, {0,1}};
    Slider* spring_k_mantissa_slider = new Slider(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, 0.0f, 9.99f, 2.5f);
    Slider* spring_k_exponent_slider = new Slider(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, 0.0f, 4.0f, 1.0f);
    Slider* spring_mass_mantissa_slider = new Slider(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, 0.0f, 9.99f, 1.0f);
    Slider* spring_mass_exponent_slider = new Slider(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, 0.0f, 4.0f, 0.0f);
    spring_k_mantissa_slider->set_label("k mantissa");
    spring_k_exponent_slider->set_label("k exponent (10^x)");
    spring_mass_mantissa_slider->set_label("mass mantissa");
    spring_mass_exponent_slider->set_label("mass exponent (10^x)");
    property_spring.push_back(spring_k_mantissa_slider);
    property_spring.push_back(spring_k_exponent_slider);
    property_spring.push_back(spring_mass_mantissa_slider);
    property_spring.push_back(spring_mass_exponent_slider);

    CheckBox *spring_massless = new CheckBox(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, false);
    spring_massless->set_label("massless");
    property_spring.push_back(spring_massless);

    CheckBox *spring_anchor = new CheckBox(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, false);
    spring_anchor->set_label("anchor");
    property_spring.push_back(spring_anchor);

    ToggleBox *spring_rededge = new ToggleBox(dummy_corners,
                                              HitboxType::RECTANGLE,
                                              Orientation::NONE,
                                              {"UP", "RIGHT", "DOWN", "LEFT"},
                                              0);
    spring_rededge->set_label("rededge");
    property_spring.push_back(spring_rededge);
    
    // Initialize property_mass: 2 sliders + 1 checkbox
    Slider* mass_mantissa_slider = new Slider(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, 0.0f, 9.99f, 1.0f);
    Slider* mass_exponent_slider = new Slider(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, 0.0f, 50.0f, 0.0f);
    mass_mantissa_slider->set_label("mantissa");
    mass_exponent_slider->set_label("exponent (10^x)");
    property_mass.push_back(mass_mantissa_slider);
    property_mass.push_back(mass_exponent_slider);
    CheckBox *mass_anchor = new CheckBox(dummy_corners, HitboxType::RECTANGLE, Orientation::NONE, false);
    mass_anchor->set_label("anchor");
    property_mass.push_back(mass_anchor);

    // Initialize property_plane: rotation toggle
    ToggleBox *plane_rotation = new ToggleBox(dummy_corners,
                                              HitboxType::RECTANGLE,
                                              Orientation::NONE,
                                              {"horizontal", "vertical"},
                                              0);
    plane_rotation->set_label("rotation");
    property_plane.push_back(plane_rotation);
    
    add_object(play_button);
    add_object(new Button(w-160, 10, 150, 50, "Object Page", [this](){ child_windows.push_back(new ObjectPage(this)); }));
}

MainWindow::~MainWindow() { 
    delete property_popup; 
    for(Object *o : property_spring) delete o;
    for(Object *o : property_mass) delete o;
    for(Object *o : property_plane) delete o;
    for(Window *w : child_windows) delete w; 
    for(Object *o : objects) delete o; 
}

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
    const size_t idx = Window::add_object(object) - 1;

    switch (object->type())
    {
        case ObjectType::BUTTON:
            buttons.insert(idx);
            break;
        case ObjectType::MASS:
            masses.insert(idx);
            break;
        case ObjectType::PLANE:
            planes.insert(idx);
            break;
        case ObjectType::SPRING:
            springs.insert(idx);
            break;
        default:
            std::cout << "What have you done\n";
            break;
    }
    
    curr_object = idx;
    has_selection = false;  // Don't select when adding
    show_property_popup = false;  // Don't show popup initially when adding object
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
        objects[curr_object]->draw_selection_frame(get_renderer(), w, h);
        if(show_property_popup && property_popup) 
        {
            // Use pre-created property UI elements based on object type
            if(auto *m = dynamic_cast<Mass*>(objects[curr_object])) 
            {
                if(property_mass.size() >= 3)
                {
                    auto *mantissa_slider = dynamic_cast<Slider*>(property_mass[0]);
                    auto *exponent_slider = dynamic_cast<Slider*>(property_mass[1]);
                    if(mantissa_slider && exponent_slider)
                    {
                        float safe_mass = std::max(0.0f, m->mass);
                        int exponent = 0;
                        float mantissa = safe_mass;
                        while(mantissa >= 10.0f && exponent < 50)
                        {
                            mantissa /= 10.0f;
                            exponent++;
                        }
                        mantissa = clamp_value(mantissa, 0.0f, 9.99f);
                        mantissa_slider->set_value(mantissa);
                        exponent_slider->set_value((float)exponent);

                        mantissa_slider->set_on_change([m, exponent_slider](float v){
                            const int exp = (int)std::round(exponent_slider->get_value());
                            m->mass = clamp_value(v, 0.0f, 9.99f) * std::pow(10.0f, (float)exp);
                        });
                        exponent_slider->set_on_change([m, mantissa_slider, exponent_slider](float v){
                            const int exp = (int)std::round(v);
                            exponent_slider->set_value((float)exp);
                            m->mass = clamp_value(mantissa_slider->get_value(), 0.0f, 9.99f) * std::pow(10.0f, (float)exp);
                        });
                    }
                    if(auto *checkbox = dynamic_cast<CheckBox*>(property_mass[2]))
                    {
                        checkbox->set_checked(objects[curr_object]->anchor);
                        checkbox->set_on_toggle([m](bool checked){ m->anchor = checked; });
                    }
                }
                property_popup->load(objects[curr_object], property_mass, w, h);
                property_popup->draw(get_renderer());
            }
            else if(auto *s = dynamic_cast<Spring*>(objects[curr_object])) 
            {
                if(property_spring.size() >= 7)
                {
                    Slider *k_mantissa_slider = dynamic_cast<Slider*>(property_spring[0]);
                    Slider *k_exponent_slider = dynamic_cast<Slider*>(property_spring[1]);
                    Slider *mass_mantissa_slider = dynamic_cast<Slider*>(property_spring[2]);
                    Slider *mass_exponent_slider = dynamic_cast<Slider*>(property_spring[3]);
                    CheckBox *massless_checkbox = dynamic_cast<CheckBox*>(property_spring[4]);
                    CheckBox *anchor_checkbox = dynamic_cast<CheckBox*>(property_spring[5]);
                    ToggleBox *rededge_toggle = dynamic_cast<ToggleBox*>(property_spring[6]);

                    if(k_mantissa_slider && k_exponent_slider)
                    {
                        float safe_k = std::max(0.0f, s->k_const);
                        int k_exp = 0;
                        float k_man = safe_k;
                        while(k_man >= 10.0f && k_exp < 4)
                        {
                            k_man /= 10.0f;
                            k_exp++;
                        }
                        k_man = clamp_value(k_man, 0.0f, 9.99f);
                        k_mantissa_slider->set_value(k_man);
                        k_exponent_slider->set_value((float)k_exp);

                        k_mantissa_slider->set_on_change([s, k_exponent_slider](float v){
                            const int exp = (int)std::round(k_exponent_slider->get_value());
                            s->k_const = clamp_value(v, 0.0f, 9.99f) * std::pow(10.0f, (float)exp);
                        });
                        k_exponent_slider->set_on_change([s, k_mantissa_slider, k_exponent_slider](float v){
                            const int exp = (int)std::round(v);
                            k_exponent_slider->set_value((float)exp);
                            s->k_const = clamp_value(k_mantissa_slider->get_value(), 0.0f, 9.99f) * std::pow(10.0f, (float)exp);
                        });
                    }

                    if(mass_mantissa_slider && mass_exponent_slider)
                    {
                        float safe_mass = std::max(0.0f, s->mass);
                        int mass_exp = 0;
                        float mass_man = safe_mass;
                        while(mass_man >= 10.0f && mass_exp < 4)
                        {
                            mass_man /= 10.0f;
                            mass_exp++;
                        }
                        mass_man = clamp_value(mass_man, 0.0f, 9.99f);
                        mass_mantissa_slider->set_value(mass_man);
                        mass_exponent_slider->set_value((float)mass_exp);

                        mass_mantissa_slider->set_on_change([s, mass_exponent_slider, mass_mantissa_slider](float v){
                            if(s->massless)
                            {
                                s->mass = 0.0f;
                                mass_mantissa_slider->set_value(0.0f);
                                return;
                            }
                            const int exp = (int)std::round(mass_exponent_slider->get_value());
                            s->mass = clamp_value(v, 0.0f, 9.99f) * std::pow(10.0f, (float)exp);
                        });
                        mass_exponent_slider->set_on_change([s, mass_mantissa_slider, mass_exponent_slider](float v){
                            if(s->massless)
                            {
                                s->mass = 0.0f;
                                mass_mantissa_slider->set_value(0.0f);
                                return;
                            }
                            const int exp = (int)std::round(v);
                            mass_exponent_slider->set_value((float)exp);
                            s->mass = clamp_value(mass_mantissa_slider->get_value(), 0.0f, 9.99f) * std::pow(10.0f, (float)exp);
                        });
                    }

                    if(massless_checkbox)
                    {
                        massless_checkbox->set_checked(s->massless);
                        massless_checkbox->set_on_toggle([s, mass_mantissa_slider](bool checked){
                            s->massless = checked;
                            if(checked)
                            {
                                s->mass = 0.0f;
                                if(mass_mantissa_slider) mass_mantissa_slider->set_value(0.0f);
                            }
                        });
                    }

                    if(anchor_checkbox)
                    {
                        anchor_checkbox->set_checked(s->anchor);
                        anchor_checkbox->set_on_toggle([s](bool checked){ s->anchor = checked; });
                    }

                    if(rededge_toggle)
                    {
                        size_t edge_index = 0;
                        switch(s->orientation)
                        {
                            case Orientation::UP: edge_index = 0; break;
                            case Orientation::RIGHT: edge_index = 1; break;
                            case Orientation::DOWN: edge_index = 2; break;
                            case Orientation::LEFT: edge_index = 3; break;
                            default: edge_index = 0; break;
                        }
                        rededge_toggle->set_index(edge_index);
                        rededge_toggle->set_on_change([s](size_t idx, const std::string &value){
                            (void)value;
                            switch(idx)
                            {
                                case 0: s->orientation = Orientation::UP; break;
                                case 1: s->orientation = Orientation::RIGHT; break;
                                case 2: s->orientation = Orientation::DOWN; break;
                                case 3: s->orientation = Orientation::LEFT; break;
                                default: break;
                            }
                        });
                    }
                }
                property_popup->load(objects[curr_object], property_spring, w, h);
                property_popup->draw(get_renderer());
            }
            else if(auto *p = dynamic_cast<Plane*>(objects[curr_object]))
            {
                if(property_plane.size() >= 1)
                {
                    if(auto *rotation_toggle = dynamic_cast<ToggleBox*>(property_plane[0]))
                    {
                        rotation_toggle->set_index(p->vertical ? 1 : 0);
                        rotation_toggle->set_on_change([p](size_t idx, const std::string &value){
                            (void)value;
                            const bool make_vertical = (idx == 1);
                            if(p->vertical == make_vertical)
                                return;

                            float left, top, right, bottom;
                            p->get_rect_bounds(left, top, right, bottom);

                            const float cx = (left + right) * 0.5f;
                            const float cy = (top + bottom) * 0.5f;
                            const float curr_w = right - left;
                            const float curr_h = bottom - top;
                            const float length = std::max(curr_w, curr_h);
                            const float thickness = std::max(1e-4f, std::min(curr_w, curr_h));

                            const float new_w = make_vertical ? thickness : length;
                            const float new_h = make_vertical ? length : thickness;

                            p->set_rect_from_bounds(
                                cx - new_w * 0.5f,
                                cy - new_h * 0.5f,
                                cx + new_w * 0.5f,
                                cy + new_h * 0.5f
                            );

                            p->vertical = make_vertical;
                        });
                    }
                }
                property_popup->load(objects[curr_object], property_plane, w, h);
                property_popup->draw(get_renderer());
            }
        }
    }
    for(Window *cw : child_windows) { cw->clear_window(&cw->theme->background); cw->main_loop(); cw->render(); }
}

void MainWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    for(auto it = child_windows.begin(); it != child_windows.end(); ) 
    {
        (*it)->event_handler(event);
        if(!(*it)->running) { delete *it; it = child_windows.erase(it); } else ++it;
    }

    if(show_property_popup && property_popup && property_popup->handle_event(event))
        return;

    int w, h; get_size(w, h);

    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_BACKSPACE && !playing && has_selection && curr_object < objects.size() && !buttons.count(curr_object))
        {
            const size_t deleted_idx = curr_object;

            delete objects[deleted_idx];
            objects.erase(objects.begin() + (long)deleted_idx);

            auto reindex_set_after_delete = [deleted_idx](std::unordered_set<size_t> &indices)
            {
                std::unordered_set<size_t> remapped;
                remapped.reserve(indices.size());
                for(size_t idx : indices)
                {
                    if(idx == deleted_idx)
                        continue;
                    remapped.insert(idx > deleted_idx ? idx - 1 : idx);
                }
                indices.swap(remapped);
            };

            reindex_set_after_delete(masses);
            reindex_set_after_delete(buttons);
            reindex_set_after_delete(planes);
            reindex_set_after_delete(springs);

            has_selection = false;
            show_property_popup = false;
            dragging = false;
            resizing = false;

            if(objects.empty()) curr_object = 0;
            else if(deleted_idx >= objects.size()) curr_object = objects.size() - 1;
            else curr_object = deleted_idx;

            return;
        }
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
    {
        bool hit_any = false;
        if(!playing && has_selection && show_property_popup && curr_object < objects.size()) 
        {
            if(property_popup && property_popup->contains(event.button.x, event.button.y))
                return;
        }
        for(size_t i=0; i<objects.size(); i++) 
        {
            if(objects[i]->is_mouse_click(event.button.x, event.button.y, w, h)) 
            {
                if(buttons.count(i)) { dynamic_cast<Button*>(objects[i])->press(); hit_any = true; break; }
                hit_any = true; curr_object = i; has_selection = true;
                
                // CRITICAL FIX: Only show popup if it's a double-click (clicks >= 2)
                show_property_popup = (event.button.clicks >= 2);

                if(show_property_popup)
                {
                    // Opening the popup cancels any object transformation.
                    dragging = false;
                    resizing = false;
                }
                else
                {
                    x_start = event.button.x; y_start = event.button.y;
                    float l, t, r, b; objects[i]->get_rect_bounds(l, t, r, b);
                    drag_anchor_x = event.button.x - (int)(l*w); drag_anchor_y = event.button.y - (int)(t*h);
                    size_t hh; resizing = objects[i]->try_get_resize_handle(event.button.x, event.button.y, w, h, hh);
                    if(resizing) resize_handle = hh;
                    dragging = !resizing;
                }
                break;
            }
        }
        if(!hit_any) { has_selection = show_property_popup = false; }
    }
    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP) dragging = resizing = false;
    if(event.type == SDL_EVENT_MOUSE_MOTION && !show_property_popup && (dragging || resizing)) 
    {
        if(resizing) objects[curr_object]->resize_rect_object_handle(resize_handle, event.motion.x-(int)x_start, event.motion.y-(int)y_start, w, h);
        else objects[curr_object]->move_object_by_pixels(event.motion.x-(int)x_start, event.motion.y-(int)y_start, w, h);
        x_start = event.motion.x; y_start = event.motion.y;
    }
}