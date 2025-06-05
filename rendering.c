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



//specifically for things like triangles, meant to be quicker than drawLine
void drawHorizontalLine(Screen *screen, int x, int y, int length, uint16_t fill) {
    if (x < 0) {
        length += x;
        x = 0;
    }
    length = MIN(length, LCD_1IN3_WIDTH-x);

    if (y < 0 || y >= LCD_1IN3_HEIGHT || length <= 0) {
        return;
    }
    memset16(screen->imageBuffer + x + y * LCD_1IN3_WIDTH, fill, length);
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
    const IntVec2D *temp;
    const IntVec2D *p0 = shape->projectedPoints + shape->triangles[index].x;
    const IntVec2D *p1 = shape->projectedPoints + shape->triangles[index].y;
    const IntVec2D *p2 = shape->projectedPoints + shape->triangles[index].z;

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

    float longSideSlope = (float)(p2->x - p0->x)/(p2->y - p0->y);
    int currentY = p0->y;

    float shortSideSlope = (float)(p1->x - p0->x)/(p1->y - p0->y); //can eval to float infinities, its fine
    float x0 = p0->x;
    float x1 = p0->x;  
    if (p0->y == p1->y) { //if the first two points have the same y, make sure x1 has a bigger x than x0
        if (p1->x > p0->x) {
            x1 = p1->x;
            x0 = p0->x;
        } else {
            x1 = p0->x;
            x0 = p1->x;
        }
    }

    float leftSlope, rightSlope;
    bool shortSideisLeft = (shortSideSlope < longSideSlope);
    if (shortSideisLeft) {
        leftSlope = shortSideSlope;
        rightSlope = longSideSlope;
    } else {
        rightSlope = shortSideSlope;
        leftSlope = longSideSlope;
    }

    for (; currentY < p1->y; currentY++) { // iterate from one point to another, draw one half
        drawHorizontalLine(screen, 
            roundf(x0), 
            currentY, 
            (x1-x0)+1, 
            fill
        );
        x1 += rightSlope;
        x0 += leftSlope;
    } 

    shortSideSlope = (float)(p2->x - p1->x)/(p2->y - p1->y);
    if (shortSideisLeft) {
        leftSlope = shortSideSlope;
    } else {
        rightSlope = shortSideSlope;
    }

    for (; currentY < p2->y; currentY++) { //iterate between the other half
        drawHorizontalLine(screen, 
            roundf(x0), 
            currentY, 
            (x1-x0)+1, 
            fill
        );
        x1 += rightSlope;
        x0 += leftSlope;
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

        memset16(screen.imageBuffer, 0, PIXEL_COUNT);
        Shape_draw(customShape, &screen, cameraPoint, xAngle, RED);
        
        LCD_1IN3_Display(screen.imageBuffer);
        printf("delta time: %lld\n", absolute_time_diff_us(beginTime, get_absolute_time()));
        DEV_Delay_ms(10);
    }
}

