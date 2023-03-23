#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "color.h"
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
    int th = -1, tw = -1;
    int p = 50;
    int rs = 0;

    SetColorFunc setColor = setTrueColor;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:p:rc?")) != -1) {
        switch (opt) {
            case 'w':
                tw = atoi(optarg);
                if (tw < 0) {
                    fprintf(stderr, "Width cannot be negative\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                th = atoi(optarg);
                if (th < 0) {
                    fprintf(stderr, "Height cannot be negative\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                p = atoi(optarg);
                if (p < 0 || p > 100) {
                    fprintf(stderr, "Percentage should be between 0 and 100\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'r':
                rs = 1;
                break;
            case 'c':
                setColor = set256Color;
                break;
            default:
                fprintf(stderr,
                        "Usage: %s [-w width] [-h height] [-p percentage] [-r] "
                        "[-c] files\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        const char* file_path = argv[i];
        int w, h;
        uint32_t* img = (uint32_t*)stbi_load(file_path, &w, &h, NULL, 4);
        if (!img) {
            fprintf(stderr, "Cannot open file %s\n", file_path);
            exit(EXIT_FAILURE);
        }

        uint32_t* pixels = img;
        uint32_t* resize = NULL;
        if (!rs) {
            int rh = 0, rw = 0, sw = 0, sh = 0;
            getWindowSize(&sh, &sw);
            sh *= 2;

            if (tw == -1 && th == -1) {
                rh = sh * p / 100.0f;
                rw = w * rh / h;
                if (rw > sw) {
                    rw = sw;
                    rh = h * rw / w;
                }
            } else {
                if (tw != -1) {
                    rw = tw ? tw : sw;
                    if (th == -1) {
                        rh = h * rw / w;
                    }
                }
                if (th != -1) {
                    rh = th ? th : sh;
                    if (tw == -1) {
                        rw = w * rh / h;
                    }
                }
            }

            resize = malloc(sizeof(uint32_t) * rw * rh);
            if (!resize) {
                fprintf(stderr, "Cannot allocate memory for resize image\n");
                exit(EXIT_FAILURE);
            }
            stbir_resize_uint8((uint8_t*)img, w, h, sizeof(uint32_t) * w,
                               (uint8_t*)resize, rw, rh, sizeof(uint32_t) * rw,
                               4);
            w = rw;
            h = rh;
            pixels = resize;
        }

        for (int x = 0; x + 1 < h; x += 2) {
            for (int y = 0; y < w; y++) {
                setColor(pixels[x * w + y], 1);
                setColor(pixels[(x + 1) * w + y], 0);
                printf("\u2584");
            }
            printf("\x1b[m\n");
        }

        free(resize);
        stbi_image_free(img);
    }
    exit(EXIT_SUCCESS);
}
