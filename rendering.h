#ifndef RENDERING_H
#define RENDERING_H


#include <stdint.h>
#include <stddef.h>
#include "LCD_1in3.h"
#include "vectors.h"

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
    uint16_t imageBuffer[PIXEL_COUNT];
    float zBuffer[PIXEL_COUNT];
} Screen;

#include "shape.h"

void drawFilledTriangle(Shape *shape, Screen *screen, int index, uint16_t fill);
void drawLine(Screen *screen, IntVec2D p1, IntVec2D p2, uint16_t fill);
void drawHorizontalLine(Screen *screen, int y, int x0, int x1, float z0, float z1, uint16_t fill);
void drawPoint(Screen *screen, IntVec2D p, uint16_t fill);

void projectPoint(FloatVec3D p3D, IntVec2D *p2D);
void rotatePoint(FloatVec3D input, FloatVec3D *output, float xAngle);

void memset16(uint16_t *ptr, uint16_t fill, size_t num);

#endif