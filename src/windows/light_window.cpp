#include "windows/light_window.hpp"
#include "windows/object_page.hpp"
#include "objects/hamburger_button.hpp"
#include "objects/slider.hpp"
#include "objects/checkbox.hpp"
#include "objects/togglebox.hpp"
#include "engine.hpp"

#include <algorithm>
#include <cmath>

// =============================================================================
// Constructor / Destructor
// =============================================================================
LightWindow::LightWindow(Theme *theme)
    : Window("Light Simulation", 1920, 1080, theme)
{
    int w, h;
    get_size(w, h);

    // ── Hamburger / object-list button ────────────────────────────────────
    menu_button = new HamburgerButton(10, 10, 50, 50,
        [this]() { child_windows.push_back(new ObjectPage(this)); });
    normalize_button(menu_button);

    // ── Play / Pause button ───────────────────────────────────────────────
    // Positioned to the right of the menu button
    play_button = new Button(70, 10, 90, 50, "Play",
        [this]() { toggle_playing(); });
    normalize_button(play_button);

    property_popup = new PropertyPopup(theme);

    // Add toolbar buttons directly to objects (already normalized above)
    // so Window::add_object doesn't normalize them a second time.
    objects.push_back(menu_button);
    ui_button_indices.insert(objects.size() - 1);
    objects.push_back(play_button);
    ui_button_indices.insert(objects.size() - 1);

    // Pre-build popup widget lists
    rebuild_property_mirror();
    rebuild_property_wall();
    rebuild_property_source();
    rebuild_property_laser();
    rebuild_property_ray();
}

LightWindow::~LightWindow()
{
    delete property_popup;
    for(Object *o : property_mirror) delete o;
    for(Object *o : property_wall)   delete o;
    for(Object *o : property_source) delete o;
    for(Object *o : property_laser)  delete o;
    for(Object *o : property_ray)    delete o;
    for(Window *cw : child_windows)  delete cw;
    for(Object *o  : objects)        delete o;
}

// =============================================================================
// Public helpers
// =============================================================================
void LightWindow::toggle_playing()
{
    playing = !playing;
    play_button->set_label(playing ? "Pause" : "Play");
    if(playing)
        dragging = resizing = has_selection = show_property_popup = false;
}

void LightWindow::add_object(Object *object)
{
    if(!object) return;

    auto *po = dynamic_cast<PhysicsObject*>(object);
    if(!po) return;

    const size_t idx = Window::add_object(object) - 1;
    physics_objects.push_back(po);

    switch(object->type())
    {
        case ObjectType::WALL:         walls.insert(idx);         break;
        case ObjectType::MIRROR:       mirrors.insert(idx);       break;
        case ObjectType::LIGHT_SOURCE: light_sources.insert(idx); break;
        case ObjectType::LASER:        lasers.insert(idx);        break;
        case ObjectType::LIGHT_RAY:    light_rays.insert(idx);    break;
        default: break;
    }
}

// =============================================================================
// Main loop
// =============================================================================
void LightWindow::main_loop()
{
    int w, h;
    get_size(w, h);

    // Draw all scene objects
    for(Object *o : objects)
        if(o) o->draw_object(get_renderer(), theme, w, h);

    // Selection frame + popup (edit mode only)
    if(!playing && has_selection && curr_object < objects.size()
       && !ui_button_indices.count(curr_object))
    {
        objects[curr_object]->draw_selection_frame(get_renderer(), w, h);

        if(show_property_popup && property_popup)
        {
            load_property_popup();
            property_popup->draw(get_renderer());
        }
    }

    // Child windows
    for(Window *cw : child_windows)
    {
        cw->clear_window(&cw->theme->background);
        cw->main_loop();
        cw->render();
    }
}

