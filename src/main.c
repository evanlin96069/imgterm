#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "color.h"
#include "enhance.h"
#include "stb_image.h"
#include "stb_image_resize.h"

static int getWindowSize(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
}

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s [options] [files]\n", prog);
    fprintf(stderr, "Options\n");
    fprintf(stderr, "    -w width\n");
    fprintf(stderr,
            "        Pixel width of the image. Use screen width if set to 0\n");
    fprintf(stderr, "    -h height\n");
    fprintf(
        stderr,
        "        Pixel height of the image. Use screen height if set to 0\n");
    fprintf(stderr, "    -p percentage\n");
    fprintf(stderr,
            "        Percentage of the image height to the screen height "
            "(Default=50)\n");
    fprintf(stderr, "    -e level\n");
    fprintf(stderr, "        Enhance level (Default=2)\n");
    fprintf(stderr, "        0 = Use space\n");
    fprintf(stderr, "        1 = Use lower half block\n");
    fprintf(stderr, "        2 = Use more unicode characters\n");
    fprintf(stderr, "    -r  Use the raw size of the image\n");
    fprintf(stderr, "    -8  Use 8-bit colors\n");
    fprintf(stderr, "    -?  Print this help\n");
}

int main(int argc, char* argv[]) {
    if (argc < 1)
        exit(EXIT_FAILURE);

    const char* prog = argv[0];
    if (argc < 2) {
        usage(prog);
        exit(EXIT_FAILURE);
    }

    int target_h = -1, target_w = -1;
    int screen_percentage = 50;
    int raw_size = 0;
    int enhance_level = 2;

    SetColorFunc setColor = setTrueColor;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:p:re:8?")) != -1) {
        switch (opt) {
            case 'w':
                target_w = atoi(optarg);
                if (target_w < 0) {
                    fprintf(stderr, "Width cannot be negative\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                target_h = atoi(optarg);
                if (target_h < 0) {
                    fprintf(stderr, "Height cannot be negative\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                screen_percentage = atoi(optarg);
                if (screen_percentage < 0 || screen_percentage > 100) {
                    fprintf(stderr, "Percentage should be between 0 and 100\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'r':
                raw_size = 1;
                break;
            case '8':
                setColor = set256Color;
                break;
            case 'e':
                enhance_level = atoi(optarg);
                if (enhance_level < 0 || enhance_level > 2) {
                    fprintf(stderr,
                            "Enhance level should be between 0 and 2\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                if (optopt == 0) {
                    usage(prog);
                    exit(EXIT_SUCCESS);
                }
                fprintf(stderr, "More info with \"%s -?\"\n", prog);
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        const char* file_path = argv[i];
        int img_w, img_h;
        uint32_t* img =
            (uint32_t*)stbi_load(file_path, &img_w, &img_h, NULL, 4);
        if (!img) {
            fprintf(stderr, "Cannot open file %s\n", file_path);
            exit(EXIT_FAILURE);
        }

        int pixel_w, pixel_h;
        switch (enhance_level) {
            case 0:
                pixel_w = 1;
                pixel_h = 1;
                break;
            case 1:
                pixel_w = 1;
                pixel_h = 2;
                break;
            default:
                pixel_w = 4;
                pixel_h = 8;
        }

        uint32_t* pixels = img;
        uint32_t* resize = NULL;
        if (!raw_size) {
            int resize_h = 0, resize_w = 0;
            int screen_w = 120, screen_h = 30;
            getWindowSize(&screen_h, &screen_w);

            // For converting screen size to pixel size
            float mul_w, mul_h;
            switch (enhance_level) {
                case 0:
                    mul_w = 0.5f;
                    mul_h = 1.0f;
                    break;
                case 1:
                    mul_w = 1.0f;
                    mul_h = 2.0f;
                    break;
                default:
                    mul_w = 4.0f;
                    mul_h = 8.0f;
            }

            screen_w *= mul_w;
            screen_h *= mul_h;

            if (target_w == -1 && target_h == -1) {
                // Both not set, use screen size
                resize_h = screen_h * screen_percentage / 100.0f;
                resize_w = img_w * resize_h / img_h;
                if (resize_w > screen_w) {
                    resize_w = screen_w;
                    resize_h = img_h * resize_w / img_w;
                }
            } else {
                if (target_w != -1) {
                    resize_w = target_w ? target_w * mul_w : screen_w;
                    if (target_h == -1) {
                        resize_h = img_h * resize_w / img_w;
                    }
                }
                if (target_h != -1) {
                    resize_h = target_h ? target_h * mul_h : screen_h;
                    if (target_w == -1) {
                        resize_w = img_w * resize_h / img_h;
                    }
                }
            }

            resize = malloc(sizeof(uint32_t) * resize_w * resize_h);
            if (!resize) {
                fprintf(stderr, "Cannot allocate memory for resize image\n");
                exit(EXIT_FAILURE);
            }
            stbir_resize_uint8((uint8_t*)img, img_w, img_h,
                               sizeof(uint32_t) * img_w, (uint8_t*)resize,
                               resize_w, resize_h, sizeof(uint32_t) * resize_w,
                               4);
            img_w = resize_w;
            img_h = resize_h;
            pixels = resize;
        }

        // handle alpha
        for (int x = 0; x < img_h; x++) {
            for (int y = 0; y < img_w; y++) {
                Color* c = (Color*)&pixels[x * img_w + y];
                c->r *= c->a / 255.0f;
                c->g *= c->a / 255.0f;
                c->b *= c->a / 255.0f;
            }
        }

        for (int x = 0; x + pixel_h <= img_h; x += pixel_h) {
            for (int y = 0; y + pixel_w <= img_w; y += pixel_w) {
                switch (enhance_level) {
                    case 0:
                        setColor(pixels[x * img_w + y], 1);
                        printf("  ");
                        break;
                    case 1:
                        setColor(pixels[x * img_w + y], 1);
                        setColor(pixels[(x + 1) * img_w + y], 0);
                        printf("\u2584");
                        break;
                    default: {
                        Color block[8][4];
                        for (int x1 = 0; x1 < 8; x1++) {
                            for (int y1 = 0; y1 < 4; y1++) {
                                block[x1][y1].color =
                                    pixels[(x + x1) * img_w + (y + y1)];
                            }
                        }
                        printClosestShape(block, setColor);
                    }
                }
            }
            printf("\x1b[m\n");
        }

        free(resize);
        stbi_image_free(img);
    }
    exit(EXIT_SUCCESS);
}
