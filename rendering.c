#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "LCD_1in3.h"
#include "DEV_Config.h"
#include "Infrared.h"

#include "shape.h"
#include "rendering.h"
#include "vectors.h"


const float Y_RATIO = LCD_1IN3_HEIGHT/VIEWPORT_DIM;
const float X_RATIO = LCD_1IN3_WIDTH/VIEWPORT_DIM;


extern FloatVec3D shapeVerticies[];
extern IntVec3D shapeTriangles[];
extern FloatVec3D unitPyramid[];
extern IntVec3D pyramidTriangles[];
extern FloatVec3D unitCube[];
extern IntVec3D cubeTriangles[];
extern size_t shapeVertexCount;
extern size_t shapeTriangleCount;


void memset16(uint16_t *ptr, uint16_t fill, size_t num) {
    for (; num; num--) {
        *(ptr++) = fill;
    }
}

void rotatePoint(FloatVec3D input, FloatVec3D *output, float xAngle) {
    float cosX = cosf(xAngle);
    float sinX = sinf(xAngle);

    output->x = input.x*cosX - input.z*sinX;
    output->y = input.y;
    output->z = input.z*cosX + input.x*sinX;
}

void projectPoint(FloatVec3D p3D, IntVec2D *p2D) {
    if (p3D.z < CLIP_CLOSE_Z) {
        return; //this vertex is invalid, all triangles with it won't be drawn
    }
    
    float newX = p3D.x / p3D.z;
    newX *= X_RATIO;

    float newY = p3D.y / p3D.z;
    newY *= Y_RATIO;
    
    p2D->x = (int)newX + LCD_1IN3_WIDTH/2;
    p2D->y = (int)newY + LCD_1IN3_HEIGHT/2;
}


void drawPoint(Screen *screen, IntVec2D p, uint16_t fill) {
    if (p.x < 0 || p.y < 0 || p.x >= LCD_1IN3_WIDTH || p.y >= LCD_1IN3_HEIGHT) {
        return;
    }
    screen->imageBuffer[p.x + p.y*LCD_1IN3_WIDTH] = fill;
}

//this is called with the following assumptions: x1 > x0, z1 > z0
void drawHorizontalLine(Screen *screen, int y, int x0, int x1, float z0, float z1, uint16_t fill) {
    if (y >= LCD_1IN3_HEIGHT || y < 0) {return;}
    int dx = x1-x0;
    if (dx == 0) {return;} //if dx < 1 (shouldn't happen) it just wont draw anything anyway
    float zSlope = (z1-z0)/dx;
    

    x0 = MAX(x0, 0);
    x1 = MIN(x1, LCD_1IN3_WIDTH);
    int yOffset = y*LCD_1IN3_WIDTH;
    for (; x0 < x1; x0++) {
        int index = x0+yOffset;
        if (z0 > screen->zBuffer[index]) {
            screen->imageBuffer[index] = fill;
            screen->zBuffer[index] = z0;
        }
        z0 += zSlope;
    }
}


//https://gist.github.com/bert/1085538
void drawLine(Screen *screen, IntVec2D p1, IntVec2D p2, uint16_t fill) {
    int dx = abs(p2.x - p1.x), sx = p1.x < p2.x ? 1 : -1;
    int dy = -abs(p2.y - p1.y), sy = p1.y < p2.y ? 1 : -1;
    int err = dx + dy, e2;
 
    for(;;) {  
        drawPoint(screen, p1, fill);
        if (p1.x == p2.x && p1.y == p2.y) {break;}
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; p1.x += sx; } 
        if (e2 <= dx) { err += dx; p1.y += sy; }
    }
}


