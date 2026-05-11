#include "objects/spring.hpp"

#include "objects/mass.hpp"

Spring::Spring(const std::vector<std::array<float,2>> &corners, float k_const, bool massless, float mass, Orientation orientation)
    : Object(corners, HitboxType::RECTANGLE, orientation)  // Call parent constructor in initializer list
{
    this->k_const = k_const;
    this->massless = massless;
    this->mass = mass;
    this->velocity = 0;
    this->equilibrium_pos_px = 0.0f;
    this->deformation_px = 0.0f;
}

bool Spring::is_mass_attached(const Mass *mass_obj) const
{
    for(const auto &entry : attached_objects)
    {
        if(entry.mass == mass_obj)
            return true;
    }
    return false;
}

void Spring::attach_mass(Mass *mass_obj, const std::array<float,2> &offset)
{
    if(!mass_obj)
        return;

    for(auto &entry : attached_objects)
    {
        if(entry.mass == mass_obj)
        {
            entry.offset = offset;
            return;
        }
    }

    attached_objects.push_back(AttachedObject{mass_obj, offset});
}

void Spring::detach_mass(Mass *mass_obj)
{
    if(!mass_obj)
        return;

    for(size_t i = 0; i < attached_objects.size(); ++i)
    {
        if(attached_objects[i].mass == mass_obj)
        {
            attached_objects.erase(attached_objects.begin() + (long)i);
            return;
        }
    }
}

float Spring::attached_mass_total() const
{
    float total = 0.0f;
    for(const auto &entry : attached_objects)
    {
        if(entry.mass)
            total += entry.mass->mass;
    }
    return total;
}

void Spring::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    Color::Color highlight = {255, 0, 0, 255};

    const int n = corners.size();
    for(int i = 0; i < n; i++)
    {
        int x1 = corners[i][0]*w;
        int y1 = corners[i][1]*h;
        int x2 = corners[(i+1)%n][0]*w;
        int y2 = corners[(i+1)%n][1]*h;

        if(i == static_cast<int>(orientation))
            Engine::draw_line(renderer, x1, y1, x2, y2, &highlight);
        else
            Engine::draw_line(renderer, x1, y1, x2, y2, &theme->foreground);
    }
}