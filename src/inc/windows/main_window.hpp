#ifndef MAIN_WINDOW_HPP

#define MAIN_WINDOW_HPP
#include "window.hpp"
#include "object.hpp"
#include "spring.hpp"
#include "mass.hpp"
#include "button.hpp"
#include <vector>
#include <unordered_set>

#define G 9.81
#define DELTA_T 0.016

class MainWindow : public Window
{
    private:
    std::vector<Object*> objects;
    std::unordered_set<size_t> springs;
    std::unordered_set<size_t> masses;
    std::unordered_set<size_t> buttons;
    bool animating = true;
    bool dragging = false;
    size_t curr_object;
    unsigned int x_start, y_start;

    public:
    MainWindow(Theme *theme);
    void add_object(Object *object);
    virtual void main_loop() override;
    virtual void event_handler(SDL_Event &event) override;
    Theme *theme;
};

#endif