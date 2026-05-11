#include "engine.hpp"
#include "objects/object.hpp"


namespace Engine
{
    namespace
    {
        bool collision_helper();
    }
    
    void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, Color::Color *color)
    {
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }

    bool is_collision(Object *a, Object *b)
    {
        if(a->hitbox.size() == 0 || b->hitbox.size() == 0)
            return false;

        bool a_bigger = a->hitbox.size() > b->hitbox.size();
        auto &smaller = !a_bigger ? a->hitbox : b->hitbox;
        auto &bigger = a_bigger ? a->hitbox : b->hitbox;

        HitboxType bigger_t = a_bigger ? a->hitbox_type : b->hitbox_type;
        HitboxType smaller_t = !a_bigger ? a->hitbox_type : b->hitbox_type;

        float sfl, sfr, sfu, sfd;
        float fl = sfl = INFINITY, fr = sfr = 0, fu = sfu = INFINITY, fd = sfd = 0;

        size_t s = 0, b = 0;
        if(bigger_t == HitboxType::ELLIPSE)
        {
            fl = bigger[0][0] - bigger[0][2];
            fr = bigger[0][0] + bigger[0][2];

            fu = bigger[0][1] - bigger[0][3];
            fu = bigger[0][1] + bigger[0][3];
        }
        else
        {
            for(auto &i : bigger)
            {
                if(i[1] < fu)
                    fl = i[1];
                if(i[1] > fd)
                    fd = i[1];

                if(i[0] < fl)
                    fl = i[0];
                if(i[1] > fr)
                    fr = i[0];
                ++b;
            }
        }

        if(smaller_t == HitboxType::ELLIPSE)
        {
            sfl = smaller[0][0] - smaller[0][2];
            sfr = smaller[0][0] + smaller[0][2];

            sfu = smaller[0][1] - smaller[0][3];
            sfu = smaller[0][1] + smaller[0][3];
        }
        else
        {
            for(auto &i : smaller)
            {
                if(i[1] < sfu)
                    sfl = i[1];
                if(i[1] > sfd)
                    sfd = i[1];

                if(i[0] < sfl)
                    sfl = i[0];
                if(i[1] > sfr)
                    sfr = i[0];
                ++s;
            }
        }

        if(sfr > fl && sfl < fr && sfu < fd && sfd > fu)
            return false;

        
    }
}