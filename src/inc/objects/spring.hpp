#ifndef SPRING_HPP

#define SPRING_HPP
#include "object.hpp"
#include "engine.hpp"

class Mass;

struct AttachedObject
{
    Mass *mass = nullptr;
    std::array<float,2> offset = {0.0f, 0.0f};
};

class Spring : public Object
{
    public:
    Spring(const std::vector<std::array<float,2>> &corners, float k_const, bool massless, float mass, Orientation orientation);
    float k_const;
    float mass;
    float velocity;
    float equilibrium_pos_px;
    float deformation_px;
    bool massless;
    std::vector<AttachedObject> attached_objects;

    bool is_mass_attached(const Mass *mass_obj) const;
    void attach_mass(Mass *mass_obj, const std::array<float,2> &offset);
    void detach_mass(Mass *mass_obj);
    float attached_mass_total() const;

    void draw_object(SDL_Renderer *renderer, Theme *theme, int w, int h) override;

    ObjectType type() const override { return ObjectType::SPRING; }
};

#endif