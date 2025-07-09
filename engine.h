//
// Created by franc on 7/9/2025.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>

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

// Macros for error treatment
#define CHECK_INITIALIZATION(msg)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {                                                                            \
            fprintf(stderr, "[ERROR] %s! \n[SDL]: %s\n", msg, SDL_GetError());                                         \
            return 0;                                                                                                  \
        }                                                                                                              \
    }while(0)

#define CHECK_WINDOW_CREATION(x, msg)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(x)) {                                                                                                    \
            fprintf(stderr, "[ERROR] %s! \n[SDL] %s", msg, SDL_GetError());                                            \
            SDL_Quit();                                                                                                \
            return 0;                                                                                                  \
        }                                                                                                              \
    }while(0)

#define CHECK_RENDERER_CREATION(window, x, msg)                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(x)) {                                                                                                    \
            fprintf(stderr, "[ERROR] %s! \n[SDL] %s", msg, SDL_GetError());                                            \
            SDL_DestroyWindow(window);                                                                                 \
            SDL_Quit();                                                                                                \
            return 0;                                                                                                  \
        }                                                                                                              \
    }while(0)

#define ALLOCATE(x, size)                                                                                              \
do                                                                                                                     \
{                                                                                                                      \
    x = malloc(size);                                                                                                  \
    if (x == NULL)                                                                                                     \
        perror("[ERROR] ALLOCATING MEMORY FAILED!");                                                                   \
} while (x == NULL);                                                                                                   \



typedef struct
{
    // TODO: Update this to use a fourth variable "w"
    float x, y, z;
} Vector;

typedef struct
{
    Vector points[3];
    float light;
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

/*Function prototypes*/
// Vector operations
void multMatVec(const Vector* i, Vector* o, const Matrix4x4* m);
void translate(const Vector* i, Vector* o, const Vector* v);
float dotProduct(const Vector* a, const Vector* b);
Vector crossProduct(const Vector* a, const Vector* b);
void normalizeVector(Vector* v);
void scale(Vector* v);
// Draw and fill function -> TODO: Update this to more generic functions
void drawTriangle(const Triangle* t, SDL_Renderer* renderer);
void fillTriangle(const Triangle* t, SDL_Renderer* renderer);

#endif //ENGINE_H
