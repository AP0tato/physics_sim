#ifndef OBJECT_LIST_WINDOW_HPP
#define OBJECT_LIST_WINDOW_HPP

#include "windows/window.hpp"
#include "objects/button.hpp"

#include <vector>

class ObjectListWindow : public Window
{
public:
    explicit ObjectListWindow(Window *source_window);
    ~ObjectListWindow() override;

    void main_loop() override;
    void event_handler(SDL_Event &event) override;

private:
    Window *source_window = nullptr;
    std::vector<Button*> controls;
    Button *play_button_control = nullptr;
    bool local_playing = false;
};

#endif // OBJECT_LIST_WINDOW_HPP