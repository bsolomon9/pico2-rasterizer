#ifndef SHAPE_H
#define SHAPE_H

#include "pico/stdlib.h"
#include "rendering.h"


Shape* Shape_init(size_t vertexCount, size_t triangleCount, const FloatVec3D *verticies, const IntVec3D *triangles);
void Shape_destroy(Shape *shape);
void Shape_transform(Shape *shape, float xShift, float yShift, float zShift);
void Shape_project(Shape *shape, FloatVec3D cameraPoint, float xAngle);
void Shape_draw(Shape *shape, Screen *screen, FloatVec3D cameraPoint, float xAngle, uint16_t fill);


#endif