void drawFilledTriangle(Shape *shape, Screen *screen, int index, uint16_t fill) {
    IntVec3D triangleIndicies = shape->triangles[index];
    const IntVec2D *temp;
    //the three points of the triangle to be drawn
    const IntVec2D *p0 = shape->projectedPoints + triangleIndicies.x;
    const IntVec2D *p1 = shape->projectedPoints + triangleIndicies.y;
    const IntVec2D *p2 = shape->projectedPoints + triangleIndicies.z;
    // the 1/z coordinates associated with the 3 points in world space
    float p0z = 1.0f/(shape->transformedVerticies[triangleIndicies.x].z);
    float p1z = 1.0f/(shape->transformedVerticies[triangleIndicies.y].z);
    float p2z = 1.0f/(shape->transformedVerticies[triangleIndicies.z].z);

    if (p1->y < p0->y) {
        temp = p1;
        p1 = p0;
        p0 = temp;
    }
    if (p2->y < p0->y) {
        temp = p2;
        p2 = p0;
        p0 = temp;
    } 
    if (p2->y < p1->y) {
        temp = p2;
        p2 = p1;
        p1 = temp;
    }

    if (p2->y == p0->y) { //the triangles points all lies on one line
        return;
    }

    float dy = (p2->y - p0->y);
    float longSideYSlope = (float)(p2->x - p0->x)/dy;
    float longSideZSlope = (float)(p2z - p0z)/dy;

    dy = (p1->y - p0->y); //if the short side dy is 0 these slopes won't be involved anyway so they can be inf
    float shortSideYSlope = (float)(p1->x - p0->x)/dy; 
    float shortSideZSlope = (float)(p1z - p0z)/dy;

    float x0 = p0->x;
    float x1 = p0->x;
    float z0 = p0z;
    float z1 = p0z;

    if (p0->y == p1->y) { //if the first two points have the same y, make sure x1 has a bigger x than x0
        if (p1->x > p0->x) {
            x1 = p1->x;
            z1 = p1z;
        } else {
            x0 = p1->x;
            z0 = p1z;
        }
    }

    float leftYSlope, rightYSlope;
    float leftZSlope, rightZSlope;
    bool shortSideisLeft = (shortSideYSlope < longSideYSlope);
    if (shortSideisLeft) {
        leftYSlope = shortSideYSlope;
        rightYSlope = longSideYSlope;
        leftZSlope = shortSideZSlope;
        rightZSlope = longSideZSlope;
    } else {
        rightYSlope = shortSideYSlope;
        leftYSlope = longSideYSlope;
        rightZSlope = shortSideZSlope;
        leftZSlope = longSideZSlope;
    }

    int currentY = p0->y;
    for (; currentY < p1->y; currentY++) { // iterate from one point to another, draw one half
        drawHorizontalLine(screen, currentY, x0, x1, z0, z1, fill);
        x1 += rightYSlope;
        z1 += rightZSlope;
        x0 += leftYSlope;
        z0 += leftZSlope;
    } 

    dy = (p2->y - p1->y);
    shortSideYSlope = (float)(p2->x - p1->x)/dy;
    shortSideZSlope = (float)(p2z - p1z)/dy;
    if (shortSideisLeft) {
        leftYSlope = shortSideYSlope;
        leftZSlope = shortSideZSlope;
    } else {
        rightYSlope = shortSideYSlope;
        rightZSlope = shortSideZSlope;
    }
    

    for (; currentY < p2->y; currentY++) { //iterate between the other half
        drawHorizontalLine(screen, currentY, x0, x1, z0, z1, fill);
        x1 += rightYSlope;
        z1 += rightZSlope;
        x0 += leftYSlope;
        z0 += leftZSlope;
    }

}

int main() {
    Screen screen;
    FloatVec3D cameraPoint = {0, 0, 0};

    float xAngle = 0;
    float cosX = 1.0f;
    float sinX = 0.0f;

    if(DEV_Module_Init()!=0){
        return -1;
    }
    DEV_SET_PWM(50);
    LCD_1IN3_Init(HORIZONTAL);

    
    SET_Infrared_PIN(UP_PIN);
    SET_Infrared_PIN(DOWN_PIN);
    SET_Infrared_PIN(LEFT_PIN);
    SET_Infrared_PIN(RIGHT_PIN);
    SET_Infrared_PIN(FORWARD_PIN);
    SET_Infrared_PIN(BACKWARD_PIN);
    SET_Infrared_PIN(TURNLEFT_PIN);
    SET_Infrared_PIN(TURNRIGHT_PIN);
    SET_Infrared_PIN(EXIT_PIN);

    
    Shape* customShape = Shape_init(
            shapeVertexCount, 
            shapeTriangleCount, 
            shapeVerticies, 
            shapeTriangles
        );
    Shape_transform(customShape, 0, 0, 5);
    

    while(1) {
        absolute_time_t beginTime = get_absolute_time();
        if (DEV_Digital_Read(LEFT_PIN) == 0) {
            cameraPoint.x -= (0.1f*cosX);
            cameraPoint.z += (0.1f*sinX);
        } else if (DEV_Digital_Read(RIGHT_PIN) == 0) {
            cameraPoint.x += (0.1f*cosX);
            cameraPoint.z -= (0.1f*sinX);
        }

        if (DEV_Digital_Read(UP_PIN) == 0) {
            cameraPoint.y -= 0.1f;
        } else if (DEV_Digital_Read(DOWN_PIN) == 0) {
            cameraPoint.y += 0.1f;
        }

        if (DEV_Digital_Read(FORWARD_PIN) == 0) {
            cameraPoint.x += (0.1f*sinX); 
            cameraPoint.z += (0.1f*cosX);
        } else if (DEV_Digital_Read(BACKWARD_PIN) == 0) {
            cameraPoint.x -= (0.1f*sinX);
            cameraPoint.z -= (0.1f*cosX);
        }

        if (DEV_Digital_Read(TURNRIGHT_PIN) == 0) {
            xAngle += 0.1f;
            sinX = sinf(xAngle);
            cosX = cosf(xAngle);
        } else if (DEV_Digital_Read(TURNLEFT_PIN) == 0) {
            xAngle -= 0.1f;
            sinX = sinf(xAngle);
            cosX = cosf(xAngle);
        }

        if (DEV_Digital_Read(EXIT_PIN) == 0) {
            Shape_destroy(customShape);
            memset16(screen.imageBuffer, 0, PIXEL_COUNT);
            LCD_1IN3_Display(screen.imageBuffer);
            DEV_Module_Exit();
            return 0;
        }

        for (int i = 0; i < PIXEL_COUNT; i++) {
            screen.imageBuffer[i] = 0;
            screen.zBuffer[i] = 0.0f;
        }

        Shape_draw(customShape, &screen, cameraPoint, xAngle, RED);
        
        LCD_1IN3_Display(screen.imageBuffer);
        printf("delta time: %lld\n", absolute_time_diff_us(beginTime, get_absolute_time()));
        DEV_Delay_ms(10);
    }
}

