#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef union Color {
    uint32_t color;
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
} Color;

typedef void (*SetColorFunc)(uint32_t color, int is_bg);

uint32_t getColorSqrDist(Color a, Color b);

void setTrueColor(uint32_t color, int is_bg);
void set256Color(uint32_t color, int is_bg);

#endif
