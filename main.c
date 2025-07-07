//
// Created by franc on 7/3/2025.
//

#include <SDL.h>
#include <stdio.h>
#include <math.h>

typedef struct
{
    float x, y, z;
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

// Macro to convert from degree to radians
#define TO_RAD(x) (x / 180.0f * M_PI)

#define WIDTH 800
#define HEIGHT 800

// Projection Matrix Values
#define Z_NEAR 0.1f
#define Z_FAR 1000.0f
#define Q (Z_FAR / (Z_FAR - Z_NEAR))
#define FOV 90.0f
#define ASPECT_RATIO (WIDTH / HEIGHT)
#define FOV_TAN (1.0f / tanf(TO_RAD(FOV * 0.5f)))

Engine* engine;
Vector camera = {0.0f, 0.0f, 0.0f};

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
    if (engine->meshes == NULL)
    {
        perror("[ERROR] ALLOCATING MEMORY FOR THE ENGINE MESHES FAILED!");
    }

    return 1;
}

/*
 *  This function multiplies a 3x3 vector i by a 4x4 matrix m and outputs the
 *  result in a Vector o. This is used to calculate the projection of a Vector
 *  following the projection matrix that is defined
 *
 */
void multMatVec(const Vector* i, Vector* o, const Matrix4x4* m)
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
 * Constructs a 4x4 translation Matrix from a given translation Vector
 * Its rough but gets the job done
 *
 */
void translate(const Vector* i, Vector* o, const Vector* v)
{
    const Matrix4x4 aux = {
        .mat = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {v->x, v->y, v->z, 1}
        }
    };
    multMatVec(i, o, &aux);
}

/*
 * Dot product of 2 vectors
 *
 */
float dotProduct(const Vector* a, const Vector* b)
{
    return (a->x * b->x + a->y * b->y + a->z * b->z);
}

/*
 * Cross Productt of 2 Vectors
 *
 */
Vector crossProduct(const Vector* a, const Vector* b)
{
    Vector n;
    n.x = a->y * b->z - a->z * b->y;
    n.y = a->z * b->x - a->x * b->z;
    n.z = a->x * b->y - a->y * b->x;
    return n;
}

void drawTriangle(const Triangle* t)
{
    // Draw the projection points
    SDL_RenderDrawPoint(engine->renderer, t->points[0].x, t->points[0].y);
    SDL_RenderDrawPoint(engine->renderer, t->points[1].x, t->points[1].y);
    SDL_RenderDrawPoint(engine->renderer, t->points[2].x, t->points[2].y);
    // Draw the triangle lines
    SDL_RenderDrawLine(engine->renderer, t->points[0].x, t->points[0].y,
                       t->points[1].x, t->points[1].y);
    SDL_RenderDrawLine(engine->renderer, t->points[1].x, t->points[1].y,
                       t->points[2].x, t->points[2].y);
    SDL_RenderDrawLine(engine->renderer, t->points[2].x, t->points[2].y,
                       t->points[0].x, t->points[0].y);
}

/*
 * Properly scales a Vector to the screen
 *
 */
void scale(Vector* v)
{
    v->x += 1.0f;
    v->y += 1.0f;
    v->x *= 0.5f * WIDTH;
    v->y *= 0.5f * HEIGHT;
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
                    multMatVec(&translated.points[k], &projection.points[k], &proj_mat);
                    scale(&projection.points[k]);
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
                // Normalize normal
                const float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                normal.x /= l;
                normal.y /= l;
                normal.z /= l;

                // Calculate the vector from the camera to one point of the triangle
                Vector t_camera = {
                    translated.points[1].x - camera.x,
                    translated.points[1].y - camera.y,
                    translated.points[1].z - camera.z
                };

                // Culling - Can be much better
                if (dotProduct(&normal, &t_camera) < 0.0f)
                    drawTriangle(&projection);

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

int main(int argc, char* argv[])
{
    engine = malloc(sizeof(Engine));
    if (engine == NULL)
    {
        perror("[ERROR] ALLOCATING MEMORY FOR ENGINE FAILED!");
        exit(EXIT_FAILURE);
    }
    const Triangle tris[12] = {
        // Back (z=0, normal deve apontar para -Z)
        {{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}}},
        {{{0, 0, 0}, {1, 1, 0}, {1, 0, 0}}},
        // Down (y=0, normal deve apontar para -Y)
        {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}}},
        {{{0, 0, 0}, {1, 0, 1}, {0, 0, 1}}},
        // Right (x=1, normal deve apontar para +X)
        {{{1, 0, 0}, {1, 1, 0}, {1, 1, 1}}},
        {{{1, 0, 0}, {1, 1, 1}, {1, 0, 1}}},
        // Left (x=0, normal deve apontar para -X)
        {{{0, 0, 0}, {0, 0, 1}, {0, 1, 1}}},
        {{{0, 0, 0}, {0, 1, 1}, {0, 1, 0}}},
        // Top (y=1, normal deve apontar para +Y)
        {{{0, 1, 0}, {0, 1, 1}, {1, 1, 1}}},
        {{{0, 1, 0}, {1, 1, 1}, {1, 1, 0}}},
        // Front (z=1, normal deve apontar para +Z)
        {{{0, 0, 1}, {1, 0, 1}, {1, 1, 1}}},
        {{{0, 0, 1}, {1, 1, 1}, {0, 1, 1}}}
    };

    Mesh cubeMesh;
    cubeMesh.tris = malloc(12 * sizeof(Triangle));
    if (cubeMesh.tris == NULL)
    {
        perror("[ERROR] ALLOCATING MEMORY FOR THE MESH TRIANGLES FAILED!");
        exit(EXIT_FAILURE);
    }

    cubeMesh.nTris = 12;
    // Copy the triangle array to mesh
    memcpy(cubeMesh.tris, tris, 12 * sizeof(Triangle));

    if (constructEngine(engine))
    {
        memcpy(engine->meshes, &cubeMesh, sizeof(cubeMesh));
        engine->nMeshes = 1;
        start(engine);
    }

    free(engine);
    return 0;
}
