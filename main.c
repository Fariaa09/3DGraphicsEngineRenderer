//
// Created by franc on 7/3/2025.
//

#include <SDL.h>
#include <stdio.h>
#include <math.h>

typedef struct
{
    int x, y, z;
} Vector;

typedef struct
{
    Vector points[3];
} Triangle;

typedef struct
{
    // Dynamically allocated for ease of expansion
    int nTris;
    Triangle* tris;
} Mesh;

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    int nMeshes;
    // Dynamically allocated for ease of expansion
    Mesh* meshes;
} Engine;

typedef struct
{
    // Hardcoded the size because it should not change
    float mat[4][4];
} Matrix4x4;

// Macros for error treatment
#define CHECK_INITIALIZATION(msg)                                         \
    do                                                                    \
    {                                                                     \
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {                               \
        fprintf(stderr, "[ERROR] %s! \n[SDL]: %s\n", msg, SDL_GetError());\
        return 0;                                                         \
        }                                                                 \
    }while(0)

#define CHECK_WINDOW_CREATION(x, msg)                             \
    do                                                            \
    {                                                             \
        if (!(x)) {                                               \
            fprintf(stderr, "[ERROR] %s! \n[SDL] %s", msg, SDL_GetError());\
            SDL_Quit();                                           \
            return 0;                                             \
        }                                                         \
    }while(0)

#define CHECK_RENDERER_CREATION(window, x, msg)                   \
    do                                                            \
    {                                                             \
        if (!(x)) {                                               \
            fprintf(stderr, "[ERROR] %s! \n[SDL] %s", msg, SDL_GetError());\
            SDL_DestroyWindow(window);                            \
            SDL_Quit();                                           \
            return 0;                                             \
        }                                                         \
    }while(0)

#define CHECK_ALLOCATION(x)                                       \
    do                                                            \
    {                                                             \
        if (!(x)) {                                               \
            fprintf(stderr, "[ERROR] ALLOCATING MEMORY FAILED! ");\
        return 0;                                                 \
        }                                                         \
    }while(0)


#define WIDTH 900
#define HEIGHT 600

// Projection Matrix Values
#define Z_NEAR 0.1
#define Z_FAR 1000.0
#define Q Z_FAR / (Z_FAR - Z_NEAR)
#define FOV 90.0
#define ASPECT_RATIO WIDTH / HEIGHT
// Conversion to rad on the spot (crazy)
#define FOV_TAN 1.0 / tan(FOV * 0.5 / 180.0 * 3.1415159)


/*
 *  This builds the engine, giving it a window and a renderer
 *
 */
