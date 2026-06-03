#include "objects/spring.hpp"
#include "objects/mass.hpp"

// =============================================================================
// Spring2D
// =============================================================================
Spring2D::Spring2D(const std::vector<std::vector<float>> &corners,
                   float k_const, bool massless, float mass, Orientation orientation)
    : PhysicsObject2D(corners, HitboxType::RECTANGLE, orientation)
{
    this->k_const            = k_const;
    this->massless           = massless;
    this->mass               = mass;
    this->equilibrium_pos_px = 0.0f;
    this->deformation_px     = 0.0f;
}

bool Spring2D::is_mass_attached(const Mass2D *mass_obj) const
{
    for(const auto &entry : attached_objects)
        if(entry.mass == mass_obj) return true;
    return false;
}

void Spring2D::attach_mass(Mass2D *mass_obj, const std::array<float, 2> &offset)
{
    if(!mass_obj) return;
    for(auto &entry : attached_objects)
    {
        if(entry.mass == mass_obj) { entry.offset = offset; return; }
    }
    attached_objects.push_back(AttachedObject2D{mass_obj, offset});
}

void Spring2D::detach_mass(Mass2D *mass_obj)
{
    if(!mass_obj) return;
    for(size_t i = 0; i < attached_objects.size(); ++i)
    {
        if(attached_objects[i].mass == mass_obj)
        {
            attached_objects.erase(attached_objects.begin() + (long)i);
            return;
        }
    }
}

float Spring2D::attached_mass_total() const
{
    float total = 0.0f;
    for(const auto &entry : attached_objects)
        if(entry.mass) total += entry.mass->mass;
    return total;
}

void Spring2D::draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h)
{
    Color::Color highlight = {255, 0, 0, 255};
    const int n = corners.size();
    for(int i = 0; i < n; i++)
    {
        int x1 = corners[i][0] * w,         y1 = corners[i][1] * h;
        int x2 = corners[(i+1)%n][0] * w,   y2 = corners[(i+1)%n][1] * h;
        if(i == static_cast<int>(orientation))
            Engine::draw_line(renderer, x1, y1, x2, y2, &highlight);
        else
            Engine::draw_line(renderer, x1, y1, x2, y2, &theme->foreground);
    }
}

// =============================================================================
// Spring3D — skeleton
// =============================================================================
Spring3D::Spring3D(const std::vector<std::vector<float>> &corners,
                   float k_const, bool massless, float mass, Orientation orientation)
    : PhysicsObject3D(corners, HitboxType::RECTANGLE, orientation)
{
    this->k_const            = k_const;
    this->massless           = massless;
    this->mass               = mass;
    this->equilibrium_pos_px = 0.0f;
    this->deformation_px     = 0.0f;
    // TODO: triangulate corners into faces, implement 3D force application.
}