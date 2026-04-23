#include "windows/main_window.hpp"
#include "windows/object_page.hpp"

#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

#include <SDL3_ttf/SDL_ttf.h>

namespace {
constexpr float selection_padding_px = 10.0f;
constexpr float selection_handle_size = 9.0f;
constexpr int selection_handle_pick_radius = 10;
constexpr float property_popup_width = 280.0f;
constexpr float property_popup_height_mass = 90.0f;
constexpr float property_popup_height_spring = 170.0f;
constexpr float property_popup_margin = 10.0f;
constexpr float property_popup_lift_px = 20.0f;
constexpr float property_slider_height = 10.0f;
constexpr float property_input_height = 24.0f;
constexpr float property_checkbox_size = 18.0f;

struct PropertyPopupRects
{
    SDL_FRect panel{};
    SDL_FRect slider_1{};
    SDL_FRect input_1{};
    SDL_FRect slider_2{};
    SDL_FRect input_2{};
    SDL_FRect checkbox{};
    SDL_FRect orientation_button{};
};

size_t orientation_index(Orientation orientation)
{
    return static_cast<size_t>(orientation);
}

size_t next_orientation_index(Orientation orientation)
{
    return (orientation_index(orientation) + 1) % 4;
}

size_t opposite_orientation_index(Orientation orientation)
{
    return (orientation_index(orientation) + 2) % 4;
}

bool constrain_object_to_window(Object *object, float &shift_x, float &shift_y)
{
    shift_x = 0.0f;
    shift_y = 0.0f;

    float left = object->corners[0][0];
    float right = object->corners[0][0];
    float top = object->corners[0][1];
    float bottom = object->corners[0][1];

    for(const auto &corner : object->corners)
    {
        if(corner[0] < left) left = corner[0];
        if(corner[0] > right) right = corner[0];
        if(corner[1] < top) top = corner[1];
        if(corner[1] > bottom) bottom = corner[1];
    }

    if(left < 0.0f)
        shift_x = -left;
    else if(right > 1.0f)
        shift_x = 1.0f - right;

    if(top < 0.0f)
        shift_y = -top;
    else if(bottom > 1.0f)
        shift_y = 1.0f - bottom;

    if(std::fabs(shift_x) < 0.000001f && std::fabs(shift_y) < 0.000001f)
        return false;

    for(size_t i = 0; i < object->corners.size(); i++)
    {
        object->corners[i][0] += shift_x;
        object->corners[i][1] += shift_y;
        object->base_shape[i][0] += shift_x;
        object->base_shape[i][1] += shift_y;
    }

    return true;
}

void move_object_by_pixels(Object *object, int dx, int dy, int w, int h)
{
    const float delta_x = (float)dx / (float)w;
    const float delta_y = (float)dy / (float)h;

    for(size_t i = 0; i < object->corners.size(); i++)
    {
        object->corners[i][0] += delta_x;
        object->corners[i][1] += delta_y;

        object->base_shape[i][0] += delta_x;
        object->base_shape[i][1] += delta_y;
    }

    float shift_x = 0.0f;
    float shift_y = 0.0f;
    constrain_object_to_window(object, shift_x, shift_y);

    object->create_hitbox();
}

void get_rect_bounds(const Object *object, float &left, float &top, float &right, float &bottom)
{
    left = right = object->corners[0][0];
    top = bottom = object->corners[0][1];

    for(const auto &corner : object->corners)
    {
        if(corner[0] < left) left = corner[0];
        if(corner[0] > right) right = corner[0];
        if(corner[1] < top) top = corner[1];
        if(corner[1] > bottom) bottom = corner[1];
    }
}

TTF_Font* popup_font()
{
    static TTF_Font *font = nullptr;
    if(font)
        return font;

    const char* font_paths[] = {
        "assets/fonts/Roboto-Regular.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    for(const char* path : font_paths)
    {
        font = TTF_OpenFont(path, 14);
        if(font)
            break;
    }

    return font;
}

bool point_in_rect(int x, int y, const SDL_FRect &rect)
{
    return (
        x >= (int)rect.x &&
        y >= (int)rect.y &&
        x <= (int)(rect.x + rect.w) &&
        y <= (int)(rect.y + rect.h)
    );
}

float clamp_value(float v, float lo, float hi)
{
    if(v < lo) return lo;
    if(v > hi) return hi;
    return v;
}

float slider_value_from_mouse(int mouse_x, const SDL_FRect &slider, float min_val, float max_val)
{
    if(slider.w <= 1.0f)
        return min_val;

    const float t = clamp_value(((float)mouse_x - slider.x) / slider.w, 0.0f, 1.0f);
    return min_val + (max_val - min_val) * t;
}

float slider_x_from_value(float value, const SDL_FRect &slider, float min_val, float max_val)
{
    const float span = max_val - min_val;
    if(span <= 0.0f)
        return slider.x;

    const float t = clamp_value((value - min_val) / span, 0.0f, 1.0f);
    return slider.x + t * slider.w;
}

const char* orientation_text(Orientation orientation)
{
    switch(orientation)
    {
        case Orientation::UP: return "UP";
        case Orientation::RIGHT: return "RIGHT";
        case Orientation::DOWN: return "DOWN";
        case Orientation::LEFT: return "LEFT";
        default: return "NONE";
    }
}

void draw_popup_text(SDL_Renderer *renderer, const std::string &text, float x, float y, SDL_Color color)
{
    TTF_Font *font = popup_font();
    if(!font)
        return;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if(!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(!texture)
    {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_FRect dst = {x, y, (float)surface->w, (float)surface->h};
    SDL_RenderTexture(renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

std::string format_value(float value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    return std::string(buf);
}

PropertyPopupRects get_property_popup_rects(const Object *object, int w, int h, bool for_spring)
{
    PropertyPopupRects rects;

    float left, top, right, bottom;
    get_rect_bounds(object, left, top, right, bottom);

    const float left_px = left * w;
    const float top_px = top * h;
    const float right_px = right * w;
    const float bottom_px = bottom * h;

    const float popup_h = for_spring ? property_popup_height_spring : property_popup_height_mass;

    float panel_x = (left_px + right_px) * 0.5f - property_popup_width * 0.5f;
    if(panel_x < 8.0f)
        panel_x = 8.0f;
    if(panel_x + property_popup_width > w - 8.0f)
        panel_x = w - property_popup_width - 8.0f;

    float panel_y = top_px - property_popup_margin - popup_h;
    if(panel_y < 8.0f)
        panel_y = bottom_px + property_popup_margin;
    panel_y -= property_popup_lift_px;
    if(panel_y + popup_h > h - 8.0f)
        panel_y = h - popup_h - 8.0f;
    if(panel_y < 8.0f)
        panel_y = 8.0f;

    rects.panel = {panel_x, panel_y, property_popup_width, popup_h};

    const float row_1_y = panel_y + 16.0f;
    const float row_2_y = panel_y + 56.0f;

    rects.slider_1 = {panel_x + 74.0f, row_1_y + 7.0f, 130.0f, property_slider_height};
    rects.input_1 = {panel_x + 212.0f, row_1_y, 58.0f, property_input_height};

    if(for_spring)
    {
        rects.slider_2 = {panel_x + 74.0f, row_2_y + 7.0f, 130.0f, property_slider_height};
        rects.input_2 = {panel_x + 212.0f, row_2_y, 58.0f, property_input_height};
        rects.checkbox = {panel_x + 16.0f, panel_y + 98.0f, property_checkbox_size, property_checkbox_size};
        rects.orientation_button = {panel_x + 144.0f, panel_y + 126.0f, 126.0f, property_input_height};
    }

    return rects;
}

void draw_slider(SDL_Renderer *renderer, const SDL_FRect &slider, float value, float min_v, float max_v)
{
    SDL_SetRenderDrawColor(renderer, 95, 95, 95, 255);
    SDL_RenderFillRect(renderer, &slider);

    SDL_SetRenderDrawColor(renderer, 40, 200, 255, 255);
    SDL_FRect filled = slider;
    filled.w = slider_x_from_value(value, slider, min_v, max_v) - slider.x;
    if(filled.w < 0.0f) filled.w = 0.0f;
    SDL_RenderFillRect(renderer, &filled);

    const float thumb_x = slider_x_from_value(value, slider, min_v, max_v);
    SDL_FRect thumb = {thumb_x - 4.0f, slider.y - 4.0f, 8.0f, slider.h + 8.0f};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderFillRect(renderer, &thumb);
}

void draw_input_box(SDL_Renderer *renderer, const SDL_FRect &box, bool active)
{
    SDL_SetRenderDrawColor(renderer, 28, 28, 28, 255);
    SDL_RenderFillRect(renderer, &box);

    if(active)
        SDL_SetRenderDrawColor(renderer, 120, 220, 255, 255);
    else
        SDL_SetRenderDrawColor(renderer, 140, 140, 140, 255);

    SDL_RenderRect(renderer, &box);
}

void draw_property_popup(SDL_Renderer *renderer, const PropertyPopupRects &rects, bool for_spring)
{
    const SDL_Color border = for_spring ? SDL_Color{255, 120, 120, 255} : SDL_Color{120, 255, 160, 255};

    SDL_SetRenderDrawColor(renderer, 35, 35, 35, 235);
    SDL_RenderFillRect(renderer, &rects.panel);
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderRect(renderer, &rects.panel);
}

std::array<SDL_FPoint, 8> get_resize_handles_px(const Object *object, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(object, left, top, right, bottom);

    const float l = left * w - selection_padding_px;
    const float t = top * h - selection_padding_px;
    const float r = right * w + selection_padding_px;
    const float b = bottom * h + selection_padding_px;
    const float mx = (l + r) * 0.5f;
    const float my = (t + b) * 0.5f;

    return {
        SDL_FPoint{l, t},
        SDL_FPoint{mx, t},
        SDL_FPoint{r, t},
        SDL_FPoint{r, my},
        SDL_FPoint{r, b},
        SDL_FPoint{mx, b},
        SDL_FPoint{l, b},
        SDL_FPoint{l, my}
    };
}

bool try_get_resize_handle(const Object *object, int mouse_x, int mouse_y, int w, int h, size_t &handle_idx)
{
    constexpr int handle_pick_radius_sq = selection_handle_pick_radius * selection_handle_pick_radius;

    auto handles = get_resize_handles_px(object, w, h);
    for(size_t i = 0; i < handles.size(); i++)
    {
        const int dx = mouse_x - (int)handles[i].x;
        const int dy = mouse_y - (int)handles[i].y;
        if(dx * dx + dy * dy <= handle_pick_radius_sq)
        {
            handle_idx = i;
            return true;
        }
    }

    return false;
}

void set_rect_from_bounds(Object *object, float left, float top, float right, float bottom)
{
    object->corners[0] = {left, top};
    object->corners[1] = {right, top};
    object->corners[2] = {right, bottom};
    object->corners[3] = {left, bottom};

    object->base_shape[0] = {left, top};
    object->base_shape[1] = {right, top};
    object->base_shape[2] = {right, bottom};
    object->base_shape[3] = {left, bottom};

    object->create_hitbox();
}

std::array<float,2> object_center(const Object *object)
{
    float left, top, right, bottom;
    get_rect_bounds(object, left, top, right, bottom);
    return {(left + right) * 0.5f, (top + bottom) * 0.5f};
}

bool rect_overlap_normalized(const Object *a, const Object *b)
{
    float al, at, ar, ab;
    float bl, bt, br, bb;
    get_rect_bounds(a, al, at, ar, ab);
    get_rect_bounds(b, bl, bt, br, bb);

    return !(ar < bl || br < al || ab < bt || bb < at);
}

std::array<float,2> spring_attachment_point(const Spring *spring)
{
    const size_t o_idx = orientation_index(spring->orientation);
    const size_t next_idx = next_orientation_index(spring->orientation);
    return {
        (spring->corners[o_idx][0] + spring->corners[next_idx][0]) * 0.5f,
        (spring->corners[o_idx][1] + spring->corners[next_idx][1]) * 0.5f
    };
}

std::array<float,2> edge_attachment_offset_for_mass(const Spring *spring, const Mass *mass_obj)
{
    const auto anchor = spring_attachment_point(spring);
    float left, top, right, bottom;
    get_rect_bounds(mass_obj, left, top, right, bottom);
    const float half_w = (right - left) * 0.5f;
    const float half_h = (bottom - top) * 0.5f;

    (void)anchor;

    // Keep attached mass centered on the edge normal axis and touching on the edge axis.
    std::array<float,2> offset = {0.0f, 0.0f};

    switch(spring->orientation)
    {
        case Orientation::UP:
            offset[0] = 0.0f;
            offset[1] = -half_h;
            break;
        case Orientation::DOWN:
            offset[0] = 0.0f;
            offset[1] = half_h;
            break;
        case Orientation::LEFT:
            offset[0] = -half_w;
            offset[1] = 0.0f;
            break;
        case Orientation::RIGHT:
            offset[0] = half_w;
            offset[1] = 0.0f;
            break;
        default:
            break;
    }

    return offset;
}

void move_object_center_to(Object *object, float target_cx, float target_cy)
{
    const auto center = object_center(object);
    const float dx = target_cx - center[0];
    const float dy = target_cy - center[1];

    for(size_t i = 0; i < object->corners.size(); i++)
    {
        object->corners[i][0] += dx;
        object->corners[i][1] += dy;
        object->base_shape[i][0] += dx;
        object->base_shape[i][1] += dy;
    }

    object->create_hitbox();
}

void sync_attached_objects(Spring *spring)
{
    if(!spring)
        return;

    const auto anchor = spring_attachment_point(spring);
    for(auto &entry : spring->attached_objects)
    {
        if(!entry.mass)
            continue;

        // Recompute edge offset from current orientation and mass size.
        entry.offset = edge_attachment_offset_for_mass(spring, entry.mass);

        const float target_x = anchor[0] + entry.offset[0];
        const float target_y = anchor[1] + entry.offset[1];
        move_object_center_to(entry.mass, target_x, target_y);
    }
}

float spring_effective_mass(const Spring *spring)
{
    if(!spring)
        return 0.001f;

    float total = spring->massless ? 0.0f : spring->mass;
    total += spring->attached_mass_total();
    if(total < 0.001f)
        total = 0.001f;
    return total;
}

bool is_mass_attached_to_any_spring(const Mass *mass_obj, const std::vector<Object*> &objects, const std::unordered_set<size_t> &springs)
{
    if(!mass_obj)
        return false;

    for(size_t idx : springs)
    {
        Spring *spring = dynamic_cast<Spring*>(objects[idx]);
        if(spring && spring->is_mass_attached(mass_obj))
            return true;
    }

    return false;
}

void resolve_spring_push_collision(
    Spring *spring,
    float prev_edge_px,
    int w,
    int h,
    const std::vector<Object*> &objects,
    const std::unordered_set<size_t> &masses,
    const std::unordered_set<size_t> &springs
)
{
    if(!spring)
        return;

    const Orientation o = spring->orientation;
    const size_t o_idx = orientation_index(o);
    const size_t next_idx = next_orientation_index(o);
    const bool horizontal = (o == Orientation::LEFT || o == Orientation::RIGHT);
    const float edge_px = horizontal ? spring->corners[o_idx][0] * w : spring->corners[o_idx][1] * h;
    const float travel = edge_px - prev_edge_px;
    if(std::fabs(travel) < 0.01f)
        return;

    const float outward_sign = (o == Orientation::RIGHT || o == Orientation::DOWN) ? 1.0f : -1.0f;
    // Only push when the red edge is moving outward, not retracting back through objects.
    if(travel * outward_sign <= 0.0f)
        return;

    auto try_push_from_face = [&](float prev_face_px, float curr_face_px, float source_min_ortho, float source_max_ortho) -> bool
    {
        for(size_t idx : masses)
        {
            Mass *mass_obj = dynamic_cast<Mass*>(objects[idx]);
            if(!mass_obj)
                continue;

            if(is_mass_attached_to_any_spring(mass_obj, objects, springs))
                continue;

            float left, top, right, bottom;
            get_rect_bounds(mass_obj, left, top, right, bottom);

            const float mass_left_px = left * w;
            const float mass_right_px = right * w;
            const float mass_top_px = top * h;
            const float mass_bottom_px = bottom * h;

            const float mass_min_ortho = horizontal ? mass_top_px : mass_left_px;
            const float mass_max_ortho = horizontal ? mass_bottom_px : mass_right_px;
            if(mass_max_ortho < source_min_ortho || mass_min_ortho > source_max_ortho)
                continue;

            bool crossed = false;
            if(horizontal)
            {
                if(o == Orientation::RIGHT)
                {
                    const float face_px = mass_left_px;
                    crossed = (prev_face_px <= face_px && curr_face_px >= face_px);
                }
                else
                {
                    const float face_px = mass_right_px;
                    crossed = (prev_face_px >= face_px && curr_face_px <= face_px);
                }
            }
            else
            {
                if(o == Orientation::DOWN)
                {
                    const float face_px = mass_top_px;
                    crossed = (prev_face_px <= face_px && curr_face_px >= face_px);
                }
                else
                {
                    const float face_px = mass_bottom_px;
                    crossed = (prev_face_px >= face_px && curr_face_px <= face_px);
                }
            }

            if(!crossed)
                continue;

            const float m1 = spring_effective_mass(spring);
            const float m2 = mass_obj->mass < 0.001f ? 0.001f : mass_obj->mass;
            const float v1 = spring->velocity;
            const float v2 = horizontal ? mass_obj->velocity_x : mass_obj->velocity_y;
            const float denom = m1 + m2;

            const float new_v1 = ((m1 - m2) * v1 + (2.0f * m2 * v2)) / denom;
            const float new_v2 = ((2.0f * m1 * v1) + (m2 - m1) * v2) / denom;

            spring->velocity = new_v1;
            if(horizontal)
                mass_obj->velocity_x = new_v2;
            else
                mass_obj->velocity_y = new_v2;

            const float eps_x = 1.0f / (float)w;
            const float eps_y = 1.0f / (float)h;
            const auto center = object_center(mass_obj);
            if(horizontal)
            {
                const float half_w = (right - left) * 0.5f;
                const float face_norm = curr_face_px / (float)w;
                const float target_cx = (o == Orientation::RIGHT)
                    ? (face_norm + half_w + eps_x)
                    : (face_norm - half_w - eps_x);
                move_object_center_to(mass_obj, target_cx, center[1]);
            }
            else
            {
                const float half_h = (bottom - top) * 0.5f;
                const float face_norm = curr_face_px / (float)h;
                const float target_cy = (o == Orientation::DOWN)
                    ? (face_norm + half_h + eps_y)
                    : (face_norm - half_h - eps_y);
                move_object_center_to(mass_obj, center[0], target_cy);
            }

            // Resolve one contact per source-step for stability.
            return true;
        }

        return false;
    };

    const float edge_min_ortho = horizontal
        ? std::min(spring->corners[o_idx][1], spring->corners[next_idx][1]) * h
        : std::min(spring->corners[o_idx][0], spring->corners[next_idx][0]) * w;
    const float edge_max_ortho = horizontal
        ? std::max(spring->corners[o_idx][1], spring->corners[next_idx][1]) * h
        : std::max(spring->corners[o_idx][0], spring->corners[next_idx][0]) * w;

    // First, let the spring red edge push.
    if(try_push_from_face(prev_edge_px, edge_px, edge_min_ortho, edge_max_ortho))
        return;

    // Then, let attached masses on the spring act as pushers too.
    for(const auto &entry : spring->attached_objects)
    {
        const Mass *attached = entry.mass;
        if(!attached)
            continue;

        float al, at, ar, ab;
        get_rect_bounds(attached, al, at, ar, ab);

        const float source_face_curr = horizontal
            ? ((o == Orientation::RIGHT) ? (ar * w) : (al * w))
            : ((o == Orientation::DOWN) ? (ab * h) : (at * h));
        const float source_face_prev = source_face_curr - travel;
        const float source_min_ortho = horizontal ? (at * h) : (al * w);
        const float source_max_ortho = horizontal ? (ab * h) : (ar * w);

        if(try_push_from_face(source_face_prev, source_face_curr, source_min_ortho, source_max_ortho))
            return;
    }
}

void integrate_free_masses(
    int w,
    int h,
    const std::vector<Object*> &objects,
    const std::unordered_set<size_t> &masses,
    const std::unordered_set<size_t> &springs
)
{
    auto resolve_mass_against_source_face = [&](Mass *mass_obj,
                                                float prev_left_px,
                                                float prev_top_px,
                                                float prev_right_px,
                                                float prev_bottom_px,
                                                Orientation o,
                                                float source_face_px,
                                                float source_min_ortho,
                                                float source_max_ortho,
                                                float source_mass,
                                                float &source_velocity) -> bool {
        float left, top, right, bottom;
        get_rect_bounds(mass_obj, left, top, right, bottom);

        const float curr_left_px = left * w;
        const float curr_right_px = right * w;
        const float curr_top_px = top * h;
        const float curr_bottom_px = bottom * h;

        const bool horizontal = (o == Orientation::LEFT || o == Orientation::RIGHT);
        const float curr_min_ortho = horizontal ? curr_top_px : curr_left_px;
        const float curr_max_ortho = horizontal ? curr_bottom_px : curr_right_px;
        if(curr_max_ortho < source_min_ortho || curr_min_ortho > source_max_ortho)
            return false;

        bool crossed = false;
        if(o == Orientation::RIGHT)
            crossed = (prev_left_px >= source_face_px && curr_left_px < source_face_px);
        else if(o == Orientation::LEFT)
            crossed = (prev_right_px <= source_face_px && curr_right_px > source_face_px);
        else if(o == Orientation::DOWN)
            crossed = (prev_top_px >= source_face_px && curr_top_px < source_face_px);
        else if(o == Orientation::UP)
            crossed = (prev_bottom_px <= source_face_px && curr_bottom_px > source_face_px);

        if(!crossed)
            return false;

        const float m1 = source_mass < 0.001f ? 0.001f : source_mass;
        const float m2 = mass_obj->mass < 0.001f ? 0.001f : mass_obj->mass;
        const float v1 = source_velocity;
        const float v2 = horizontal ? mass_obj->velocity_x : mass_obj->velocity_y;
        const float denom = m1 + m2;

        const float new_v1 = ((m1 - m2) * v1 + (2.0f * m2 * v2)) / denom;
        const float new_v2 = ((2.0f * m1 * v1) + (m2 - m1) * v2) / denom;

        source_velocity = new_v1;
        if(horizontal)
            mass_obj->velocity_x = new_v2;
        else
            mass_obj->velocity_y = new_v2;

        const float eps_x = 1.0f / (float)w;
        const float eps_y = 1.0f / (float)h;
        float shift_x = 0.0f;
        float shift_y = 0.0f;

        if(o == Orientation::RIGHT)
        {
            const float target_left = source_face_px + 1.0f;
            shift_x = (target_left - curr_left_px) / (float)w + eps_x;
        }
        else if(o == Orientation::LEFT)
        {
            const float target_right = source_face_px - 1.0f;
            shift_x = (target_right - curr_right_px) / (float)w - eps_x;
        }
        else if(o == Orientation::DOWN)
        {
            const float target_top = source_face_px + 1.0f;
            shift_y = (target_top - curr_top_px) / (float)h + eps_y;
        }
        else if(o == Orientation::UP)
        {
            const float target_bottom = source_face_px - 1.0f;
            shift_y = (target_bottom - curr_bottom_px) / (float)h - eps_y;
        }

        for(size_t i = 0; i < mass_obj->corners.size(); i++)
        {
            mass_obj->corners[i][0] += shift_x;
            mass_obj->corners[i][1] += shift_y;
            mass_obj->base_shape[i][0] += shift_x;
            mass_obj->base_shape[i][1] += shift_y;
        }

        mass_obj->create_hitbox();
        return true;
    };

    for(size_t idx : masses)
    {
        Mass *mass_obj = dynamic_cast<Mass*>(objects[idx]);
        if(!mass_obj)
            continue;

        if(is_mass_attached_to_any_spring(mass_obj, objects, springs))
            continue;

        float prev_left, prev_top, prev_right, prev_bottom;
        get_rect_bounds(mass_obj, prev_left, prev_top, prev_right, prev_bottom);
        const float prev_left_px = prev_left * w;
        const float prev_top_px = prev_top * h;
        const float prev_right_px = prev_right * w;
        const float prev_bottom_px = prev_bottom * h;

        const float dx_norm = (mass_obj->velocity_x * DELTA_T) / (float)w;
        const float dy_norm = (mass_obj->velocity_y * DELTA_T) / (float)h;
        if(std::fabs(dx_norm) < 0.000001f && std::fabs(dy_norm) < 0.000001f)
            continue;

        for(size_t i = 0; i < mass_obj->corners.size(); i++)
        {
            mass_obj->corners[i][0] += dx_norm;
            mass_obj->corners[i][1] += dy_norm;
            mass_obj->base_shape[i][0] += dx_norm;
            mass_obj->base_shape[i][1] += dy_norm;
        }

        float shift_x = 0.0f;
        float shift_y = 0.0f;
        if(constrain_object_to_window(mass_obj, shift_x, shift_y))
        {
            if(std::fabs(shift_x) > 0.000001f)
                mass_obj->velocity_x = -mass_obj->velocity_x;
            if(std::fabs(shift_y) > 0.000001f)
                mass_obj->velocity_y = -mass_obj->velocity_y;
        }

        mass_obj->create_hitbox();

        bool handled_collision = false;
        for(size_t sidx : springs)
        {
            Spring *spring = dynamic_cast<Spring*>(objects[sidx]);
            if(!spring)
                continue;

            const Orientation o = spring->orientation;
            const size_t o_idx = orientation_index(o);
            const size_t next_idx = next_orientation_index(o);
            const bool horizontal = (o == Orientation::LEFT || o == Orientation::RIGHT);
            const float source_face_px = horizontal
                ? spring->corners[o_idx][0] * w
                : spring->corners[o_idx][1] * h;
            const float source_min_ortho = horizontal
                ? std::min(spring->corners[o_idx][1], spring->corners[next_idx][1]) * h
                : std::min(spring->corners[o_idx][0], spring->corners[next_idx][0]) * w;
            const float source_max_ortho = horizontal
                ? std::max(spring->corners[o_idx][1], spring->corners[next_idx][1]) * h
                : std::max(spring->corners[o_idx][0], spring->corners[next_idx][0]) * w;

            float &source_velocity = spring->velocity;
            const float source_mass = spring_effective_mass(spring);
            if(resolve_mass_against_source_face(
                mass_obj,
                prev_left_px,
                prev_top_px,
                prev_right_px,
                prev_bottom_px,
                o,
                source_face_px,
                source_min_ortho,
                source_max_ortho,
                source_mass,
                source_velocity
            ))
            {
                handled_collision = true;
                sync_attached_objects(spring);
                break;
            }

            for(const auto &entry : spring->attached_objects)
            {
                const Mass *attached = entry.mass;
                if(!attached)
                    continue;

                float al, at, ar, ab;
                get_rect_bounds(attached, al, at, ar, ab);

                const float attached_face_px = horizontal
                    ? ((o == Orientation::RIGHT) ? (ar * w) : (al * w))
                    : ((o == Orientation::DOWN) ? (ab * h) : (at * h));
                const float attached_min_ortho = horizontal ? (at * h) : (al * w);
                const float attached_max_ortho = horizontal ? (ab * h) : (ar * w);

                if(resolve_mass_against_source_face(
                    mass_obj,
                    prev_left_px,
                    prev_top_px,
                    prev_right_px,
                    prev_bottom_px,
                    o,
                    attached_face_px,
                    attached_min_ortho,
                    attached_max_ortho,
                    source_mass,
                    source_velocity
                ))
                {
                    handled_collision = true;
                    sync_attached_objects(spring);
                    break;
                }
            }

            if(handled_collision)
                break;
        }
    }
}

void detach_mass_from_all_springs(Mass *mass_obj, const std::vector<Object*> &objects, const std::unordered_set<size_t> &springs)
{
    if(!mass_obj)
        return;

    for(size_t idx : springs)
    {
        Spring *spring = dynamic_cast<Spring*>(objects[idx]);
        if(spring)
            spring->detach_mass(mass_obj);
    }
}

Spring* find_overlapping_spring_for_mass(Mass *mass_obj, const std::vector<Object*> &objects, const std::unordered_set<size_t> &springs)
{
    if(!mass_obj)
        return nullptr;

    for(size_t idx : springs)
    {
        Spring *spring = dynamic_cast<Spring*>(objects[idx]);
        if(!spring)
            continue;

        if(rect_overlap_normalized(mass_obj, spring))
            return spring;
    }

    return nullptr;
}

void erase_and_reindex_set(std::unordered_set<size_t> &set, size_t removed_idx)
{
    std::unordered_set<size_t> rebuilt;
    rebuilt.reserve(set.size());

    for(size_t idx : set)
    {
        if(idx == removed_idx)
            continue;

        rebuilt.insert(idx > removed_idx ? (idx - 1) : idx);
    }

    set.swap(rebuilt);
}

void draw_selection_frame(SDL_Renderer *renderer, const Object *object, int w, int h)
{
    float left, top, right, bottom;
    get_rect_bounds(object, left, top, right, bottom);

    SDL_SetRenderDrawColor(renderer, 80, 170, 255, 255);
    SDL_FRect frame = {
        left * w - selection_padding_px,
        top * h - selection_padding_px,
        (right - left) * w + (2.0f * selection_padding_px),
        (bottom - top) * h + (2.0f * selection_padding_px)
    };
    SDL_RenderRect(renderer, &frame);

    auto handles = get_resize_handles_px(object, w, h);
    for(const auto &handle : handles)
    {
        SDL_FRect dot = {
            handle.x - selection_handle_size * 0.5f,
            handle.y - selection_handle_size * 0.5f,
            selection_handle_size,
            selection_handle_size
        };
        SDL_RenderFillRect(renderer, &dot);
    }
}

void resize_rect_object_handle(Object *object, size_t handle_idx, int dx, int dy, int w, int h)
{
    if(object->corners.size() != 4)
        return;

    float left, top, right, bottom;
    get_rect_bounds(object, left, top, right, bottom);

    const float ndx = (float)dx / (float)w;
    const float ndy = (float)dy / (float)h;

    bool move_left = false;
    bool move_right = false;
    bool move_top = false;
    bool move_bottom = false;

    switch(handle_idx)
    {
        case 0: move_left = true; move_top = true; break;
        case 1: move_top = true; break;
        case 2: move_right = true; move_top = true; break;
        case 3: move_right = true; break;
        case 4: move_right = true; move_bottom = true; break;
        case 5: move_bottom = true; break;
        case 6: move_left = true; move_bottom = true; break;
        case 7: move_left = true; break;
        default: return;
    }

    if(move_left) left += ndx;
    if(move_right) right += ndx;
    if(move_top) top += ndy;
    if(move_bottom) bottom += ndy;

    constexpr float min_size_px = 16.0f;
    const float min_w = min_size_px / (float)w;
    const float min_h = min_size_px / (float)h;

    if(right - left < min_w)
    {
        if(move_left && !move_right) left = right - min_w;
        else right = left + min_w;
    }

    if(bottom - top < min_h)
    {
        if(move_top && !move_bottom) top = bottom - min_h;
        else bottom = top + min_h;
    }

    set_rect_from_bounds(object, left, top, right, bottom);
}
}

void open_objects_page(Window *main_window_ptr)
{
    printf("Button pressed\n");
    windows.push_back(new ObjectPage(main_window_ptr));
}


MainWindow::MainWindow(Theme *theme) : Window("Physics Sim", 1920, 1080) 
{ 
    this->theme = theme;
    massless_checkbox = CheckBox(0, 0, (int)property_checkbox_size, false);

    if(!SDL_SetWindowFullscreen(get_window(), true))
        std::cout << "Error entering fullscreen: " << SDL_GetError() << "\n";
    SDL_SyncWindow(get_window());

    int x, y;
    SDL_GetWindowSize(get_window(), &x, &y);

    const int w = 150;
    const int h = 50;
    const int top_margin = 10;
    const int right_margin = 10;
    const int button_gap = 10;

    const int object_page_x = x - w - right_margin;
    const int object_page_y = top_margin;
    const int play_x = object_page_x - w - button_gap;
    const int play_y = top_margin;

    play_button = new Button(play_x, play_y, w, h, "Play", [this]() { toggle_playing(); });
    this->add_object(play_button);

    this->add_object(new Button(object_page_x, object_page_y, w, h, "Object Page", [this]() { open_objects_page(this); }));
}

void MainWindow::capture_runtime_snapshot()
{
    runtime_snapshot.clear();
    runtime_snapshot.reserve(objects.size());

    for(size_t i = 0; i < objects.size(); i++)
    {
        RuntimeSnapshot snap;
        snap.corners = objects[i]->corners;
        snap.base_shape = objects[i]->base_shape;
        snap.orientation = objects[i]->orientation;

        if(masses.count(i))
        {
            Mass *mass = dynamic_cast<Mass*>(objects[i]);
            if(mass)
            {
                snap.mass_velocity_x = mass->velocity_x;
                snap.mass_velocity_y = mass->velocity_y;
            }
        }

        if(springs.count(i))
        {
            Spring *spring = dynamic_cast<Spring*>(objects[i]);
            if(spring)
            {
                snap.spring_velocity = spring->velocity;
                snap.spring_attached = spring->attached_objects;
            }
        }

        runtime_snapshot.push_back(std::move(snap));
    }

    has_runtime_snapshot = (runtime_snapshot.size() == objects.size());
}

void MainWindow::restore_runtime_snapshot()
{
    if(!has_runtime_snapshot || runtime_snapshot.size() != objects.size())
        return;

    for(size_t i = 0; i < objects.size(); i++)
    {
        objects[i]->corners = runtime_snapshot[i].corners;
        objects[i]->base_shape = runtime_snapshot[i].base_shape;
        objects[i]->orientation = runtime_snapshot[i].orientation;
        objects[i]->create_hitbox();

        if(masses.count(i))
        {
            Mass *mass = dynamic_cast<Mass*>(objects[i]);
            if(mass)
            {
                mass->velocity_x = runtime_snapshot[i].mass_velocity_x;
                mass->velocity_y = runtime_snapshot[i].mass_velocity_y;
            }
        }

        if(springs.count(i))
        {
            Spring *spring = dynamic_cast<Spring*>(objects[i]);
            if(spring)
            {
                spring->velocity = runtime_snapshot[i].spring_velocity;
                spring->attached_objects = runtime_snapshot[i].spring_attached;
            }
        }
    }
}

void MainWindow::toggle_playing()
{
    if(!playing)
    {
        capture_runtime_snapshot();
        playing = true;
        animating = true;
    }
    else
    {
        playing = false;
        animating = false;
        restore_runtime_snapshot();
        dragging = false;
        resizing = false;
        has_selection = false;
        show_property_popup = false;
    }

    active_slider = ActiveSlider::NONE;
    active_input = ActiveInput::NONE;
    property_input.clear();
    SDL_StopTextInput(get_window());
    if(play_button)
        play_button->label.set_text(playing ? "Stop" : "Play");
}

void MainWindow::add_object(Object *object)
{
    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);
    for(size_t i = 0; i < object->corners.size(); i++)
    {
        object->corners[i][0] /= (float)w;
        object->corners[i][1] /= (float)h;

        object->base_shape[i][0] /= (float)w;
        object->base_shape[i][1] /= (float)h;
    }
    
    object->create_hitbox();

    switch(object->type())
    {
        case ObjectType::BUTTON:
            buttons.insert(objects.size());
            break;
        case ObjectType::MASS:
            masses.insert(objects.size());
            break;
        case ObjectType::SPRING:
            springs.insert(objects.size());
            break;
        default:
            printf("Not a real object\n");
            break;
    }
    objects.push_back(object);
}

void MainWindow::event_handler(SDL_Event &event)
{
    Window::event_handler(event);
    if(!running)
        return;

    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);

    auto commit_property_input = [&]() {
        if(active_input == ActiveInput::NONE || curr_object >= objects.size())
            return;

        if(property_input.empty() || property_input == "." || property_input == "-")
        {
            active_input = ActiveInput::NONE;
            property_input.clear();
            SDL_StopTextInput(get_window());
            return;
        }

        try
        {
            const float parsed = std::stof(property_input);
            if(active_input == ActiveInput::MASS && masses.count(curr_object))
            {
                Mass *mass = dynamic_cast<Mass*>(objects[curr_object]);
                if(mass)
                    mass->mass = clamp_value(parsed, 0.1f, 100.0f);
            }
            else if(active_input == ActiveInput::SPRING_K && springs.count(curr_object))
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                    spring->k_const = clamp_value(parsed, 1.0f, 300.0f);
            }
            else if(active_input == ActiveInput::SPRING_MASS && springs.count(curr_object))
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                    spring->mass = clamp_value(parsed, 0.0f, 100.0f);
            }
        }
        catch(...)
        {
            // Ignore malformed input; keep current value.
        }

        active_input = ActiveInput::NONE;
        property_input.clear();
        SDL_StopTextInput(get_window());
    };

    if(!playing && has_selection && active_input == ActiveInput::NONE && curr_object < objects.size() && event.type == SDL_EVENT_KEY_DOWN)
    {
        if(!event.key.repeat && (event.key.key == SDLK_BACKSPACE || event.key.key == SDLK_DELETE))
        {
            if(masses.count(curr_object))
            {
                Mass *mass_obj = dynamic_cast<Mass*>(objects[curr_object]);
                detach_mass_from_all_springs(mass_obj, objects, springs);
            }

            delete objects[curr_object];
            objects.erase(objects.begin() + (long)curr_object);

            erase_and_reindex_set(masses, curr_object);
            erase_and_reindex_set(springs, curr_object);
            erase_and_reindex_set(buttons, curr_object);

            has_selection = false;
            show_property_popup = false;
            dragging = false;
            resizing = false;
            active_slider = ActiveSlider::NONE;
            property_input.clear();
            SDL_StopTextInput(get_window());

            return;
        }
    }

    if(!playing && has_selection && active_input != ActiveInput::NONE && event.type == SDL_EVENT_TEXT_INPUT)
    {
        for(const char *p = event.text.text; *p; ++p)
        {
            const char c = *p;
            if(std::isdigit((unsigned char)c))
                property_input.push_back(c);
            else if(c == '.' && property_input.find('.') == std::string::npos)
                property_input.push_back(c);
            else if(c == '-' && property_input.empty())
                property_input.push_back(c);
        }
        return;
    }

    if(!playing && has_selection && active_input != ActiveInput::NONE && event.type == SDL_EVENT_KEY_DOWN)
    {
        if(event.key.key == SDLK_BACKSPACE)
        {
            if(!property_input.empty())
                property_input.pop_back();
            return;
        }
        if(event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)
        {
            commit_property_input();
            return;
        }
        if(event.key.key == SDLK_ESCAPE)
        {
            active_input = ActiveInput::NONE;
            property_input.clear();
            SDL_StopTextInput(get_window());
            return;
        }
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
    {
        bool hit_any_object = false;

        if(!playing && has_selection && show_property_popup && curr_object < objects.size() && !buttons.count(curr_object))
        {
            const bool is_spring = springs.count(curr_object);
            const bool is_mass = masses.count(curr_object);
            PropertyPopupRects popup = get_property_popup_rects(objects[curr_object], w, h, is_spring);

            if(point_in_rect(event.button.x, event.button.y, popup.input_1))
            {
                active_slider = ActiveSlider::NONE;
                if(is_mass)
                {
                    Mass *mass = dynamic_cast<Mass*>(objects[curr_object]);
                    active_input = ActiveInput::MASS;
                    property_input = mass ? format_value(mass->mass) : "";
                }
                else if(is_spring)
                {
                    Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                    active_input = ActiveInput::SPRING_K;
                    property_input = spring ? format_value(spring->k_const) : "";
                }

                SDL_StartTextInput(get_window());
                return;
            }

            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.input_2))
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                active_slider = ActiveSlider::NONE;
                active_input = ActiveInput::SPRING_MASS;
                property_input = spring ? format_value(spring->mass) : "";
                SDL_StartTextInput(get_window());
                return;
            }

            if(point_in_rect(event.button.x, event.button.y, popup.slider_1))
            {
                active_input = ActiveInput::NONE;
                property_input.clear();
                SDL_StopTextInput(get_window());

                if(is_mass)
                {
                    Mass *mass = dynamic_cast<Mass*>(objects[curr_object]);
                    if(mass)
                        mass->mass = slider_value_from_mouse(event.button.x, popup.slider_1, 0.1f, 100.0f);
                    active_slider = ActiveSlider::MASS;
                }
                else if(is_spring)
                {
                    Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                    if(spring)
                        spring->k_const = slider_value_from_mouse(event.button.x, popup.slider_1, 1.0f, 300.0f);
                    active_slider = ActiveSlider::SPRING_K;
                }
                return;
            }

            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.slider_2))
            {
                active_input = ActiveInput::NONE;
                property_input.clear();
                SDL_StopTextInput(get_window());

                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                    spring->mass = slider_value_from_mouse(event.button.x, popup.slider_2, 0.0f, 100.0f);
                active_slider = ActiveSlider::SPRING_MASS;
                return;
            }

            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.checkbox))
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                {
                    massless_checkbox.set_position((int)popup.checkbox.x, (int)popup.checkbox.y);
                    massless_checkbox.set_size((int)popup.checkbox.w);
                    massless_checkbox.set_checked(spring->massless);
                    if(massless_checkbox.hit_test(event.button.x, event.button.y))
                        massless_checkbox.toggle();

                    spring->massless = massless_checkbox.is_checked();
                    if(spring->massless)
                        spring->mass = 0.0f;
                }
                return;
            }

            if(is_spring && point_in_rect(event.button.x, event.button.y, popup.orientation_button))
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                {
                    const size_t next = next_orientation_index(spring->orientation);
                    spring->orientation = static_cast<Orientation>(next);
                    spring->velocity = 0.0f;
                    sync_attached_objects(spring);
                }
                return;
            }

            if(point_in_rect(event.button.x, event.button.y, popup.panel))
                return;
        }

        // In pause mode, let handle clicks win even when handles sit outside object bounds.
        if(!playing && has_selection && curr_object < objects.size() && !buttons.count(curr_object))
        {
            size_t hit_handle = 0;
            if(try_get_resize_handle(objects[curr_object], event.button.x, event.button.y, w, h, hit_handle))
            {
                hit_any_object = true;
                resizing = true;
                dragging = false;
                resize_handle = hit_handle;
                x_start = event.button.x;
                y_start = event.button.y;
                last_dx = 0;
                last_dy = 0;
            }
        }

        if(hit_any_object)
            return;

        for(size_t i = 0; i < objects.size(); i++)
        {
            if(buttons.count(i) && objects[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                Button *curr = dynamic_cast<Button*>(objects[i]);
                curr->press();
                hit_any_object = true;
                break;
            }
            if((springs.count(i) || masses.count(i)) && objects[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                hit_any_object = true;
                y_start = event.button.y;
                x_start = event.button.x;
                curr_object = i;
                has_selection = true;
                last_dx = 0;
                last_dy = 0;
                active_input = ActiveInput::NONE;
                property_input.clear();
                SDL_StopTextInput(get_window());

                float left, top, right, bottom;
                get_rect_bounds(objects[i], left, top, right, bottom);
                drag_anchor_x = event.button.x - (int)(left * w);
                drag_anchor_y = event.button.y - (int)(top * h);

                if(!playing)
                {
                    show_property_popup = (event.button.clicks >= 2);
                    size_t hit_handle = 0;
                    resizing = try_get_resize_handle(objects[i], event.button.x, event.button.y, w, h, hit_handle);
                    dragging = !resizing;
                    if(resizing)
                        resize_handle = hit_handle;
                }
                else
                {
                    dragging = true;
                    resizing = false;
                }

                if(dragging && masses.count(curr_object))
                {
                    Mass *mass_obj = dynamic_cast<Mass*>(objects[curr_object]);
                    detach_mass_from_all_springs(mass_obj, objects, springs);
                }

                if(playing && springs.count(curr_object))
                {
                    Spring *curr = dynamic_cast<Spring*>(objects[curr_object]);
                    if(curr)
                    {
                        curr->velocity = 0.0f;
                        animating = false;
                    }
                }
                break;
            }
        }

        if(!hit_any_object)
        {
            has_selection = false;
            show_property_popup = false;
            active_slider = ActiveSlider::NONE;
            active_input = ActiveInput::NONE;
            property_input.clear();
            SDL_StopTextInput(get_window());
        }
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP) 
    {
        if(dragging && playing && springs.count(curr_object))
        {
            Spring *curr = dynamic_cast<Spring*>(objects[curr_object]);
            if(curr)
            {
                const bool horizontal = (curr->orientation == Orientation::LEFT || curr->orientation == Orientation::RIGHT);
                const float axis_delta = horizontal ? (float)last_dx : (float)last_dy;
                curr->velocity = axis_delta / DELTA_T;
            }
        }

        if(dragging && curr_object < objects.size() && masses.count(curr_object))
        {
            Mass *mass_obj = dynamic_cast<Mass*>(objects[curr_object]);
            Spring *target = find_overlapping_spring_for_mass(mass_obj, objects, springs);
            if(target)
            {
                target->attach_mass(mass_obj, edge_attachment_offset_for_mass(target, mass_obj));
                sync_attached_objects(target);
            }
        }

        dragging = false;
        resizing = false;
        active_slider = ActiveSlider::NONE;
        if(playing)
            animating = true;
    }

    if(event.type == SDL_EVENT_MOUSE_MOTION) 
    {
        if(!playing && has_selection && curr_object < objects.size() && active_slider != ActiveSlider::NONE)
        {
            const bool is_spring = springs.count(curr_object);
            PropertyPopupRects popup = get_property_popup_rects(objects[curr_object], w, h, is_spring);

            if(active_slider == ActiveSlider::MASS && masses.count(curr_object))
            {
                Mass *mass = dynamic_cast<Mass*>(objects[curr_object]);
                if(mass)
                    mass->mass = slider_value_from_mouse(event.motion.x, popup.slider_1, 0.1f, 100.0f);
            }
            else if(active_slider == ActiveSlider::SPRING_K && is_spring)
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                    spring->k_const = slider_value_from_mouse(event.motion.x, popup.slider_1, 1.0f, 300.0f);
            }
            else if(active_slider == ActiveSlider::SPRING_MASS && is_spring)
            {
                Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
                if(spring)
                    spring->mass = slider_value_from_mouse(event.motion.x, popup.slider_2, 0.0f, 100.0f);
            }

            return;
        }

        if((dragging || resizing) && curr_object < objects.size())
        {
            int d_y = event.motion.y - (int)y_start;
            int d_x = event.motion.x - (int)x_start;
            last_dx = d_x;
            last_dy = d_y;

            if(!playing && resizing)
            {
                resize_rect_object_handle(objects[curr_object], resize_handle, d_x, d_y, w, h);
                if(springs.count(curr_object))
                    sync_attached_objects(dynamic_cast<Spring*>(objects[curr_object]));
            }
            else if(playing && springs.count(curr_object))
            {
                Spring *curr = dynamic_cast<Spring*>(objects[curr_object]);
                if(curr)
                {
                    Orientation o = curr->orientation;
                    const size_t o_idx = orientation_index(o);
                    const size_t next_idx = next_orientation_index(o);

                    int new_y = (int)(curr->corners[o_idx][1] * h) + d_y;
                    int new_x = (int)(curr->corners[o_idx][0] * w) + d_x;

                    if(
                        (o == Orientation::DOWN && new_y > (int)(curr->corners[orientation_index(Orientation::UP)][1] * h)) ||
                        (o == Orientation::UP && new_y < (int)(curr->corners[orientation_index(Orientation::DOWN)][1] * h))
                    )
                    {
                        curr->corners[o_idx][1] = curr->corners[next_idx][1] = (float)new_y / h;
                    }
                    else if(
                        (o == Orientation::RIGHT && new_x > (int)(curr->corners[orientation_index(Orientation::LEFT)][0] * w)) ||
                        (o == Orientation::LEFT && new_x < (int)(curr->corners[orientation_index(Orientation::RIGHT)][0] * w))
                    )
                    {
                        curr->corners[o_idx][0] = curr->corners[next_idx][0] = (float)new_x / w;
                    }

                    curr->create_hitbox();
                    sync_attached_objects(curr);
                }
            }
            else
            {
                if(!playing)
                {
                    float left, top, right, bottom;
                    get_rect_bounds(objects[curr_object], left, top, right, bottom);

                    const int curr_left_px = (int)(left * w);
                    const int curr_top_px = (int)(top * h);
                    const int target_left_px = event.motion.x - drag_anchor_x;
                    const int target_top_px = event.motion.y - drag_anchor_y;

                    move_object_by_pixels(
                        objects[curr_object],
                        target_left_px - curr_left_px,
                        target_top_px - curr_top_px,
                        w,
                        h
                    );
                    if(springs.count(curr_object))
                        sync_attached_objects(dynamic_cast<Spring*>(objects[curr_object]));
                }
                else
                {
                    move_object_by_pixels(objects[curr_object], d_x, d_y, w, h);
                }
            }

            x_start = event.motion.x;
            y_start = event.motion.y;
        }
    }
}

