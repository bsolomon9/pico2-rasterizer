#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "shape.h"


Shape* Shape_init(size_t vertexCount, size_t triangleCount, const FloatVec3D *verticies, const IntVec3D *triangles) {
    Shape* shape = malloc(sizeof(Shape));
    assert(shape);

    shape->vertexCount = vertexCount;
    shape->triangleCount = triangleCount;

    shape->verticies = verticies;
    shape->triangles = triangles;

    shape->transformedVerticies = malloc(sizeof(FloatVec3D) * vertexCount);
    memset(shape->transformedVerticies, 0, sizeof(FloatVec3D) * vertexCount);
    
    shape->projectedPoints = malloc(sizeof(IntVec2D) * vertexCount);
    memset(shape->projectedPoints, 0, sizeof(IntVec2D) * vertexCount);

    shape->triangleNormals = malloc(sizeof(FloatVec3D) * triangleCount);
    //precalculating the surface normals of each triangle
    for (int i = 0; i < triangleCount; i++) {
        FloatVec3D side1;
        FloatVec3D side2;
        IntVec3D triangleIndicies = triangles[i];

        FloatVec3D_subtract(verticies[triangleIndicies.z], verticies[triangleIndicies.x], &side1);
        FloatVec3D_subtract(verticies[triangleIndicies.y], verticies[triangleIndicies.x], &side2);  

        FloatVec3D_crossProduct(side1, side2, shape->triangleNormals + i);
    }

    shape->transformShift.x = 0;
    shape->transformShift.y = 0;
    shape->transformShift.z = 0;

    return shape;
}


void Shape_destroy(Shape *shape) {
    free(shape->transformedVerticies);
    free(shape->projectedPoints);
    free(shape);
}


void Shape_transform(Shape *shape, float xShift, float yShift, float zShift) {
    shape->transformShift.x += xShift;
    shape->transformShift.y += yShift;
    shape->transformShift.z += zShift;
}


void Shape_project(Shape *shape, FloatVec3D cameraPoint, float xAngle) {
    for (int i = 0; i < shape->vertexCount; i++) {    
        FloatVec3D_add(shape->verticies[i], shape->transformShift, shape->transformedVerticies+i);
        FloatVec3D_subtract(shape->transformedVerticies[i], cameraPoint, shape->transformedVerticies+i);
        rotatePoint(shape->transformedVerticies[i], shape->transformedVerticies+i, xAngle);
        projectPoint(shape->transformedVerticies[i], shape->projectedPoints+i);
    }
}


void Shape_draw(Shape *shape, Screen *screen, FloatVec3D cameraPoint, float xAngle, uint16_t fill) {
    Shape_project(shape, cameraPoint, xAngle);
    
    IntVec2D *points = shape->projectedPoints;

    for (int i = 0; i < shape->triangleCount; i++) {
        
        IntVec3D triangleIndicies = shape->triangles[i];
        FloatVec3D lineOfSight;
        
        if (shape->transformedVerticies[triangleIndicies.x].z < CLIP_CLOSE_Z
            ||shape->transformedVerticies[triangleIndicies.y].z < CLIP_CLOSE_Z
            || shape->transformedVerticies[triangleIndicies.z].z < CLIP_CLOSE_Z) {
                continue;
        }
        FloatVec3D_subtract(shape->verticies[triangleIndicies.x], cameraPoint, &lineOfSight);
        FloatVec3D_add(lineOfSight, shape->transformShift, &lineOfSight);
        
        if (FloatVec3D_dotProduct(lineOfSight, shape->triangleNormals[i]) > 0) { //cull
            continue;
        }

        drawFilledTriangle(screen, 
            points + (triangleIndicies.x), 
            points + (triangleIndicies.y),
            points + (triangleIndicies.z),
            (i % 32) << 5);
        
       /*
        drawLine(screen->imageBuffer,
            shape->projectedPoints[triangleIndicies.x],
            shape->projectedPoints[triangleIndicies.y],
            BLUE);
        drawLine(screen->imageBuffer,
            shape->projectedPoints[triangleIndicies.y],
            shape->projectedPoints[triangleIndicies.z],
            BLUE);*/
    }
}