// =============================================================================
// Event handler
// =============================================================================
void LightWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running) return;

    // Forward to child windows, prune closed ones
    for(auto it = child_windows.begin(); it != child_windows.end(); )
    {
        (*it)->event_handler(event);
        if(!(*it)->running) { delete *it; it = child_windows.erase(it); }
        else ++it;
    }

    // Property popup swallows events when open
    if(show_property_popup && property_popup && property_popup->handle_event(event))
        return;

    // Only handle mouse events that belong to this window
    if((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.type == SDL_EVENT_MOUSE_BUTTON_UP   ||
        event.type == SDL_EVENT_MOUSE_MOTION)
       && event.window.windowID != SDL_GetWindowID(get_window()))
        return;

    int w, h;
    get_size(w, h);

    // ── Backspace — delete selected object ────────────────────────────────
    if(event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_BACKSPACE && !playing && has_selection
           && curr_object < objects.size() && !ui_button_indices.count(curr_object))
        {
            const size_t del = curr_object;
            auto *po = dynamic_cast<PhysicsObject*>(objects[del]);
            delete objects[del];
            objects.erase(objects.begin() + (long)del);
            if(po)
                physics_objects.erase(
                    std::remove(physics_objects.begin(), physics_objects.end(), po),
                    physics_objects.end());

            auto reindex = [del](std::unordered_set<size_t> &s)
            {
                std::unordered_set<size_t> r;
                for(size_t i : s) { if(i == del) continue; r.insert(i > del ? i-1 : i); }
                s.swap(r);
            };
            reindex(walls); reindex(mirrors); reindex(light_sources);
            reindex(lasers); reindex(light_rays); reindex(ui_button_indices);

            has_selection = show_property_popup = dragging = resizing = false;
            curr_object = objects.empty() ? 0 : std::min(del, objects.size()-1);
            return;
        }
    }

    // ── Mouse button down ─────────────────────────────────────────────────
    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        // Popup open — click outside closes it
        if(show_property_popup && property_popup
           && !property_popup->contains(event.button.x, event.button.y))
        {
            show_property_popup = false;
            return;
        }

        bool hit_any = false;

        // Toolbar buttons first
        for(size_t i : ui_button_indices)
        {
            if(i < objects.size() && objects[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                dynamic_cast<Button*>(objects[i])->press();
                hit_any = true;
                break;
            }
        }

        if(!hit_any)
        {
            // Iterate in reverse so topmost (last-added) object wins
            for(int i = (int)objects.size()-1; i >= 0; --i)
            {
                if(ui_button_indices.count((size_t)i)) continue;
                if(!objects[i]->is_mouse_click(event.button.x, event.button.y, w, h)) continue;

                hit_any      = true;
                curr_object  = (size_t)i;
                has_selection = true;

                if(event.button.clicks >= 2)
                {
                    // Double-click → open property popup
                    show_property_popup = true;
                    dragging = resizing = false;
                }
                else
                {
                    show_property_popup = false;
                    x_start = event.button.x;
                    y_start = event.button.y;

                    size_t hh;
                    resizing = objects[i]->try_get_resize_handle(
                        event.button.x, event.button.y, w, h, hh);
                    if(resizing) resize_handle = hh;
                    dragging = !resizing;
                }
                break;
            }
        }

        if(!hit_any)
            has_selection = show_property_popup = false;
    }

    // ── Mouse button up ───────────────────────────────────────────────────
    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        dragging = resizing = false;

    // ── Mouse motion — drag or resize ─────────────────────────────────────
    if(event.type == SDL_EVENT_MOUSE_MOTION && !show_property_popup
       && (dragging || resizing) && curr_object < objects.size())
    {
        const int dx = event.motion.x - (int)x_start;
        const int dy = event.motion.y - (int)y_start;

        if(resizing)
            objects[curr_object]->resize_rect_object_handle(resize_handle, dx, dy, w, h);
        else
            objects[curr_object]->move_object_by_pixels(dx, dy, w, h);

        x_start = event.motion.x;
        y_start = event.motion.y;
    }
}

// =============================================================================
// Protected — double-click hook from Window base
// =============================================================================
void LightWindow::on_physics_object_double_click(PhysicsObject * /*object*/,
                                                  size_t index, SDL_Event & /*event*/)
{
    if(index < objects.size() && !ui_button_indices.count(index))
    {
        curr_object        = index;
        has_selection      = true;
        show_property_popup = true;
        dragging = resizing = false;
    }
}

// =============================================================================
// Collision step
// =============================================================================
void LightWindow::handle_collisions()
{
    for(size_t i = 0; i < physics_objects.size(); ++i)
        for(size_t j = i+1; j < physics_objects.size(); ++j)
            Engine::is_collision(physics_objects[i], physics_objects[j]);
}

