#ifndef RENDERING_H
#define RENDERING_H


#include <stdint.h>
#include <stddef.h>
#include "LCD_1in3.h"

#define PIXEL_COUNT (LCD_1IN3_HEIGHT*LCD_1IN3_WIDTH)

#define RED 0b0000011111000000
#define GREEN 0b0000000000111111
#define BLUE 0b1111100000000000
#define WHITE 0b1111111111111111

#define UP_PIN 2
#define DOWN_PIN 18
#define LEFT_PIN 16
#define RIGHT_PIN 20
#define FORWARD_PIN 15
#define BACKWARD_PIN 17
#define TURNLEFT_PIN 19 
#define TURNRIGHT_PIN 21
#define EXIT_PIN 3

#define FOV 90

// 2 * tan(FOV/2)
#define VIEWPORT_DIM 2

#define CLIP_CLOSE_Z 0.001f


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

typedef struct {
    uint16_t imageBuffer[PIXEL_COUNT];
    //float zBuffer[PIXEL_COUNT];
} Screen;


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


void drawFilledTriangle(Shape *shape, Screen *screen, int index, uint16_t fill);
void drawLine(Screen *screen, IntVec2D p1, IntVec2D p2, uint16_t fill);
void drawHorizontalLine(Screen *screen, int x, int y, int length, uint16_t fill);
void drawPoint(Screen *screen, IntVec2D p, uint16_t fill);

void projectPoint(FloatVec3D p3D, IntVec2D *p2D);
void rotatePoint(FloatVec3D input, FloatVec3D *output, float xAngle);

void memset16(uint16_t *ptr, uint16_t fill, size_t num);

void FloatVec3D_crossProduct(FloatVec3D first, FloatVec3D second, FloatVec3D *output);
float FloatVec3D_dotProduct(FloatVec3D first, FloatVec3D second);
void FloatVec3D_subtract(FloatVec3D first, FloatVec3D second, FloatVec3D *output);
void FloatVec3D_add(FloatVec3D first, FloatVec3D second, FloatVec3D *output);

#endif