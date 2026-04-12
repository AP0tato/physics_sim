#ifndef MAIN_WINDOW_HPP

#define MAIN_WINDOW_HPP
#include "window.hpp"
#include "object.hpp"
#include "spring.hpp"
#include "mass.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "app_state.hpp"

#include <string>
#include <vector>
#include <unordered_set>

#define G 9.81
#define DELTA_T 0.016

class MainWindow : public Window
{
    private:
    enum class ActiveSlider { NONE, MASS, SPRING_K, SPRING_MASS };
    enum class ActiveInput { NONE, MASS, SPRING_K, SPRING_MASS };

    std::vector<Object*> objects;
    std::unordered_set<size_t> springs;
    std::unordered_set<size_t> masses;
    std::unordered_set<size_t> buttons;
    Button *play_button = nullptr;
    bool animating = true;
    bool playing = false;
    bool dragging = false;
    bool resizing = false;
    bool has_selection = false;
    bool show_property_popup = false;
    size_t curr_object = 0;
    size_t resize_handle = 0;
    unsigned int x_start, y_start;
    int last_dx = 0;
    int last_dy = 0;
    int drag_anchor_x = 0;
    int drag_anchor_y = 0;
    ActiveSlider active_slider = ActiveSlider::NONE;
    ActiveInput active_input = ActiveInput::NONE;
    std::string property_input;
    CheckBox massless_checkbox;

    public:
    MainWindow(Theme *theme);
    void add_object(Object *object);
    void toggle_playing();
    Theme *theme;

    virtual void main_loop() override;
    virtual void event_handler(SDL_Event &event) override;
};

#endif