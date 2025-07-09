//
// Created by franc on 7/9/2025.
//

#include "engine.h"
#include <math.h>

/**
 *  This function multiplies a 3x3 vector i by a 4x4 matrix m and outputs the
 *  result in a Vector o. This is used to calculate the projection of a Vector
 *  following the projection matrix that is defined
 *
 *  @param i Pointer to the Vector to perform the multiplication on
 *  @param o Pointer to the Vector to store the output
 *  @param m Matrix to multiply _i_ by
 *
 *  @return void
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

/**
 * Build the necessary 4x4 Matrix to **translate** a vector i by a
 * vector v, storing the result in vector o
 *
 *  @param i Pointer to the Vector to perform the multiplication on
 *  @param o Pointer to the Vector to store the output
 *  @param v Pointer to the translation Vector
 *
 *  @return void
 */
void translate(const Vector* i, Vector* o, const Vector* v)
{
    // Create a 4x4 matrix that translates a vector i by v
    const Matrix4x4 aux = {
        .mat = {
            {   1,    0,    0, 0},
            {   0,    1,    0, 0},
            {   0,    0,    1, 0},
            {v->x, v->y, v->z, 1}
        }
    };

    multMatVec(i, o, &aux);
}

/**
 * Dot product of 2 vectors
 *
 *  @param a First Vector
 *  @param b Second Vector
 *
 *  @return Degree of similarity between the two vectors
 */
float dotProduct(const Vector* a, const Vector* b)
{
    return (a->x * b->x + a->y * b->y + a->z * b->z);
}

/**
 * Cross Product of 2 Vectors
 *
 *  @param a First Vector
 *  @param b Second Vector
 *
 *  @return Vector perpendicular to the plane formed by a and b
 *  (normal vector)
 */
Vector crossProduct(const Vector* a, const Vector* b)
{
    Vector n;
    n.x = a->y * b->z - a->z * b->y;
    n.y = a->z * b->x - a->x * b->z;
    n.z = a->x * b->y - a->y * b->x;
    return n;
}

/**
* Normalize a vector to be between -1 and 1
*
* @param v Vector to be normalized
*
* @return void
*/
void normalizeVector(Vector* v)
{
    const float m = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= m; v->y /= m; v->z /= m;
}

/**
 * Draws a triangle. Useful for wireframe view.
 *
 * @param t Triangle to draw
 * @param renderer Renderer to use to draw the triangle
 *
 * @return void
 */
void drawTriangle(const Triangle* t, SDL_Renderer* renderer)
{
    // Draw the points
    SDL_RenderDrawPoint(renderer, t->points[0].x, t->points[0].y);
    SDL_RenderDrawPoint(renderer, t->points[1].x, t->points[1].y);
    SDL_RenderDrawPoint(renderer, t->points[2].x, t->points[2].y);
    // Draw the lines
    SDL_RenderDrawLine(renderer, t->points[0].x, t->points[0].y, t->points[1].x, t->points[1].y);
    SDL_RenderDrawLine(renderer, t->points[1].x, t->points[1].y, t->points[2].x, t->points[2].y);
    SDL_RenderDrawLine(renderer, t->points[2].x, t->points[2].y, t->points[0].x, t->points[0].y);
}

/**
 * Uses RenderGeometry to take a triangle and fill it in
 *
 * @param t Triangle to be filled
 * @param renderer Renderer to use to fill the triangle
 */
void fillTriangle(const Triangle* t, SDL_Renderer* renderer)
{
    // Define triangle vertices
    SDL_Vertex vertices[3];
    for (int i = 0; i  < 3; i++) {
        // Position
        vertices[i].position.x = t->points[i].x;
        vertices[i].position.y = t->points[i].y;
        // RGB Values - Darkened by the quantity of light from the triangle
        vertices[i].color.r = 255 * t->light;
        vertices[i].color.g = 255 * t->light;
        vertices[i].color.b = 255 * t->light;
        vertices[i].color.a = 255;
    }
    // Render geometric Shape
    SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);
}

/**
 * Scales a Vector to the screen
 *
 *  @param v Vector to scale
 *
 *  @return void
 */
void scale(Vector* v)
{
    v->x += 1.0f;
    v->y += 1.0f;
    v->x *= 0.5f * WIDTH;
    v->y *= 0.5f * HEIGHT;
}
