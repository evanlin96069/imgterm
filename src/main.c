#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "stb_image.h"
#include "stb_image_resize.h"

int getWindowSize(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
}

int main(int argc, char* argv[]) {
    const char* file_path = NULL;
    int target_width = -1;
    int raw_size = 0;
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (strcmp(arg, "-w") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "No value provided for %s\n", arg);
                return 1;
            }
            target_width = atoi(argv[i]);
            if (target_width < 0) {
                fprintf(stderr, "Width cannot be negative\n");
                return 1;
            }
        } else if (strcmp(arg, "-r") == 0) {
            raw_size = 1;
        } else {
            file_path = arg;
        }
    }

    if (!file_path) {
        fprintf(stderr, "No file is provided\n");
        return 1;
    }

    int w, h;
    uint32_t* img = (uint32_t*)stbi_load(file_path, &w, &h, NULL, 4);
    if (!img) {
        fprintf(stderr, "Cannot open file %s\n", file_path);
        return 1;
    }

    int rh = 0, rw = 0;
    uint32_t* resize = NULL;

    if (target_width > 0) {
        rw = target_width;
        rh = h * rw / w;
    } else {
        int sw, sh;
        if (getWindowSize(&sh, &sw) != -1) {
            // sh *= 2;
            rh = sh;
            rw = w * rh / h;
            if (target_width == 0 || rw > sw) {
                rw = sw;
                rh = h * rw / w;
            }
        }
    }

    uint32_t* pixels = img;
    if (!raw_size) {
        resize = malloc(sizeof(uint32_t) * rw * rh);
        if (!resize) {
            fprintf(stderr, "Cannot allocate memory for resize image\n");
            return 1;
        }
        stbir_resize_uint8((uint8_t*)img, w, h, sizeof(uint32_t) * w,
                           (uint8_t*)resize, rw, rh, sizeof(uint32_t) * rw, 4);
        w = rw;
        h = rh;
        pixels = resize;
    }

    for (int x = 0; x + 1 < h; x += 2) {
        for (int y = 0; y < w; y++) {
            uint32_t pixel, r, g, b, a;
            pixel = pixels[x * w + y];
            r = (pixel >> 8 * 0) & 0xFF;
            g = (pixel >> 8 * 1) & 0xFF;
            b = (pixel >> 8 * 2) & 0xFF;
            a = (pixel >> 8 * 3) & 0xFF;
            r *= a / 255;
            g *= a / 255;
            b *= a / 255;
            printf("\x1b[48;2;%d;%d;%dm", r, g, b);

            pixel = pixels[(x + 1) * w + y];
            r = (pixel >> 8 * 0) & 0xFF;
            g = (pixel >> 8 * 1) & 0xFF;
            b = (pixel >> 8 * 2) & 0xFF;
            a = (pixel >> 8 * 3) & 0xFF;
            r *= a / 255;
            g *= a / 255;
            b *= a / 255;
            printf("\x1b[38;2;%d;%d;%dm\u2584", r, g, b);
        }
        printf("\x1b[m\n");
    }

    free(resize);
    stbi_image_free(img);

    return 0;
}
