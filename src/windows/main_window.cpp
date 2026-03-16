#include "windows/main_window.hpp"

namespace {
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
}

MainWindow::MainWindow(Theme *theme) : Window("Physics Sim", 1920, 1080) 
{ 
    this->theme = theme;
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
    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);

    if(event.type == SDL_EVENT_WINDOW_RESIZED)
    {

    }
    
    if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        running = false;

    if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
    {
        for(size_t i = 0; i < objects.size(); i++)
        {
            if(springs.count(i) && objects[i]->is_mouse_click(event.button.x, event.button.y, w, h))
            {
                dragging = true;
                y_start = event.button.y;
                x_start = event.button.x;
                curr_object = i;
                if(springs.count(curr_object))
                {
                    Spring *curr = dynamic_cast<Spring*>(objects[i]);
                    curr->velocity = 0;
                }
                break;
            }
        }
    }

    if(event.type == SDL_EVENT_MOUSE_BUTTON_UP) 
    {
        dragging = false;
        animating = true;
    }

    if(event.type == SDL_EVENT_MOUSE_MOTION) 
    {
        if(dragging && curr_object >= 0 && curr_object < objects.size())
        {
            animating = false;

            int d_y = event.button.y - y_start;
            int d_x = event.button.x - x_start;
            Orientation o = objects[curr_object]->orientation;
            const size_t o_idx = orientation_index(o);
            const size_t next_idx = next_orientation_index(o);
            int new_y = objects[curr_object]->base_shape[o_idx][1]*h + d_y;
            int new_x = objects[curr_object]->base_shape[o_idx][0]*w + d_x;

            if(
                (o == Orientation::DOWN && new_y > objects[curr_object]->corners[orientation_index(Orientation::UP)][1]*h) || 
                (o == Orientation::UP && new_y < objects[curr_object]->corners[orientation_index(Orientation::DOWN)][1]*h)
            )
            {
                objects[curr_object]->corners[o_idx][1] = objects[curr_object]->corners[next_idx][1] = (float)new_y/h;
            }
            else if(
                (o == Orientation::RIGHT && new_x > objects[curr_object]->corners[orientation_index(Orientation::LEFT)][0]*w) ||
                (o == Orientation::LEFT && new_x < objects[curr_object]->corners[orientation_index(Orientation::RIGHT)][0]*w)
            )
            {
                objects[curr_object]->corners[o_idx][0] = objects[curr_object]->corners[next_idx][0] = (float)new_x/w;
            }
            
            objects[curr_object]->create_hitbox();
        }
    }
}

void MainWindow::main_loop()
{
    int w, h;
    SDL_GetWindowSize(get_window(), &w, &h);
    
    for(size_t i = 0; i < objects.size(); i++)
    {
        if(springs.count(i) && animating)
        {
            Spring *curr = dynamic_cast<Spring*>(objects[i]);
            Orientation o = curr->orientation;
            const size_t o_idx = orientation_index(o);
            const size_t next_idx = next_orientation_index(o);
            const size_t opposite_idx = opposite_orientation_index(o);
            float mass = 0.1;
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

            objects[i]->create_hitbox();
        }
        objects[i]->draw_object(get_renderer(), theme, w, h);
    }
}
