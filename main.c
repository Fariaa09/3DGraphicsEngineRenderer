//
// Created by franc on 7/3/2025.
//

#include <SDL.h>
#include <stdio.h>

typedef  struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;

} Engine;

// Macros for error treatment
#define SDL_INIT_CHECK(msg)                                              \
    do                                                                   \
    {                                                                    \
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {                              \
        fprintf(stderr, "[ERROR] %s \n[SDL]: %s\n", msg, SDL_GetError());\
        return 0;                                                        \
        }                                                                \
    }while(0)                                                            \

#define SDL_WINDOW_CHECK(x, msg)                                  \
    do                                                            \
    {                                                             \
        if (!(x)) {                                               \
            fprintf(stderr, "[ERROR] %s \n[SDL] %s", msg, SDL_GetError());\
            SDL_Quit();                                           \
            return 0;                                             \
        }                                                         \
    }while(0)                                                     \

#define SDL_RENDERER_CHECK(window, x, msg)                        \
    do                                                            \
    {                                                             \
        if (!(x)) {                                               \
            fprintf(stderr, "[ERROR] %s \n[SDL] %s", msg, SDL_GetError());\
            SDL_DestroyWindow(window);                            \
            SDL_Quit();                                           \
            return 0;                                             \
        }                                                         \
    }while(0)                                                     \

#define WIDTH 1000
#define HEIGHT 600

/*
 *  This builds the engine, giving it a window and a renderer
 *
 */
int constructEngine(Engine* engine) {
    // Initialize SDL
    SDL_INIT_CHECK("SOMETHING WENT WRONG WHILE INITIALIZING SDL");

    // Create a window
    SDL_Window* window = SDL_CreateWindow("RENDERER",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WIDTH, HEIGHT,SDL_WINDOW_SHOWN);
    SDL_WINDOW_CHECK(window, "SOMETHING WENT WRONG WHEN CREATING THE WINDOW");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RENDERER_CHECK(window, renderer, "SOMETHING WENT WRONG WHILE CREATING THE RENDERER");

    engine->window = window;
    engine->renderer = renderer;

    return 1;
}

/*
 *  This starts the engine, entering the main loop of execution.
 *
 *  TODO: Implement the update loop of the engine
 */
void start(Engine* engine) {
    SDL_Event event;
    int running = 1;

    // Main Loop
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

    }
}

int main(int argc, char* argv[]) {
    Engine engine;

    if (constructEngine(&engine))
    {
        start(&engine);
    }

    return 0;
}
