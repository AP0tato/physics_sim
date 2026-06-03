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
    // (now handled automatically by PropertyPopup::load_for_object)
}

LightWindow::~LightWindow()
{
    delete property_popup;
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

    // Ray simulation (always — edit mode shows preview too)
    simulate_rays(get_renderer(), w, h);

    // Selection frame + popup (edit mode only)
    if(!playing && has_selection && curr_object < objects.size()
       && !ui_button_indices.count(curr_object))
    {
        objects[curr_object]->draw_selection_frame(get_renderer(), w, h);

        if(show_property_popup && property_popup)
        {
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
        // Popup open — click outside closes it, but still process the click
        if(show_property_popup && property_popup
           && !property_popup->contains(event.button.x, event.button.y))
        {
            show_property_popup = false;
            // Fall through so the click can select/interact with objects
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
                    load_property_popup();
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
        curr_object         = index;
        has_selection       = true;
        show_property_popup = true;
        dragging = resizing = false;
        load_property_popup();
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
    property_popup->load_for_object(objects[curr_object], w, h);
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
// =============================================================================
// Ray simulation — emits and traces rays from LightSource and Laser objects
// =============================================================================

namespace
{
    // -------------------------------------------------------------------------
    // Parametric segment intersection.
    // Ray: P = origin + t * dir  (t >= 0)
    // Segment: Q = seg_a + u * (seg_b - seg_a)  (u in [0,1])
    // Returns true and writes t if a forward intersection is found.
    // -------------------------------------------------------------------------
    static bool ray_vs_segment(float ox, float oy, float dx, float dy,
                                float ax, float ay, float bx, float by,
                                float &t_out)
    {
        const float sx = bx - ax;
        const float sy = by - ay;
        const float denom = dx * sy - dy * sx;
        if(std::fabs(denom) < 1e-9f)
            return false;

        const float qx = ax - ox;
        const float qy = ay - oy;

        const float t = (qx * sy - qy * sx) / denom;
        const float u = (qx * dy - qy * dx) / denom;

        if(t > 1e-4f && u >= -1e-5f && u <= 1.0f + 1e-5f)
        {
            t_out = t;
            return true;
        }
        return false;
    }

    // -------------------------------------------------------------------------
    // Reflect direction (dx,dy) about surface normal (nx,ny).
    // -------------------------------------------------------------------------
    static void reflect(float dx, float dy, float nx, float ny,
                        float &rx, float &ry)
    {
        const float dot = dx * nx + dy * ny;
        rx = dx - 2.0f * dot * nx;
        ry = dy - 2.0f * dot * ny;
    }

    // -------------------------------------------------------------------------
    // Collect all edges from scene objects that can block or reflect rays.
    // edge_type: 0 = wall (absorb), 1 = mirror flat, 2 = mirror concave, 3 = mirror convex
    // -------------------------------------------------------------------------
    struct SceneEdge
    {
        float ax, ay, bx, by;   // endpoints (normalized 0..1)
        int   edge_type;         // 0=wall 1=flat-mirror 2=concave 3=convex
        float concavity;         // used only for curved mirrors
    };

    static void collect_edges(const std::vector<Object*>       &objects,
                               const std::unordered_set<size_t> &walls,
                               const std::unordered_set<size_t> &mirrors,
                               std::vector<SceneEdge>            &edges)
    {
        edges.clear();
        for(size_t idx : walls)
        {
            if(idx >= objects.size()) continue;
            auto *wall = dynamic_cast<Wall*>(objects[idx]);
            if(!wall) continue;

            float left, top, right, bottom;
            wall->get_rect_bounds(left, top, right, bottom);

            // Match exactly the line that draw_object renders
            if(wall->is_vertical())
            {
                const float mx = (left + right) * 0.5f;
                edges.push_back({mx, top, mx, bottom, 0, 0.0f});
            }
            else
            {
                const float my = (top + bottom) * 0.5f;
                edges.push_back({left, my, right, my, 0, 0.0f});
            }
        }
        for(size_t idx : mirrors)
        {
            if(idx >= objects.size()) continue;
            auto *m = dynamic_cast<Mirror*>(objects[idx]);
            if(!m) continue;
            const int etype = (m->mirror_type == FLAT)    ? 1 :
                              (m->mirror_type == CONCAVE) ? 2 : 3;
            const auto &c = objects[idx]->corners;
            for(size_t i = 0; i < c.size(); ++i)
            {
                const auto &p0 = c[i];
                const auto &p1 = c[(i+1) % c.size()];
                edges.push_back({p0[0], p0[1], p1[0], p1[1], etype, m->concavity});
            }
        }
    }

    // -------------------------------------------------------------------------
    // Trace a single ray from (ox,oy) in direction (dx,dy) (pre-normalised).
    // Draws ray segments and bounces off mirrors; stops at walls or max depth.
    // -------------------------------------------------------------------------
    static void trace_ray(SDL_Renderer *renderer,
                          float ox, float oy, float dx, float dy,
                          const std::vector<SceneEdge> &edges,
                          SDL_Color color, int w, int h,
                          int depth = 0)
    {
        constexpr int   MAX_DEPTH_RAY = 8;
        constexpr float MAX_T     = 2.0f;   // diagonal of unit square ~ 1.41

        if(depth >= MAX_DEPTH_RAY) return;

        float best_t    = MAX_T;
        int   best_edge = -1;

        for(int i = 0; i < (int)edges.size(); ++i)
        {
            float t;
            if(ray_vs_segment(ox, oy, dx, dy,
                               edges[i].ax, edges[i].ay,
                               edges[i].bx, edges[i].by, t))
            {
                if(t < best_t)
                {
                    best_t    = t;
                    best_edge = i;
                }
            }
        }

        // Hit point (or far end)
        const float hx = ox + dx * best_t;
        const float hy = oy + dy * best_t;

        // Draw segment
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderLine(renderer,
                       (int)(ox * w), (int)(oy * h),
                       (int)(hx * w), (int)(hy * h));

        if(best_edge < 0) return;  // no hit — ray left scene

        const SceneEdge &e = edges[best_edge];
        if(e.edge_type == 0) return;  // wall — absorb

        // Compute outward normal of the hit edge
        float ex = e.bx - e.ax;
        float ey = e.by - e.ay;
        const float elen = std::sqrt(ex*ex + ey*ey);
        if(elen < 1e-9f) return;
        ex /= elen;  ey /= elen;
        float nx = -ey, ny = ex;  // rotate 90° CCW

        // Make sure normal faces the incoming ray
        if(dx * nx + dy * ny > 0.0f) { nx = -nx; ny = -ny; }

        if(e.edge_type == 1)
        {
            // Flat mirror — simple specular reflection
            float rx, ry;
            reflect(dx, dy, nx, ny, rx, ry);
            const float rlen = std::sqrt(rx*rx + ry*ry);
            if(rlen < 1e-9f) return;
            trace_ray(renderer, hx, hy, rx/rlen, ry/rlen, edges, color, w, h, depth+1);
        }
        else
        {
            // Curved mirror — perturb the normal based on concavity.
            // We approximate curvature by rotating the normal toward the
            // edge centre using a fraction of the concavity parameter.
            const float midx = (e.ax + e.bx) * 0.5f;
            const float midy = (e.ay + e.by) * 0.5f;
            float cnx = midx - hx;
            float cny = midy - hy;
            const float cnlen = std::sqrt(cnx*cnx + cny*cny);
            if(cnlen > 1e-9f)
            {
                cnx /= cnlen;  cny /= cnlen;
                const float k = e.concavity * ((e.edge_type == 2) ? 1.0f : -1.0f);
                float pnx = nx + k * cnx;
                float pny = ny + k * cny;
                const float pnlen = std::sqrt(pnx*pnx + pny*pny);
                if(pnlen > 1e-9f) { pnx /= pnlen; pny /= pnlen; nx = pnx; ny = pny; }
            }
            float rx, ry;
            reflect(dx, dy, nx, ny, rx, ry);
            const float rlen = std::sqrt(rx*rx + ry*ry);
            if(rlen < 1e-9f) return;
            trace_ray(renderer, hx, hy, rx/rlen, ry/rlen, edges, color, w, h, depth+1);
        }
    }
}

void LightWindow::simulate_rays(SDL_Renderer *renderer, int w, int h)
{
    std::vector<SceneEdge> edges;
    collect_edges(objects, walls, mirrors, edges);
    // Note: edges may be empty — rays still emit and travel to screen boundary

    constexpr float PI = 3.14159265358979f;
    constexpr float DEG2RAD = PI / 180.0f;

    // ── LightSource emitters ─────────────────────────────────────────────
    for(size_t idx : light_sources)
    {
        if(idx >= objects.size()) continue;
        auto *src = dynamic_cast<LightSource*>(objects[idx]);
        if(!src) continue;

        // Centre of the source hitbox
        float left, top, right, bottom;
        src->get_rect_bounds(left, top, right, bottom);
        const float cx = (left + right)  * 0.5f;
        const float cy = (top  + bottom) * 0.5f;

        const SDL_Color yellow = {255, 220, 80, 220};

        const float center_rad = src->angle_deg * DEG2RAD;

        if(src->radial)
        {
            // Radial — rays spread outward from centre in an arc around facing angle.
            // emission_angle=360 gives a full omnidirectional point light.
            const int   num_rays = (src->strength > 0) ? src->strength : 64;
            const float arc      = src->emission_angle * DEG2RAD;
            const float start    = center_rad - arc * 0.5f;
            for(int i = 0; i < num_rays; ++i)
            {
                const float angle = start + arc * ((float)i / (float)(num_rays > 1 ? num_rays : 1));
                trace_ray(renderer, cx, cy, std::cos(angle), std::sin(angle),
                          edges, yellow, w, h);
            }
        }
        else
        {
            // Linear — parallel rays from the front face of the emitter.
            // Pick the rectangle edge most aligned with the emission direction;
            // spread origins along it. All rays share the same direction.
            const int   num_rays    = (src->strength > 0) ? src->strength : 16;
            const float ddx         = std::cos(center_rad);
            const float ddy         = std::sin(center_rad);
            const float spread_frac = src->emission_angle / 360.0f;

            float ox_a, oy_a, ox_b, oy_b;
            if(std::abs(ddx) >= std::abs(ddy))
            {
                // Horizontal dominant — left or right edge
                const float ex   = (ddx >= 0.0f) ? right : left;
                const float midy = (top + bottom) * 0.5f;
                const float half = (bottom - top) * 0.5f * spread_frac;
                ox_a = ex;  oy_a = midy - half;
                ox_b = ex;  oy_b = midy + half;
            }
            else
            {
                // Vertical dominant — top or bottom edge
                const float ey   = (ddy >= 0.0f) ? bottom : top;
                const float midx = (left + right) * 0.5f;
                const float half = (right - left) * 0.5f * spread_frac;
                ox_a = midx - half;  oy_a = ey;
                ox_b = midx + half;  oy_b = ey;
            }

            for(int i = 0; i < num_rays; ++i)
            {
                const float t = (num_rays > 1) ? (float)i / (float)(num_rays - 1) : 0.5f;
                trace_ray(renderer,
                          ox_a + t * (ox_b - ox_a),
                          oy_a + t * (oy_b - oy_a),
                          ddx, ddy,
                          edges, yellow, w, h);
            }
        }
    }

    // ── Laser emitters ───────────────────────────────────────────────────
    for(size_t idx : lasers)
    {
        if(idx >= objects.size()) continue;
        auto *laser = dynamic_cast<Laser*>(objects[idx]);
        if(!laser) continue;

        float left, top, right, bottom;
        laser->get_rect_bounds(left, top, right, bottom);
        const float cx = (left + right) * 0.5f;
        const float cy = (top  + bottom) * 0.5f;

        // Laser fires in direction set by angle_deg
        const float la = laser->angle_deg * (3.14159265f / 180.0f);
        trace_ray(renderer, cx, cy, std::cos(la), std::sin(la), edges, laser->ray_color, w, h);
    }

    // ── Pre-placed LightRay objects (static directed rays) ───────────────
    for(size_t idx : light_rays)
    {
        if(idx >= objects.size()) continue;
        auto *ray = dynamic_cast<LightRay*>(objects[idx]);
        if(!ray) continue;

        float left, top, right, bottom;
        ray->get_rect_bounds(left, top, right, bottom);
        const float cx = (left  + right) * 0.5f;
        const float cy = (top   + bottom) * 0.5f;
        const float ra = ray->angle_deg * (3.14159265f / 180.0f);
        const SDL_Color white = {255, 255, 255, 200};
        trace_ray(renderer, cx, cy, std::cos(ra), std::sin(ra), edges, white, w, h);
    }
}