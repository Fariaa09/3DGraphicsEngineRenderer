//
// Created by franc on 7/9/2025.
//

#include "engine.h"

#include <math.h>
#include <stdio.h>


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
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
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
    v->x /= m;
    v->y /= m;
    v->z /= m;
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // Define triangle vertices
    SDL_Vertex vertices[3];
    for (int i = 0; i < 3; i++)
    {
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

/**
 *  Load an object from a .obj file
 *
 * @param file name of the file to load the object from
 * @param e engine that loads the object
 *
 * @return 0 for success, 1 for failure
 */
int loadFromFile(const char* file, Engine* e)
{
    e->nMeshes = 1;

    printf("[DEBUG] FILE: %s\n", file);
    FILE* fp = fopen(file, "r");
    if (fp == NULL)
    {
        perror("[ERROR] COULDN'T OPEN THE FILE!");
        return 0;
    }
    // Read the file
    char line[128];
    // Initial size of
    int v_count = 0;
    int f_count = 0;
    // Get number of vertices anf faces
    while (fgets(line, sizeof(line), fp))
        if (line[0] == 'v' && line[1] == ' ')
            v_count++;
        else if (line[0] == 'f' && line[1] == ' ')
            f_count++;

    printf("[DEBUG] NUMBER OF VERTICES FOUND: %d\n", v_count);
    printf("[DEBUG] NUMBER OF FACES FOUND: %d\n", f_count);

    fclose(fp);
    fopen(file, "r");

    Vector vertices[v_count];
    Mesh mesh; mesh.nTris = f_count;
    ALLOCATE(mesh.tris, f_count * sizeof(Triangle));
    int v_pos = 0, f_pos = 0;

    while (fgets(line, sizeof(line), fp))
    {
        // Read a Vertex Line
        if (line[0] == 'v' && line[1] == ' ')
        {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            // Store in the vertex array
            const Vector v = { x, y, z };
            vertices[v_pos++] = v;
            // Debug message to be sure of the information read
            printf("[DEBUG] VERTEX %d READ FROM FILE: %.2f, %.2f, %.2f\n", v_pos, x, y, z);
        }
        // Read a Face Line - supports multiple formats and does automatic triangulation
        else if (line[0] == 'f' && line[1] == ' ')
        {
            char* ptr = line + 2;
            int v_indices[5];
            int count = 0;

            while (*ptr && count < 5) {
                int v = 0, vt = 0, vn = 0;
                int matches = sscanf(ptr, "%d/%d/%d", &v, &vt, &vn);

                if (matches != 3) {
                    matches = sscanf(ptr, "%d//%d", &v, &vn);
                    if (matches != 2) {
                        matches = sscanf(ptr, "%d/%d", &v, &vt);
                        if (matches != 2) {
                            matches = sscanf(ptr, "%d", &v);
                            if (matches != 1) {
                                fprintf(stderr, "[ERROR] FACE FORMAT IS NOT VALID: %s", ptr);
                                break;
                            }
                        }
                    }
                }
                // Convert base 1 to base 0 for array indices
                v_indices[count++] = v - 1;
                // Go tto the next vertex
                while (*ptr && *ptr != ' ') ptr++;
                while (*ptr == ' ') ptr++;
            }

            if (count < 3) {
                fprintf(stderr, "[ERROR] FACE WITH LESS THEN 3 VERTICES DETECTED: %s", line);
            } else
            {
                // Automatic triangulation of faces that are not triangles
                for (int i = 1; i < count - 1; i++) {
                    const Triangle t = {
                        vertices[v_indices[0]],
                        vertices[v_indices[i]],
                        vertices[v_indices[i + 1]]
                    };

                    if (f_pos + 1 > mesh.nTris)
                    {
                        mesh.nTris++;
                        do
                        {
                            mesh.tris = realloc(mesh.tris, mesh.nTris * sizeof(Triangle));
                            if (mesh.tris == NULL)
                                printf("[DEBUG] ERROR REALLOCATING MESH %d TRIANGLES", i);
                        } while (mesh.tris == NULL);
                    }
                    mesh.tris[f_pos++] = t;
                    printf("[DEBUG] TRI: %d, %d, %d\n", v_indices[0], v_indices[i], v_indices[i + 1]);
                }
            }
        }
    }
    // Put mesh on engine TODO: Make this more solid
    ALLOCATE(e->meshes, sizeof(Mesh));
    e->meshes[0] = mesh;
    // Close file
    fclose(fp);
    return 1;
}