int constructEngine(Engine* engine)
{
    // Initialize SDL
    CHECK_INITIALIZATION("SOMETHING WENT WRONG WHILE INITIALIZING SDL");
    // Create a window
    SDL_Window* window = SDL_CreateWindow("RENDERER",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    CHECK_WINDOW_CREATION(window, "SOMETHING WENT WRONG WHEN CREATING THE WINDOW");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    CHECK_RENDERER_CREATION(window, renderer, "SOMETHING WENT WRONG WHILE CREATING THE RENDERER");

    engine->window = window;
    engine->renderer = renderer;
    // Set Background color to white
    SDL_SetRenderDrawColor(engine->renderer, 255, 255, 255, 255);

    // Allocate memory for a single mesh for now
    engine->meshes = malloc(sizeof(Mesh));
    CHECK_ALLOCATION(engine->meshes);

    return 1;
}

/*
 *  This function multtiplies a 3x3 vector i by a 4x4 matrix m and outputs the
 *  result in a Vector o. This is used to calculate the projection of a Vector
 *  following the projection matrix that is defined
 *
 */
void multiplyMatrixVector(const Vector* i, Vector* o, const Matrix4x4* m)
{
    // calculate the values
    o->x = i->x * m->mat[0][0] + i->y * m->mat[1][0] + i->z * m->mat[2][0] + m->mat[3][0];
    o->y = i->x * m->mat[0][1] + i->y * m->mat[1][1] + i->z * m->mat[2][1] + m->mat[3][1];
    o->z = i->x * m->mat[0][2] + i->y * m->mat[1][2] + i->z * m->mat[2][2] + m->mat[3][2];
    // Calculate the fourth value
    const float w = i->x * m->mat[0][3] + i->y * m->mat[1][3] + i->z * m->mat[2][3] + m->mat[3][3];

    // Divide to get the correct values on x, y, z
    if (w != 0.0f)
    {
        o->x /= w;
        o->y /= w;
        o->z /= w;
    }
}

/*
 *  This starts the engine, entering the main loop of execution.
 *
 */
void start(const Engine* engine)
{
    SDL_Event event;
    int running = 1;

    // Defining the projection matrix
    Matrix4x4 proj_mat = {
        {
            {ASPECT_RATIO * FOV_TAN, 0.0f, 0.0f, 0.0f},
            {0.0f, FOV_TAN, 0.0f, 0.0f},
            {0.0f, 0.0f, Q, 1.0f},
            {0.0f, 0.0f, - Z_NEAR * Q, 0.0f}
        }
    };
    // Main Loop
    while (running)
    {
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT)
                running = 0;

        // Renderer settings to draw white on black
        SDL_SetRenderDrawColor(engine->renderer, 0, 0, 0, 255);
        SDL_RenderClear(engine->renderer);
        SDL_SetRenderDrawColor(engine->renderer, 255, 255, 255, 255);

        // Render each mesh
        for (int i = 0; i < engine->nMeshes; i++)
        {
            const Mesh mesh = engine->meshes[i];
            // Render each triangle
            for (int j = 0; j < mesh.nTris; j++)
            {
                const Triangle t = mesh.tris[j];

                // TODO: Make functions to make this more readable
                Triangle projection, translated = t;
                // Make things further away
                translated.points[0].z = t.points[0].z + 3.0f;
                translated.points[1].z = t.points[1].z + 3.0f;
                translated.points[2].z = t.points[2].z + 3.0f;
                // Calculate projection
                multiplyMatrixVector(&translated.points[0], &projection.points[0], &proj_mat);
                multiplyMatrixVector(&translated.points[1], &projection.points[1], &proj_mat);
                multiplyMatrixVector(&translated.points[2], &projection.points[2], &proj_mat);
                // Scale to view
                projection.points[0].x += 1.0f; projection.points[0].y += 1.0f;
                projection.points[1].x += 1.0f; projection.points[1].y += 1.0f;
                projection.points[2].x += 1.0f; projection.points[2].y += 1.0f;
                // Scale to the screen
                projection.points[0].x *= 0.5f * WIDTH; projection.points[0].y *= 0.5f * HEIGHT;
                projection.points[1].x *= 0.5f * WIDTH; projection.points[1].y *= 0.5f * HEIGHT;
                projection.points[2].x *= 0.5f * WIDTH; projection.points[2].y *= 0.5f * HEIGHT;
                // Draw the projection points
                SDL_RenderDrawPoint(engine->renderer, projection.points[0].x, projection.points[0].y);
                SDL_RenderDrawPoint(engine->renderer, projection.points[1].x, projection.points[1].y);
                SDL_RenderDrawPoint(engine->renderer, projection.points[2].x, projection.points[2].y);
                // Draw the triangle lines
                SDL_RenderDrawLine(engine->renderer, projection.points[0].x, projection.points[0].y,
                                                     projection.points[1].x, projection.points[1].y);
                SDL_RenderDrawLine(engine->renderer, projection.points[1].x, projection.points[1].y,
                                                     projection.points[2].x, projection.points[2].y);
                SDL_RenderDrawLine(engine->renderer, projection.points[2].x, projection.points[2].y,
                                                     projection.points[0].x, projection.points[0].y);
            }
        }
        // Present the drawing in the screen
        SDL_RenderPresent(engine->renderer);
    }

    // Free Meshes array of triangles
    for (int i = 0; i < engine->nMeshes; i++)
        if (engine->meshes[i].tris != NULL)
            free(engine->meshes[i].tris);

    // Free the array of Meshes
    free(engine->meshes);
}

int main(int argc, char* argv[])
{
    Engine engine;

    Triangle tris[12] = {
        {{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}}},
        {{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}}},
        {{{0, 0, 0}, {1, 0, 1}, {1, 0, 0}}},
        {{{0, 0, 0}, {0, 0, 1}, {1, 0, 1}}},
        {{{1, 0, 0}, {1, 0, 1}, {1, 1, 0}}},
        {{{1, 0, 1}, {1, 1, 0}, {1, 1, 1}}},
        {{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}}},
        {{{0, 0, 0}, {0, 0, 1}, {0, 1, 1}}},
        {{{0, 1, 0}, {0, 1, 1}, {1, 1, 1}}},
        {{{0, 1, 0}, {1, 1, 0}, {1, 1, 1}}},
        {{{0, 0, 1}, {0, 1, 1}, {1, 1, 1}}},
        {{{0, 0, 1}, {1, 1, 1}, {1, 0, 1}}},
    };

    Mesh cubeMesh;
    cubeMesh.tris = malloc(sizeof(Triangle) * 12);
    CHECK_ALLOCATION(cubeMesh.tris);

    cubeMesh.nTris = 12;
    // Copy the triangle array to mesh
    memcpy(cubeMesh.tris, tris, sizeof(Triangle) * 12);

    if (constructEngine(&engine))
    {
        memcpy(engine.meshes, &cubeMesh, sizeof(cubeMesh));
        engine.nMeshes = 1;
        start(&engine);
    }

    return 0;
}
