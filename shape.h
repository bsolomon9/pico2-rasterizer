#ifndef SHAPE_H
#define SHAPE_H

#include "pico/stdlib.h"
#include "vectors.h"


typedef struct {
    size_t vertexCount;
    size_t triangleCount;
    FloatVec3D transformShift;
    const FloatVec3D const *verticies;
    const IntVec3D const *triangles;

    FloatVec3D *transformedVerticies;
    IntVec2D *projectedPoints;
    FloatVec3D *triangleNormals;
} Shape;


#include "rendering.h"


Shape* Shape_init(size_t vertexCount, size_t triangleCount, const FloatVec3D *verticies, const IntVec3D *triangles);
void Shape_destroy(Shape *shape);
void Shape_transform(Shape *shape, float xShift, float yShift, float zShift);
void Shape_project(Shape *shape, FloatVec3D cameraPoint, float xAngle);
void Shape_draw(Shape *shape, Screen *screen, FloatVec3D cameraPoint, float xAngle, uint16_t fill);


#endif