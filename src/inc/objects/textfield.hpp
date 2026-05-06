#ifndef TEXTFIELD_HPP

#define TEXTFIELD_HPP

#include "objects/object.hpp"
#include <SDL3/SDL.h>

class TextField : public Object
{
    private:
    SDL_FRect   panel;
    std::string value;
    bool        active;

    public:
    TextField(const std::vector<std::array<float,2>> &corners, HitboxType hitbox_type, Orientation orientation, const std::string &value = "");

    virtual void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;
    void on_property_popup_load(float x, float y, float width, float height) override;
    ObjectType type() const override { return ObjectType::TEXTFIELD; }

};

#endif