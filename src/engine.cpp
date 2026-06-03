#include "engine.hpp"
#include "objects/physicsobject.hpp"

#include <algorithm>
#include <cmath>

namespace Engine
{
    namespace
    {
        struct AABB
        {
            float min_x = 0.0f;
            float min_y = 0.0f;
            float min_z = 0.0f;
            float max_x = 0.0f;
            float max_y = 0.0f;
            float max_z = 0.0f;
        };

        AABB aabb_from_object(const PhysicsObject *object)
        {
            AABB bounds;
            if(object->hitbox_type == HitboxType::ELLIPSE)
            {
                bounds.min_x = object->hitbox[0][0] - object->hitbox[1][0];
                bounds.min_y = object->hitbox[0][1] - object->hitbox[1][1];
                bounds.max_x = object->hitbox[0][0] + object->hitbox[1][0];
                bounds.max_y = object->hitbox[0][1] + object->hitbox[1][1];
                return bounds;
            }

            bounds.min_x = object->hitbox[0][0];
            bounds.min_y = object->hitbox[0][1];
            bounds.max_x = object->hitbox[1][0];
            bounds.max_y = object->hitbox[1][1];
            return bounds;
        }

        inline float cross_2d(float ax, float ay, float bx, float by)
        {
            return ax * by - ay * bx;
        }

        bool point_in_polygon_2d(float px, float py, const std::vector<std::vector<float>> &polygon)
        {
            if(polygon.size() < 3)
                return false;

            bool inside = false;
            for(size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++)
            {
                const float xi = polygon[i][0];
                const float yi = polygon[i][1];
                const float xj = polygon[j][0];
                const float yj = polygon[j][1];

                const bool intersects = ((yi > py) != (yj > py)) &&
                    (px < (xj - xi) * (py - yi) / ((yj - yi) + 1e-12f) + xi);
                if(intersects)
                    inside = !inside;
            }

            return inside;
        }

        bool segment_intersect_2d(float x1, float y1, float x2, float y2,
                                  float x3, float y3, float x4, float y4)
        {
            const float r_x = x2 - x1;
            const float r_y = y2 - y1;
            const float s_x = x4 - x3;
            const float s_y = y4 - y3;
            const float denom = cross_2d(r_x, r_y, s_x, s_y);

            const float qpx = x3 - x1;
            const float qpy = y3 - y1;

            if(std::fabs(denom) < 1e-6f)
            {
                if(std::fabs(cross_2d(qpx, qpy, r_x, r_y)) >= 1e-6f)
                    return false;

                const float rr = r_x * r_x + r_y * r_y;
                if(rr < 1e-12f)
                    return false;

                const float t0 = (qpx * r_x + qpy * r_y) / rr;
                const float t1 = ((x4 - x1) * r_x + (y4 - y1) * r_y) / rr;
                const float min_t = std::min(t0, t1);
                const float max_t = std::max(t0, t1);
                return max_t >= 0.0f && min_t <= 1.0f;
            }

            const float t = cross_2d(qpx, qpy, s_x, s_y) / denom;
            const float u = cross_2d(qpx, qpy, r_x, r_y) / denom;
            return t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f;
        }

        bool segment_intersects_ellipse_2d(float x1, float y1, float x2, float y2,
                                           const std::array<float, 3> &center,
                                           const std::array<float, 3> &radius)
        {
            const float cx = center[0];
            const float cy = center[1];
            const float rx = radius[0];
            const float ry = radius[1];
            if(rx <= 0.0f || ry <= 0.0f)
                return false;

            const float nx1 = (x1 - cx) / rx;
            const float ny1 = (y1 - cy) / ry;
            const float nx2 = (x2 - cx) / rx;
            const float ny2 = (y2 - cy) / ry;

            if(nx1 * nx1 + ny1 * ny1 <= 1.0f || nx2 * nx2 + ny2 * ny2 <= 1.0f)
                return true;

            const float dx = nx2 - nx1;
            const float dy = ny2 - ny1;
            const float a = dx * dx + dy * dy;
            const float b = 2.0f * (nx1 * dx + ny1 * dy);
            const float c = nx1 * nx1 + ny1 * ny1 - 1.0f;
            const float disc = b * b - 4.0f * a * c;
            if(disc < 0.0f)
                return false;

            const float sqrt_disc = std::sqrt(disc);
            const float inv_2a = 0.5f / a;
            const float t0 = (-b - sqrt_disc) * inv_2a;
            const float t1 = (-b + sqrt_disc) * inv_2a;
            return (t0 >= 0.0f && t0 <= 1.0f) || (t1 >= 0.0f && t1 <= 1.0f);
        }

