// Standard C/C++ includes
#include <iostream>
#include <unordered_set>

// SDL includes
#include <SDL3/SDL.h>

// User includes
#include "engine.hpp"
#include "spring.hpp"
#include "mass.hpp"
#include "button.hpp"
#include "themes.hpp"

#define SDL_INIT (SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)
#define HEIGHT 720
#define WIDTH 1080
#define G 9.81
#define DELTA_T 0.016

size_t create_spring(const std::vector<std::array<float,2>> &corners, float k_const, bool massless, float mass, std::vector<Object*> &objects, Orientation orientation);
size_t create_mass(const std::vector<std::array<float,2>> &corners, float mass, std::vector<Object*> &objects);
size_t create_button(int x, int y, int w, int h, std::vector<Object*> &objects, std::function<void()> on_press);

int main(int argc, char const *argv[])
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    if(!SDL_Init(SDL_INIT))
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    window = SDL_CreateWindow("Thing", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

    if(!window)
    {
        std::cout << "Error creating window: " << SDL_GetError() << "\n";
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if(!renderer)
    {
        std::cout << "Error creating renderer: " << SDL_GetError() << "\n";
        return 1;
    }

    // Enable vsync for smooth animation
    SDL_SetRenderVSync(renderer, 1);

    float width = 30;
    float height = 30;

    std::vector<std::array<float,2>> spring_shape = {
        {(float)(WIDTH/2), (float)(HEIGHT/2)},
        {(float)(WIDTH/2+width), (float)(HEIGHT/2)},
        {(float)(WIDTH/2+width), (float)(HEIGHT/2+height)},
        {(float)(WIDTH/2), (float)(HEIGHT/2+height)}
    },
    mass_shape = {
        {5, 5},
        {5 + width, 5},
        {5 + width, 5 + height},
        {5, 5 + height}
    };
    
    std::vector<Object*> objects;
    std::unordered_set<size_t> springs;
    std::unordered_set<size_t> masses;

    springs.insert(create_spring(spring_shape, 25, false, 0, objects, UP));
    masses.insert(create_mass(mass_shape, 1, objects));

    SDL_Event event;
    bool running = true;
    bool dragging = false;
    bool animating = true;
    unsigned int y_start, x_start;
    size_t curr_object;
    Theme theme = Light();
    theme = Dark();

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                running = false;

            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) 
            {
                for(size_t i = 0; i < objects.size(); i++)
                {
                    if(springs.count(i) && objects[i]->is_mouse_click(event.button.x, event.button.y))
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
                    int new_y = objects[curr_object]->base_shape[o][1] + d_y;
                    int new_x = objects[curr_object]->base_shape[o][0] + d_x;

                    if(
                        (o == DOWN && new_y > objects[curr_object]->corners[UP][1]) || 
                        (o == UP && new_y < objects[curr_object]->corners[DOWN][1])
                    )
                    {
                        objects[curr_object]->corners[o][1] = objects[curr_object]->corners[(o + 1) % 4][1] = new_y;
                    }
                    else if(
                        (o == RIGHT && new_x > objects[curr_object]->corners[LEFT][0]) ||
                        (o == LEFT && new_x < objects[curr_object]->corners[RIGHT][0])
                    )
                    {
                        objects[curr_object]->corners[o][0] = objects[curr_object]->corners[(o + 1) % 4][0] = new_x;
                    }
                    
                    objects[curr_object]->create_hitbox();
                }
            }
        }
        Color *bg = &theme.background;
        SDL_SetRenderDrawColor(renderer, bg->r, bg->g, bg->g, bg->a);
        SDL_RenderClear(renderer);
        for(size_t i = 0; i < objects.size(); i++)
        {
            if(springs.count(i) && animating)
            {
                Spring *curr = dynamic_cast<Spring*>(objects[i]);
                Orientation o = curr->orientation;
                float mass = 0.1;
                float eq, d;
                if(o == LEFT || o == RIGHT)
                {
                    eq = curr->base_shape[o][0];
                    d = curr->corners[o][0] - eq;
                }
                else
                {
                    eq = G * mass / curr->k_const + curr->base_shape[o][1];
                    d = curr->corners[o][1] - eq;
                }

                float a = G - curr->k_const * d / mass;

                // Euler integration: update velocity then position
                curr->velocity += a * DELTA_T;
                float displacement = curr->velocity * DELTA_T;

                if(o == LEFT || o == RIGHT)
                {
                    curr->corners[o][0] += displacement;
                    curr->corners[(o + 1) % 4][0] += displacement;

                    if(
                        (o == LEFT && curr->corners[o][0] > curr->corners[(o + 2) % 4][0]) ||
                        (o == RIGHT && curr->corners[o][0] < curr->corners[(o + 2) % 4][0])
                    )
                    {
                        curr->corners[o][0] = curr->corners[(o + 1) % 4][0] = curr->base_shape[(o + 2) % 4][0];
                        curr->velocity = 0;
                    }
                }
                else
                {
                    curr->corners[o][1] += displacement;
                    curr->corners[(o + 1) % 4][1] += displacement;

                    if(
                        (o == DOWN && curr->corners[o][1] < curr->corners[(o + 2) % 4][1]) ||
                        (o == UP && curr->corners[o][1] > curr->corners[(o + 2) % 4][1])
                    )
                    {
                        curr->corners[o][1] = curr->corners[(o + 1) % 4][1] = curr->base_shape[(o + 2) % 4][1];
                        curr->velocity = 0;
                    }
                }

                objects[i]->create_hitbox();
            }
            objects[i]->draw_object(renderer, &theme);
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "Quitting...\n";

    return 0;
}

size_t create_button(int x, int y, int w, int h, std::vector<Object*> &objects, std::function<void()> on_press)
{
    objects.push_back(new Button(x, y, w, h, on_press));
    return objects.size()-1;
}

size_t create_mass(const std::vector<std::array<float,2>> &corners, float mass, std::vector<Object*> &objects)
{
    objects.push_back(new Mass(corners, RECTANGLE, mass));
    return objects.size()-1;
}

size_t create_spring(const std::vector<std::array<float,2>> &corners, float k_const, bool massless, float mass, std::vector<Object*> &objects, Orientation orientation)
{
    objects.push_back(new Spring(corners, k_const, massless, mass, orientation));
    return objects.size() - 1;
}
