#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef void (*SetColorFunc)(uint32_t color, int is_bg);

void setTrueColor(uint32_t color, int is_bg);
void set256Color(uint32_t color, int is_bg);

#endif
