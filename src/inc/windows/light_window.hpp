#ifndef LIGHT_WINDOW_HPP
#define LIGHT_WINDOW_HPP

#include "windows/window.hpp"
#include "objects/object.hpp"
#include "objects/physicsobject.hpp"
#include "objects/button.hpp"
#include "objects/mirror.hpp"
#include "objects/light.hpp"
#include "objects/laser.hpp"
#include "objects/lightray.hpp"
#include "objects/wall.hpp"
#include "windows/property_popup.hpp"

#include <unordered_set>
#include <vector>

class ObjectPage;

class LightWindow : public Window
{
public:
    explicit LightWindow(Theme *theme);
    ~LightWindow();

    void add_object(Object *object);
    void toggle_playing();
    bool is_playing() const { return playing; }

    void main_loop()                  override;
    void event_handler(SDL_Event &ev) override;

protected:
    void on_physics_object_double_click(PhysicsObject *object, size_t index, SDL_Event &event) override;

private:
    // ── Toolbar buttons ───────────────────────────────────────────────────
    Button               *play_button    = nullptr;
    Button               *menu_button    = nullptr;

    // ── Child windows ─────────────────────────────────────────────────────
    std::vector<Window*>  child_windows;

    // ── Scene objects ─────────────────────────────────────────────────────
    std::vector<PhysicsObject*> physics_objects;

    // Index sets by type — mirrors MainWindow pattern
    std::unordered_set<size_t> walls;
    std::unordered_set<size_t> mirrors;
    std::unordered_set<size_t> light_sources;
    std::unordered_set<size_t> lasers;
    std::unordered_set<size_t> light_rays;
    std::unordered_set<size_t> ui_button_indices; // toolbar buttons, never selectable

    // ── Property popup ────────────────────────────────────────────────────
    PropertyPopup        *property_popup = nullptr;

    // Pre-built popup widget lists (one per object type)
    std::vector<Object*>  property_mirror;
    std::vector<Object*>  property_wall;
    std::vector<Object*>  property_source;   // LightSource
    std::vector<Object*>  property_laser;
    std::vector<Object*>  property_ray;

    // ── Interaction state ─────────────────────────────────────────────────
    bool   playing           = false;
    bool   dragging          = false;
    bool   resizing          = false;
    bool   has_selection     = false;
    bool   show_property_popup = false;
    size_t curr_object       = 0;
    size_t resize_handle     = 0;
    unsigned int x_start     = 0;
    unsigned int y_start     = 0;

    // ── Private helpers ───────────────────────────────────────────────────
    void normalize_button(Button *btn);
    void handle_collisions();

    // Build and load the right popup widgets for the currently selected object
    void load_property_popup();

    void rebuild_property_mirror();
    void rebuild_property_wall();
    void rebuild_property_source();
    void rebuild_property_laser();
    void rebuild_property_ray();
};

#endif // LIGHT_WINDOW_HPP