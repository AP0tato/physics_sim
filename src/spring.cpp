#include "spring.hpp"

Spring::Spring(const std::vector<std::array<float,2>> &corners, float k_const, bool massless, float mass, Orientation orientation)
    : Object(corners, RECTANGLE, orientation)  // Call parent constructor in initializer list
{
    this->k_const = k_const;
    this->massless = massless;
    this->mass = mass;
    this->velocity = 0;
}

void Spring::draw_object(SDL_Renderer *renderer, Theme *theme)
{
    Color highlight = {255, 0, 0, 255};

    const int n = corners.size();
    for(int i = 0; i < n; i++)
    {
        int x1 = corners[i][0];
        int y1 = corners[i][1];
        int x2 = corners[(i+1)%n][0];
        int y2 = corners[(i+1)%n][1];

        if(i == orientation)
            draw_line(renderer, x1, y1, x2, y2, &highlight);
        else
            draw_line(renderer, x1, y1, x2, y2, &theme->foreground);
    }
}