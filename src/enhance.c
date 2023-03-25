#include <stdio.h>

#include "color.h"

static uint32_t bitmap[] = {
    0x00000000, 0x00a0,

    // Block graphics
    // 0xffff0000, 0x2580,  // upper 1/2; redundant with inverse lower 1/2

    0x0000000f, 0x2581,  // lower 1/8
    0x000000ff, 0x2582,  // lower 1/4
    0x00000fff, 0x2583,  //
    0x0000ffff, 0x2584,  // lower 1/2
    0x000fffff, 0x2585,  //
    0x00ffffff, 0x2586,  // lower 3/4
    0x0fffffff, 0x2587,  //
    // 0xffffffff, 0x2588,  // full; redundant with inverse space

    0xeeeeeeee, 0x258a,  // left 3/4
    0xcccccccc, 0x258c,  // left 1/2
    0x88888888, 0x258e,  // left 1/4

    0x0000cccc, 0x2596,  // quadrant lower left
    0x00003333, 0x2597,  // quadrant lower right
    0xcccc0000, 0x2598,  // quadrant upper left
    // 0xccccffff, 0x2599,  // 3/4 redundant with inverse 1/4
    0xcccc3333, 0x259a,  // diagonal 1/2
                         // 0xffffcccc, 0x259b,  // 3/4 redundant
    // 0xffff3333, 0x259c,  // 3/4 redundant
    0x33330000, 0x259d,  // quadrant upper right
                         // 0x3333cccc, 0x259e,  // 3/4 redundant
    // 0x3333ffff, 0x259f,  // 3/4 redundant

    // Line drawing subset: no double lines, no complex light lines

    0x000ff000, 0x2501,  // Heavy horizontal
    0x66666666, 0x2503,  // Heavy vertical

    0x00077666, 0x250f,  // Heavy down and right
    0x000ee666, 0x2513,  // Heavy down and left
    0x66677000, 0x2517,  // Heavy up and right
    0x666ee000, 0x251b,  // Heavy up and left

    0x66677666, 0x2523,  // Heavy vertical and right
    0x666ee666, 0x252b,  // Heavy vertical and left
    0x000ff666, 0x2533,  // Heavy down and horizontal
    0x666ff000, 0x253b,  // Heavy up and horizontal
    0x666ff666, 0x254b,  // Heavy cross

    0x000cc000, 0x2578,  // Bold horizontal left
    0x00066000, 0x2579,  // Bold horizontal up
    0x00033000, 0x257a,  // Bold horizontal right
    0x00066000, 0x257b,  // Bold horizontal down

    0x06600660, 0x254f,  // Heavy double dash vertical

    0x000f0000, 0x2500,  // Light horizontal
    0x0000f000, 0x2500,  //
    0x44444444, 0x2502,  // Light vertical
    0x22222222, 0x2502,  //

    0x000e0000, 0x2574,  // light left
    0x0000e000, 0x2574,  // light left
    0x44440000, 0x2575,  // light up
    0x22220000, 0x2575,  // light up
    0x00030000, 0x2576,  // light right
    0x00003000, 0x2576,  // light right
    0x00004444, 0x2577,  // light down
    0x00002222, 0x2577,  // light down

    // Misc technical

    0x44444444, 0x23a2,  // [ extension
    0x22222222, 0x23a5,  // ] extension

    0x0f000000, 0x23ba,  // Horizontal scanline 1
    0x00f00000, 0x23bb,  // Horizontal scanline 3
    0x00000f00, 0x23bc,  // Horizontal scanline 7
    0x000000f0, 0x23bd,  // Horizontal scanline 9

    // Geometrical shapes. Tricky because some of them are too wide.

    // 0x00ffff00, 0x25fe,  // Black medium small square
    0x00066000, 0x25aa,  // Black small square
};

static uint32_t getPixelsDist(Color p1[8][4], Color p2[8][4]) {
    uint32_t dist = 0;
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 4; y++) {
            dist += getColorSqrDist(p1[x][y], p2[x][y]);
        }
    }
    return dist;
}

static inline int getBit4x8(uint32_t mask, int x, int y) {
    return (mask >> (31 - (x * 4 + y))) & 1;
}

static void printUnicode(uint32_t codepoint) {
    if (codepoint < 128) {
        printf("%c", (char)codepoint);
    } else if (codepoint < 0x7ff) {
        printf("%c%c", (char)(0xc0 | (codepoint >> 6)),
               (char)(0x80 | (codepoint & 0x3f)));
    } else if (codepoint < 0xffff) {
        printf("%c%c%c", (char)(0xe0 | (codepoint >> 12)),
               (char)(0x80 | ((codepoint >> 6) & 0x3f)),
               (char)(0x80 | (codepoint & 0x3f)));
    } else if (codepoint < 0x10ffff) {
        printf("%c%c%c%c", (char)(0xf0 | (codepoint >> 18)),
               (char)(0x80 | ((codepoint >> 12) & 0x3f)),
               (char)(0x80 | ((codepoint >> 6) & 0x3f)),
               (char)(0x80 | (codepoint & 0x3f)));
    }
}

void printClosestShape(Color pixels[8][4], SetColorFunc setColor) {
    uint32_t min_dist = UINT32_MAX;
    uint32_t symbol = 0x00a0;
    Color result_colors[2] = {0};

    // Find the closest symbol and colors
    for (size_t i = 0; i < sizeof(bitmap) / sizeof(uint32_t); i += 2) {
        int mask = bitmap[i];
        // Calculate the average color of fg and bg
        int r_sum[2] = {0}, g_sum[2] = {0}, b_sum[2] = {0}, count[2] = {0};
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 4; y++) {
                int group = getBit4x8(mask, x, y);
                r_sum[group] += pixels[x][y].r;
                g_sum[group] += pixels[x][y].g;
                b_sum[group] += pixels[x][y].b;
                count[group]++;
            }
        }

        Color colors[2];
        if (count[0]) {
            colors[0].r = r_sum[0] / count[0];
            colors[0].g = g_sum[0] / count[0];
            colors[0].b = b_sum[0] / count[0];
        }
        if (count[1]) {
            colors[1].r = r_sum[1] / count[1];
            colors[1].g = g_sum[1] / count[1];
            colors[1].b = b_sum[1] / count[1];
        }

        // Calculate the distance between the result and the original
        Color result_pixels[8][4];
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 4; y++) {
                int group = getBit4x8(mask, x, y);
                result_pixels[x][y] = colors[group];
            }
        }
        uint32_t dist = getPixelsDist(result_pixels, pixels);
        if (dist < min_dist) {
            symbol = bitmap[i + 1];
            result_colors[0] = colors[0];
            result_colors[1] = colors[1];
            min_dist = dist;
        }
    }
    setColor(result_colors[0].color, 1);
    setColor(result_colors[1].color, 0);
    printUnicode(symbol);
}
