//
// Created by franc on 7/3/2025.
//

#include <SDL.h>
#include <stdio.h>

#include "engine.h"

Vector camera = {0.0f, 0.0f, 0.0f};

/**
 * Construct the engine (initialize the window, renderer, meshes, ...)
 *
 * @param engine Engine to be initialized
 *
 * @return status
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
    ALLOCATE(engine->meshes, sizeof(Mesh));

    return 1;
}

/**
 * Defines the necessary things for the engine to run and enters
 * the main loop
 *
 *  @param engine Engine that is going to be started
 *
 *  @return void
 */
void start(const Engine* engine)
{
    SDL_Event event;
    int running = 1;

    // Defining the projection matrix
    const Matrix4x4 proj_mat =
    {
        {
            {ASPECT_RATIO * FOV_TAN, 0.0f, 0.0f, 0.0f},
            {0.0f, FOV_TAN, 0.0f, 0.0f},
            {0.0f, 0.0f, Q, 1.0f},
            {0.0f, 0.0f, -Z_NEAR * Q, 0.0f}
        }
    };

    float theta = 0.0f;
    // Main Loop
    while (running)
    {
        // TODO: Make this Matrices better and use less space
        Matrix4x4 rot_mat_x =
        {
            {
                {1.0f, 0.0f, 0.0f, 0.0f},
                {0.0f, cosf(TO_RAD(theta)), -sinf(TO_RAD(theta)), 0.0f},
                {0.0f, sinf(TO_RAD(theta)), cosf(TO_RAD(theta)), 0.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}
            }
        };

        Matrix4x4 rot_mat_y =
        {
            {
                {cosf(TO_RAD(theta)), 0.0f, sinf(TO_RAD(theta)), 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {-sinf(TO_RAD(theta)), 0.0f, cosf(TO_RAD(theta)), 0.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}
            }
        };

        Matrix4x4 rot_mat_z =
        {
            {
                {cosf(TO_RAD(theta)), -sinf(TO_RAD(theta)), 0.0f, 0.0f},
                {sinf(TO_RAD(theta)), cosf(TO_RAD(theta)), 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f, 0.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}
            }
        };

        Vector translate_vec = {0.0f, 0.0f, 3.0f};
        // Check to close the window
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT)
                running = 0;

        // Renderer settings to draw white on black
        SDL_SetRenderDrawColor(engine->renderer, 0, 0, 0, 255);
        SDL_RenderClear(engine->renderer);
        SDL_SetRenderDrawColor(engine->renderer, 255, 255, 255, 255);

        for (int i = 0; i < engine->nMeshes; i++)
        {
            const Mesh mesh = engine->meshes[i];
            for (int j = 0; j < mesh.nTris; j++)
            {
                Triangle translated, rotated_x, rotated_z, projection;
                for (int k = 0; k < 3; k++)
                {
                    // Rotate -> Translate -> Project -> Scale
                    multMatVec(&mesh.tris[j].points[k], &rotated_x.points[k], &rot_mat_x);
                    multMatVec(&rotated_x.points[k], &rotated_z.points[k], &rot_mat_z);
                    translate(&rotated_z.points[k], &translated.points[k], &translate_vec);
                }
                // Calculate 2 lines of the triangle (l1, l2) and get the normal through crossProduct
                Vector l1 = {
                    translated.points[1].x - translated.points[0].x,
                    translated.points[1].y - translated.points[0].y,
                    translated.points[1].z - translated.points[0].z
                };

                Vector l2 = {
                    translated.points[2].x - translated.points[0].x,
                    translated.points[2].y - translated.points[0].y,
                    translated.points[2].z - translated.points[0].z
                };

                Vector normal = crossProduct(&l1, &l2);
                normalizeVector(&normal);
                // Calculate the vector from the camera to one point of the triangle
                Vector t_camera = {
                    translated.points[1].x - camera.x,
                    translated.points[1].y - camera.y,
                    translated.points[1].z - camera.z
                };
                // Culling - Can be much better
                if (dotProduct(&normal, &t_camera) < 0.0f) {
                    // Create a normalized light source
                    Vector light_source = {0.0f, 0.0f, -1.0f};
                    normalizeVector(&light_source);
                    for (int k = 0; k < 3; k++) {
                        // Process Projection and Scaling only for faces we see
                        multMatVec(&translated.points[k], &projection.points[k], &proj_mat);
                        scale(&projection.points[k]);
                    }
                    // See the alignment between the light source and the normal of the triangle
                    projection.light = dotProduct(&normal, &light_source);
                    fillTriangle(&projection, engine->renderer);
                }
            }
            // Present the drawing in the screen
            SDL_RenderPresent(engine->renderer);
            theta += 0.1f;
        }
    }
    // Free Meshes array of triangles
    for (int i = 0; i < engine->nMeshes; i++)
        if (engine->meshes[i].tris != NULL)
            free(engine->meshes[i].tris);

    // Free the array of Meshes
    free(engine->meshes);
}

/**
 * MAIN
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
    // Allocate memory for our engine
    Engine* engine;
    ALLOCATE(engine, sizeof(Engine));

    // Coded in place for now until we can actually import some models
    const Triangle tris[12] = {
        // Back
        {{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}}},
        {{{0, 0, 0}, {1, 1, 0}, {1, 0, 0}}},
        // Down
        {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}}},
        {{{0, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
        // Right
        {{{1, 0, 0}, {1, 1, 0}, {1, 1, 1}}},
        {{{1, 0, 0}, {1, 1, 1}, {1, 0, 1}}},
        // Left
        {{{0, 0, 0}, {0, 0, 1}, {0, 1, 1}}},
        {{{0, 0, 0}, {0, 1, 1}, {0, 1, 0}}},
        // Top
        {{{0, 1, 0}, {0, 1, 1}, {1, 1, 1}}},
        {{{0, 1, 0}, {1, 1, 1}, {1, 1, 0}}},
        // Front
        {{{0, 0, 1}, {1, 0, 1}, {1, 1, 1}}},
        {{{0, 0, 1}, {1, 1, 1}, {0, 1, 1}}}
    };

    // Allocate space for our meshes triangles
    Mesh cubeMesh;
    cubeMesh.nTris = 12;
    const int size_to_copy = sizeof(tris) * cubeMesh.nTris;
    ALLOCATE(cubeMesh.tris, size_to_copy);
    memcpy(cubeMesh.tris, tris, size_to_copy);

    if (constructEngine(engine))
    {
        memcpy(engine->meshes, &cubeMesh, sizeof(cubeMesh));
        engine->nMeshes = 1;
        start(engine);
    }

    // Free things
    free(engine);
    return 0;
}
