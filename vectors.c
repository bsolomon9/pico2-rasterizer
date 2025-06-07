#include "vectors.h"

void FloatVec3D_crossProduct(FloatVec3D first, FloatVec3D second, FloatVec3D *output) {
    output->x = first.y*second.z - first.z*second.y;
    output->y = first.z*second.x - first.x*second.z;
    output->z = first.x*second.y - first.y*second.x;  
}

float FloatVec3D_dotProduct(FloatVec3D first, FloatVec3D second) {
    return first.x*second.x + first.y*second.y + first.z*second.z;
}

void FloatVec3D_subtract(FloatVec3D first, FloatVec3D second, FloatVec3D *output) {
    output->x = first.x - second.x;
    output->y = first.y - second.y;
    output->z = first.z - second.z;
}

void FloatVec3D_add(FloatVec3D first, FloatVec3D second, FloatVec3D *output) {
    output->x = first.x + second.x;
    output->y = first.y + second.y;
    output->z = first.z + second.z;
}