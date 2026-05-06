#include "windows/property_popup.hpp"
#include "objects/slider.hpp"
#include "objects/checkbox.hpp"
#include "objects/textfield.hpp"
#include "objects/togglebox.hpp"

namespace
{
    const float property_popup_width       = 280.0f;
    const float property_popup_elem_height = 32.0f;
    const float property_popup_padding     = 8.0f;
    const float property_popup_top_padding = 28.0f;

    float clamp_value(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
}

PropertyPopup::PropertyPopup(Theme *theme)
{
    this->theme = theme;
}

PropertyPopup::PropertyPopup()
{
    theme = new Theme();

    theme->background = Color::Color{35, 35, 35, 235};
    theme->foreground = Color::WHITE;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, 255); 

    uint8_t r = distr(gen);
    uint8_t g = distr(gen);
    uint8_t b = distr(gen);
    theme->border = Color::Color{r, g, b, 255};
}

std::string PropertyPopup::format_value(float value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    return buf;
}

void PropertyPopup::load(const Object* obj, std::vector<Object*> options, int w, int h)
{
    this->options = options;
    this->window_w = w;
    this->window_h = h;
    float left, top, right, bottom;
    obj->get_rect_bounds(left, top, right, bottom);
    const float lpx = left*w, tpx = top*h, rpx = right*w, bpx = bottom*h;
    const float ph = property_popup_top_padding +
                     options.size() * property_popup_elem_height +
                     (options.size() + 1) * property_popup_padding;
    float px = (lpx+rpx)*0.5f - property_popup_width*0.5f;
    px = clamp_value(px, 8.0f, (float)w-property_popup_width-8.0f);
    float py = tpx - property_popup_padding - ph - property_popup_padding;
    if(py < 8.0f) 
        py = bpx + property_popup_padding;
    py = clamp_value(py, 8.0f, (float)h-ph-8.0f);

    float current_y = py + property_popup_top_padding;
    float content_left = px + property_popup_padding;
    float content_right = px + property_popup_width - property_popup_padding;
    float content_width = content_right - content_left;

    for(size_t i = 0; i < options.size(); i++)
    {
        // Sliders span full width after padding
        if(Slider* slider = dynamic_cast<Slider*>(options[i])) {
            slider->on_property_popup_load(content_left, current_y, content_width, property_popup_elem_height);
        }
        // Checkboxes: square checkbox + text beside it
        else if(CheckBox* checkbox = dynamic_cast<CheckBox*>(options[i])) {
            checkbox->on_property_popup_load(content_left, current_y, content_width, property_popup_elem_height);
        }
        // TextFields: label + textfield spanning the rest of the width
        else if(TextField* textfield = dynamic_cast<TextField*>(options[i])) {
            textfield->on_property_popup_load(content_left, current_y, content_width, property_popup_elem_height);
        }
        else if(ToggleBox* togglebox = dynamic_cast<ToggleBox*>(options[i])) {
            togglebox->on_property_popup_load(content_left, current_y, content_width, property_popup_elem_height);
        }
        
        current_y += property_popup_elem_height + property_popup_padding;
    }

    // Update panel bounds
    panel = {px, py, property_popup_width, ph};
}

bool PropertyPopup::contains(int x, int y) const
{
    return x >= (int)panel.x && y >= (int)panel.y && x <= (int)(panel.x + panel.w) && y <= (int)(panel.y + panel.h);
}

bool PropertyPopup::handle_event(SDL_Event &event)
{
    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if(!contains(event.button.x, event.button.y))
            return false;

        for(Object *option : options)
        {
            if(Slider *slider = dynamic_cast<Slider*>(option))
            {
                if(slider->handle_mouse_down(event.button.x, event.button.y))
                    return true;
            }
            else if(CheckBox *checkbox = dynamic_cast<CheckBox*>(option))
            {
                if(checkbox->hit_test(event.button.x, event.button.y))
                {
                    checkbox->toggle();
                    return true;
                }
            }
            else if(ToggleBox *togglebox = dynamic_cast<ToggleBox*>(option))
            {
                if(togglebox->handle_mouse_down(event.button.x, event.button.y))
                    return true;
            }
        }
        return true;
    }

    if(event.type == SDL_EVENT_MOUSE_MOTION)
    {
        bool handled = false;
        for(Object *option : options)
        {
            if(Slider *slider = dynamic_cast<Slider*>(option))
                handled = slider->handle_mouse_motion(event.motion.x) || handled;
        }
        return handled;
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        bool handled = false;
        for(Object *option : options)
        {
            if(Slider *slider = dynamic_cast<Slider*>(option))
            {
                slider->handle_mouse_up();
                handled = true;
            }
        }
        return handled;
    }

    return false;
}

void PropertyPopup::draw(SDL_Renderer *renderer)
{
    draw_panel(renderer);
    // Draw all UI elements with proper window dimensions
    for(Object* option : options)
    {
        if(option) option->draw_object(renderer, theme, window_w, window_h);
    }
}

void PropertyPopup::draw_panel(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, theme->background.r, theme->background.g, theme->background.b, theme->background.a);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, theme->border.r, theme->border.g, theme->border.b, theme->border.a);
    SDL_RenderRect(renderer, &panel);
}
