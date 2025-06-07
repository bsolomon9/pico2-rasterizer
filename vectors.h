#ifndef VECTORS_H
#define VECTORS_H

typedef struct {
    float x;
    float y;
} FloatVec2D; 

typedef struct {
    float x;
    float y;
    float z;
} FloatVec3D;

typedef struct {
    int x;
    int y;
} IntVec2D;

typedef struct {
    int x;
    int y;
    int z;
} IntVec3D;


void FloatVec3D_crossProduct(FloatVec3D first, FloatVec3D second, FloatVec3D *output);
float FloatVec3D_dotProduct(FloatVec3D first, FloatVec3D second);
void FloatVec3D_subtract(FloatVec3D first, FloatVec3D second, FloatVec3D *output);
void FloatVec3D_add(FloatVec3D first, FloatVec3D second, FloatVec3D *output);

#endif