// =============================================================================
// Property popup — load correct widgets for selected object
// =============================================================================
void LightWindow::load_property_popup()
{
    if(!property_popup || curr_object >= objects.size()) return;
    int w, h; get_size(w, h);
    Object *obj = objects[curr_object];

    if(mirrors.count(curr_object))
    {
        if(auto *m = dynamic_cast<Mirror*>(obj))
        {
            if(property_mirror.size() >= 2)
            {
                // Sync toggle to current mirror type
                auto *tb = dynamic_cast<ToggleBox*>(property_mirror[0]);
                if(tb)
                {
                    tb->set_index(m->mirror_type == FLAT ? 0 :
                                  m->mirror_type == CONCAVE ? 1 : 2);
                    tb->set_on_change([m](size_t idx, const std::string &)
                    {
                        m->mirror_type = idx == 0 ? FLAT :
                                         idx == 1 ? CONCAVE : CONVEX;
                    });
                }

                // Sync concavity slider
                auto *sl = dynamic_cast<Slider*>(property_mirror[1]);
                if(sl)
                {
                    sl->set_value(m->concavity);
                    sl->set_on_change([m](float v){ m->concavity = v; });
                }
            }
        }
        property_popup->load(obj, property_mirror, w, h);
    }
    else if(walls.count(curr_object))
    {
        property_popup->load(obj, property_wall, w, h);
    }
    else if(light_sources.count(curr_object))
    {
        if(auto *src = dynamic_cast<LightSource*>(obj))
        {
            if(property_source.size() >= 2)
            {
                // Sync emission toggle
                auto *tb = dynamic_cast<ToggleBox*>(property_source[0]);
                if(tb)
                {
                    tb->set_index(src->radial ? 1 : 0);
                    tb->set_on_change([src](size_t idx, const std::string &)
                    {
                        src->radial = (idx == 1);
                    });
                }

                // Sync strength slider (0 = infinite)
                auto *sl = dynamic_cast<Slider*>(property_source[1]);
                if(sl)
                {
                    sl->set_value((float)src->strength);
                    sl->set_on_change([src](float v)
                    {
                        src->strength = (int)v;  // 0 means infinite
                    });
                }
            }
        }
        property_popup->load(obj, property_source, w, h);
    }
    else if(lasers.count(curr_object))
    {
        property_popup->load(obj, property_laser, w, h);
    }
    else if(light_rays.count(curr_object))
    {
        property_popup->load(obj, property_ray, w, h);
    }
}

// =============================================================================
// Property popup widget builders (called once in constructor)
// =============================================================================
void LightWindow::rebuild_property_mirror()
{
    for(Object *o : property_mirror) delete o;
    property_mirror.clear();

    std::vector<std::vector<float>> dc = {{0,0},{1,0},{1,1},{0,1}};

    auto *mirror_type_toggle = new ToggleBox(dc, HitboxType::RECTANGLE, Orientation::NONE,
                                              {"Flat", "Concave", "Convex"}, 0);
    mirror_type_toggle->set_label("type");
    // Callback is set fresh each time load_property_popup() is called
    property_mirror.push_back(mirror_type_toggle);

    auto *concavity_slider = new Slider(dc, HitboxType::RECTANGLE, Orientation::NONE,
                                         0.0f, 1.0f, 0.5f);
    concavity_slider->set_label("concavity");
    property_mirror.push_back(concavity_slider);
}

void LightWindow::rebuild_property_wall()
{
    for(Object *o : property_wall) delete o;
    property_wall.clear();

    std::vector<std::vector<float>> dc = {{0,0},{1,0},{1,1},{0,1}};

    auto *orientation = new ToggleBox(dc, HitboxType::RECTANGLE, Orientation::NONE,
                                       {"Horizontal", "Vertical"}, 0);
    orientation->set_label("orientation");
    property_wall.push_back(orientation);
}

void LightWindow::rebuild_property_source()
{
    for(Object *o : property_source) delete o;
    property_source.clear();

    std::vector<std::vector<float>> dc = {{0,0},{1,0},{1,1},{0,1}};

    // Emission mode toggle — callback wired in load_property_popup
    auto *emission = new ToggleBox(dc, HitboxType::RECTANGLE, Orientation::NONE,
                                    {"Linear", "Radial"}, 0);
    emission->set_label("emission");
    property_source.push_back(emission);

    // Strength — number of rays; 0 = infinite (slider goes 0–256, label shows "∞" at 0)
    auto *strength = new Slider(dc, HitboxType::RECTANGLE, Orientation::NONE,
                                 0.0f, 256.0f, 8.0f);
    strength->set_label("strength (rays)");
    property_source.push_back(strength);
}

void LightWindow::rebuild_property_laser()
{
    for(Object *o : property_laser) delete o;
    property_laser.clear();

    std::vector<std::vector<float>> dc = {{0,0},{1,0},{1,1},{0,1}};

    auto *intensity = new Slider(dc, HitboxType::RECTANGLE, Orientation::NONE,
                                  0.0f, 1.0f, 1.0f);
    intensity->set_label("intensity");
    property_laser.push_back(intensity);
}

void LightWindow::rebuild_property_ray()
{
    for(Object *o : property_ray) delete o;
    property_ray.clear();

    // Light ray has no configurable properties yet — popup will show empty
}

// =============================================================================
// Normalize button coords from px → [0,1]
// =============================================================================
void LightWindow::normalize_button(Button *btn)
{
    int w, h;
    get_size(w, h);
    for(size_t i = 0; i < btn->corners.size(); i++)
    {
        btn->corners[i][0]    /= (float)w;
        btn->corners[i][1]    /= (float)h;
        btn->base_shape[i][0] /= (float)w;
        btn->base_shape[i][1] /= (float)h;
    }
    btn->create_hitbox();
}