        bool generic_narrow_2d(PhysicsObject *a, PhysicsObject *b)
        {
            if(a->hitbox_type == HitboxType::ELLIPSE && b->hitbox_type == HitboxType::ELLIPSE)
            {
                const float dx = a->hitbox[0][0] - b->hitbox[0][0];
                const float dy = a->hitbox[0][1] - b->hitbox[0][1];
                const float rx = a->hitbox[1][0] + b->hitbox[1][0];
                const float ry = a->hitbox[1][1] + b->hitbox[1][1];
                if(rx <= 0.0f || ry <= 0.0f)
                    return false;
                const float nx = dx / rx;
                const float ny = dy / ry;
                return nx * nx + ny * ny <= 1.0f;
            }

            if(a->hitbox_type == HitboxType::ELLIPSE || b->hitbox_type == HitboxType::ELLIPSE)
            {
                const PhysicsObject *ellipse = (a->hitbox_type == HitboxType::ELLIPSE) ? a : b;
                const PhysicsObject *poly    = (a->hitbox_type == HitboxType::ELLIPSE) ? b : a;

                for(size_t i = 0; i < poly->corners.size(); ++i)
                {
                    const auto &p0 = poly->corners[i];
                    const auto &p1 = poly->corners[(i + 1) % poly->corners.size()];
                    if(segment_intersects_ellipse_2d(p0[0], p0[1], p1[0], p1[1], ellipse->hitbox[0], ellipse->hitbox[1]))
                        return true;
                }

                if(!poly->corners.empty() && point_in_polygon_2d(ellipse->hitbox[0][0], ellipse->hitbox[0][1], poly->corners))
                    return true;

                return false;
            }

            for(size_t i = 0; i < a->corners.size(); ++i)
            {
                const auto &a0 = a->corners[i];
                const auto &a1 = a->corners[(i + 1) % a->corners.size()];
                for(size_t j = 0; j < b->corners.size(); ++j)
                {
                    const auto &b0 = b->corners[j];
                    const auto &b1 = b->corners[(j + 1) % b->corners.size()];
                    if(segment_intersect_2d(a0[0], a0[1], a1[0], a1[1], b0[0], b0[1], b1[0], b1[1]))
                        return true;
                }
            }

            if(!a->corners.empty() && point_in_polygon_2d(a->corners[0][0], a->corners[0][1], b->corners))
                return true;
            if(!b->corners.empty() && point_in_polygon_2d(b->corners[0][0], b->corners[0][1], a->corners))
                return true;

            return false;
        }

        bool generic_narrow_3d(PhysicsObject *a, PhysicsObject *b)
        {
            (void)a;
            (void)b;
            return false;
        }
    }

    bool aabb_overlap(float min_x1, float min_y1,
                      float max_x1, float max_y1,
                      float min_x2, float min_y2,
                      float max_x2, float max_y2)
    {
        return (max_x1 > min_x2 && min_x1 < max_x2) &&
               (max_y1 > min_y2 && min_y1 < max_y2);
    }

    bool aabb_overlap(float min_x1, float min_y1, float min_z1,
                      float max_x1, float max_y1, float max_z1,
                      float min_x2, float min_y2, float min_z2,
                      float max_x2, float max_y2, float max_z2)
    {
        return (max_x1 > min_x2 && min_x1 < max_x2) &&
               (max_y1 > min_y2 && min_y1 < max_y2) &&
               (max_z1 > min_z2 && min_z1 < max_z2);
    }

    bool is_collision(PhysicsObject *a, PhysicsObject *b)
    {
        if(!a || !b)
            return false;

        if(a->dimension() != b->dimension())
            return false;

        const AABB bb_a = aabb_from_object(a);
        const AABB bb_b = aabb_from_object(b);

        if(a->dimension() == SimDimension::DIM_2D)
        {
            if(!aabb_overlap(bb_a.min_x, bb_a.min_y, bb_a.max_x, bb_a.max_y,
                             bb_b.min_x, bb_b.min_y, bb_b.max_x, bb_b.max_y))
                return false;
        }
        else
        {
            if(!aabb_overlap(bb_a.min_x, bb_a.min_y, bb_a.min_z,
                             bb_a.max_x, bb_a.max_y, bb_a.max_z,
                             bb_b.min_x, bb_b.min_y, bb_b.min_z,
                             bb_b.max_x, bb_b.max_y, bb_b.max_z))
                return false;
        }

        if(a->dimension() == SimDimension::DIM_2D)
        {
            switch(a->domain())
            {
                case SimDomain::LIGHT: return generic_narrow_2d(a, b);
                case SimDomain::RIGID: return generic_narrow_2d(a, b);
                case SimDomain::FLUID: return generic_narrow_2d(a, b);
            }
        }
        else
        {
            switch(a->domain())
            {
                case SimDomain::LIGHT: return generic_narrow_3d(a, b);
                case SimDomain::RIGID: return generic_narrow_3d(a, b);
                case SimDomain::FLUID: return generic_narrow_3d(a, b);
            }
        }

        return false;
    }

    void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2,
                   Color::Color *color)
    {
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }
}
