#ifndef OBJECT_PAGE_HPP

#define OBJECT_PAGE_HPP

#include "windows/window.hpp"
#include "button.hpp"

class ObjectPage : public Window
{
    private:
    std::vector<Button*> buttons;

    public:
    ObjectPage();
};

#endif
