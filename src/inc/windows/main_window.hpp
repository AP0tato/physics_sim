#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "windows/window.hpp"
#include "object.hpp"
#include "spring.hpp"
#include "mass.hpp"
#include "plane.hpp"
#include "button.hpp"
#include "checkbox.hpp"

#include <string>
#include <vector>
#include <unordered_set>

#define G       9.81
#define DELTA_T 0.016

class MainWindow : public Window
{
public:
    explicit MainWindow(Theme *theme);

    void add_object(Object *object);
    void toggle_playing();

    void main_loop()                   override;
    void event_handler(SDL_Event &ev)  override;
    void check_collisions();

private:
    // ── Editor / playback state ───────────────────────────────────────────
    enum class ActiveSlider { NONE, MASS, SPRING_K, SPRING_MASS };
    enum class ActiveInput  { NONE, MASS, SPRING_K, SPRING_MASS };

    struct RuntimeSnapshot
    {
        std::vector<std::array<float,2>> corners;
        std::vector<std::array<float,2>> base_shape;
        Orientation orientation      = Orientation::NONE;
        float       mass_velocity_x  = 0.0f;
        float       mass_velocity_y  = 0.0f;
    };

    // ── Scene ─────────────────────────────────────────────────────────────
    std::vector<Object*>              objects;
    std::unordered_set<size_t>        masses;
    std::unordered_set<size_t>        buttons;
    std::unordered_set<unsigned long> planes;

    // ── UI widgets ────────────────────────────────────────────────────────
    Button   *play_button      = nullptr;
    CheckBox  massless_checkbox;
    CheckBox  anchor_checkbox;

    // ── Interaction state ─────────────────────────────────────────────────
    bool         playing           = false;
    bool         animating         = true;
    bool         dragging          = false;
    bool         resizing          = false;
    bool         has_selection     = false;
    bool         show_property_popup = false;
    size_t       curr_object       = 0;
    size_t       resize_handle     = 0;
    unsigned int x_start           = 0;
    unsigned int y_start           = 0;
    int          last_dx           = 0;
    int          last_dy           = 0;
    int          drag_anchor_x     = 0;
    int          drag_anchor_y     = 0;
    ActiveSlider active_slider     = ActiveSlider::NONE;
    ActiveInput  active_input      = ActiveInput::NONE;
    std::string  property_input;

    // ── Snapshot ──────────────────────────────────────────────────────────
    std::vector<RuntimeSnapshot> runtime_snapshot;
    bool                         has_runtime_snapshot = false;

    // ── Private helpers ───────────────────────────────────────────────────
    void capture_runtime_snapshot();
    void restore_runtime_snapshot();

    // Physics step — does NOT draw; caller draws separately
    void step_gravity(Object *object, int w, int h);
};

#endif // MAIN_WINDOW_HPP