void MainWindow::main_loop()
{
    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);

    if(playing && animating)
    {
        for(size_t idx : springs)
        {
            Spring *curr = dynamic_cast<Spring*>(objects[idx]);
            if(!curr)
                continue;

            Orientation o = curr->orientation;
            const size_t o_idx = orientation_index(o);
            const size_t next_idx = next_orientation_index(o);
            const size_t opposite_idx = opposite_orientation_index(o);
            const float prev_edge_px = (o == Orientation::LEFT || o == Orientation::RIGHT)
                ? (curr->corners[o_idx][0] * w)
                : (curr->corners[o_idx][1] * h);
            const float mass = spring_effective_mass(curr);
            float eq, d;
            if(o == Orientation::LEFT || o == Orientation::RIGHT)
            {
                eq = curr->base_shape[o_idx][0]*w;
                d = curr->corners[o_idx][0]*w - eq;
            }
            else
            {
                eq = G * mass / curr->k_const + curr->base_shape[o_idx][1]*h;
                d = curr->corners[o_idx][1]*h - eq;
            }

            float a = G - curr->k_const * d / mass;

            // Euler integration: update velocity then position
            curr->velocity += a * DELTA_T;
            float displacement = curr->velocity * DELTA_T;

            if(o == Orientation::LEFT || o == Orientation::RIGHT)
            {
                curr->corners[o_idx][0] += (float)displacement/w;
                curr->corners[next_idx][0] += (float)displacement/w;

                if(
                    (o == Orientation::LEFT && curr->corners[o_idx][0] > curr->corners[opposite_idx][0]) ||
                    (o == Orientation::RIGHT && curr->corners[o_idx][0] < curr->corners[opposite_idx][0])
                )
                {
                    curr->corners[o_idx][0] = curr->corners[next_idx][0] = curr->base_shape[opposite_idx][0];
                    curr->velocity = 0;
                }
            }
            else
            {
                curr->corners[o_idx][1] += displacement / h;
                curr->corners[next_idx][1] += displacement / h;

                if(
                    (o == Orientation::DOWN && curr->corners[o_idx][1] < curr->corners[opposite_idx][1]) ||
                    (o == Orientation::UP && curr->corners[o_idx][1] > curr->corners[opposite_idx][1])
                )
                {
                    curr->corners[o_idx][1] = curr->corners[next_idx][1] = curr->base_shape[opposite_idx][1];
                    curr->velocity = 0;
                }
            }

            objects[idx]->create_hitbox();

            float shift_x = 0.0f;
            float shift_y = 0.0f;
            if(constrain_object_to_window(curr, shift_x, shift_y))
            {
                const bool horizontal = (o == Orientation::LEFT || o == Orientation::RIGHT);
                if((horizontal && std::fabs(shift_x) > 0.000001f) || (!horizontal && std::fabs(shift_y) > 0.000001f))
                    curr->velocity = -curr->velocity;

                objects[idx]->create_hitbox();
            }

            sync_attached_objects(curr);
            resolve_spring_push_collision(curr, prev_edge_px, w, h, objects, masses, springs);
        }

        integrate_free_masses(w, h, objects, masses, springs);
    }

    for(size_t i = 0; i < objects.size(); i++)
    {
        if(!objects[i]->anchor)
        {
            
        }
        objects[i]->draw_object(get_renderer(), theme, w, h);
    }

    if(!playing && has_selection && curr_object < objects.size() && !buttons.count(curr_object))
        draw_selection_frame(get_renderer(), objects[curr_object], w, h);

    if(!playing && show_property_popup && has_selection && curr_object < objects.size() && (springs.count(curr_object) || masses.count(curr_object)))
    {
        const bool is_spring = springs.count(curr_object);
        PropertyPopupRects popup = get_property_popup_rects(objects[curr_object], w, h, is_spring);
        draw_property_popup(get_renderer(), popup, is_spring);

        SDL_Color text_color = {230, 230, 230, 255};

        if(masses.count(curr_object))
        {
            Mass *mass = dynamic_cast<Mass*>(objects[curr_object]);
            if(mass)
            {
                draw_popup_text(get_renderer(), "mass", popup.panel.x + 12.0f, popup.input_1.y + 4.0f, text_color);
                draw_slider(get_renderer(), popup.slider_1, mass->mass, 0.1f, 100.0f);
                draw_input_box(get_renderer(), popup.input_1, active_input == ActiveInput::MASS);
                draw_popup_text(
                    get_renderer(),
                    active_input == ActiveInput::MASS ? property_input : format_value(mass->mass),
                    popup.input_1.x + 6.0f,
                    popup.input_1.y + 4.0f,
                    text_color
                );
            }
        }
        else if(is_spring)
        {
            Spring *spring = dynamic_cast<Spring*>(objects[curr_object]);
            if(spring)
            {
                draw_popup_text(get_renderer(), "k", popup.panel.x + 12.0f, popup.input_1.y + 4.0f, text_color);
                draw_slider(get_renderer(), popup.slider_1, spring->k_const, 1.0f, 300.0f);
                draw_input_box(get_renderer(), popup.input_1, active_input == ActiveInput::SPRING_K);
                draw_popup_text(
                    get_renderer(),
                    active_input == ActiveInput::SPRING_K ? property_input : format_value(spring->k_const),
                    popup.input_1.x + 6.0f,
                    popup.input_1.y + 4.0f,
                    text_color
                );

                draw_popup_text(get_renderer(), "mass", popup.panel.x + 12.0f, popup.input_2.y + 4.0f, text_color);
                draw_slider(get_renderer(), popup.slider_2, spring->mass, 0.0f, 100.0f);
                draw_input_box(get_renderer(), popup.input_2, active_input == ActiveInput::SPRING_MASS);
                draw_popup_text(
                    get_renderer(),
                    active_input == ActiveInput::SPRING_MASS ? property_input : format_value(spring->mass),
                    popup.input_2.x + 6.0f,
                    popup.input_2.y + 4.0f,
                    text_color
                );

                massless_checkbox.set_position((int)popup.checkbox.x, (int)popup.checkbox.y);
                massless_checkbox.set_size((int)popup.checkbox.w);
                massless_checkbox.set_checked(spring->massless);
                massless_checkbox.draw(get_renderer());
                draw_popup_text(get_renderer(), "massless", popup.checkbox.x + popup.checkbox.w + 8.0f, popup.checkbox.y - 1.0f, text_color);

                draw_popup_text(get_renderer(), "red edge", popup.panel.x + 16.0f, popup.orientation_button.y + 4.0f, text_color);
                draw_input_box(get_renderer(), popup.orientation_button, false);
                draw_popup_text(
                    get_renderer(),
                    orientation_text(spring->orientation),
                    popup.orientation_button.x + 8.0f,
                    popup.orientation_button.y + 4.0f,
                    text_color
                );
            }
        }
    }
}
