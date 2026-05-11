#ifndef OBJECT_PAGE_HPP
#define OBJECT_PAGE_HPP

#include "windows/window.hpp"
#include "objects/button.hpp"
#include "objects/spring.hpp"
#include "objects/mass.hpp"
#include "objects/plane.hpp"
#include "objects/mirror.hpp"

#define BTN_WIDTH  180
#define BTN_HEIGHT 56

class MainWindow;
class LightWindow;

// Which object is being configured before placement
enum class PendingType { NONE, MASS, MIRROR };

class ObjectPage : public Window
{
public:
    ObjectPage(Window *main_window_ptr);
    ~ObjectPage();

    void main_loop()                   override;
    void event_handler(SDL_Event &ev)  override;

private:
    MainWindow  *ptr_main  = nullptr;
    LightWindow *ptr_light = nullptr;

    // ── Primary sidebar buttons ───────────────────────────────────────────
    std::vector<Button*> buttons;
    void normalize_button(Button *btn);

    // ── Pending-creation popup ────────────────────────────────────────────
    PendingType  pending_type = PendingType::NONE;

    // Mass config (shown for MASS pending)
    float       mass_mantissa  = 1.0f;   // 0.0 – 9.99
    int         mass_exponent  = 0;      // 0 – 50   →  value = mantissa × 10^exponent
    enum class ActiveMassSlider { NONE, MANTISSA, EXPONENT };
    ActiveMassSlider active_mass_slider = ActiveMassSlider::NONE;

    // Mirror config (shown for MIRROR pending)
    MirrorType mirror_shape   = MirrorType::FLAT;

    // ── Object placement ──────────────────────────────────────────────────
    void open_popup(PendingType type);
    void close_popup();
    void confirm_and_place();           // place the pending object

    // ── Popup drawing helpers ─────────────────────────────────────────────
    struct PopupLayout
    {
        SDL_FRect panel;
        // Mass rows
        SDL_FRect mantissa_slider;
        SDL_FRect exponent_slider;
        // Mirror rows (3 option buttons laid out inside panel)
        SDL_FRect mirror_flat;
        SDL_FRect mirror_concave;
        SDL_FRect mirror_convex;
        // Confirm / Cancel
        SDL_FRect confirm_btn;
        SDL_FRect cancel_btn;
    };
    PopupLayout build_layout(int w, int h) const;

    void draw_popup(int w, int h);
    bool handle_popup_click(int mx, int my, int w, int h);
    bool handle_popup_motion(int mx, int my, int w, int h);

    // ── Object factories ──────────────────────────────────────────────────
    void create_and_add(ObjectType type);     // simple objects (spring, plane, wall…)
    Spring* make_spring();
    Mass*   make_mass(float value);           // value = mantissa × 10^exponent
    Plane*  make_plane();
    Plane*  make_wall();
    Mirror*  make_mirror(MirrorType shape);
};

#endif // OBJECT_PAGE_HPP