/* File: gl.c
 * ----------
 *  gl implementation + draw_line + triangle (retest)
 */
#include "gl.h"
#include "font.h"

// Static global variables for gl
static int gl_width;
static int gl_height;
static int gl_depth;

void gl_init(int width, int height, gl_mode_t mode) {
    fb_init(width, height, mode);

    // make variables for gl
    gl_width = width;
    gl_height = height;
    gl_depth = fb_get_depth();
}

int gl_get_width(void) {
    return gl_width;
}

int gl_get_height(void) {
    return gl_height;
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b) {
    // color address (fixed to BGRA from ARGB)
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void gl_swap_buffer(void) {
    fb_swap_buffer();
}

void gl_clear(color_t c) {
    int nbytes = gl_width * gl_height * gl_depth;
    unsigned char *buffer = fb_get_draw_buffer();
    unsigned char *pixel = (unsigned char *)&c;

    for (int i = 0; i < nbytes; i += 4) {
        // blue
        buffer[i] = pixel[0];     
        // green
        buffer[i + 1] = pixel[1];
        // red
        buffer[i + 2] = pixel[2];
        // alpha
        buffer[i + 3] = pixel[3];
    }
}

void gl_draw_pixel(int x, int y, color_t c) {
    // within bounds
    if (x < 0 || x >= gl_width || y < 0 || y >= gl_height) {
        return;
    }

    unsigned char *buffer = (unsigned char *)fb_get_draw_buffer();
    int offset = (y * gl_width + x) * gl_depth;
    unsigned char *pixel = (unsigned char *)&c;

    // blue
    buffer[offset] = pixel[0];     
    // green
    buffer[offset + 1] = pixel[1];
    // red  
    buffer[offset + 2] = pixel[2];
    // alpha
    buffer[offset + 3] = pixel[3];
}

color_t gl_read_pixel(int x, int y) {
    // bounds check
    if (x < 0 || x >= gl_width || y < 0 || y >= gl_height) {
        return 0;
    }

    unsigned char *buffer = (unsigned char *)fb_get_draw_buffer();
    int offset = (y * gl_width + x) * gl_depth;
    unsigned char pixel[4];

    // blue
    pixel[0] = buffer[offset];     
    // green
    pixel[1] = buffer[offset + 1];
    // red
    pixel[2] = buffer[offset + 2];
    // alpha
    pixel[3] = buffer[offset + 3];

    return *(color_t *)pixel;
}

void gl_draw_rect(int x, int y, int w, int h, color_t c) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            // call pixel to discard invalid pixels
            gl_draw_pixel(x + i, y + j, c);
        }
    }
}

void gl_draw_char(int x, int y, char ch, color_t c) {
    // glyph parameters
    int glyph_width = font_get_glyph_width();
    int glyph_height = font_get_glyph_height();
    int glyph_size = font_get_glyph_size();
    unsigned char glyph[glyph_size];

    // check if there's no glyph
    if (!font_get_glyph(ch, glyph, glyph_size)) {
        return;
    }

    for (int i = 0; i < glyph_height; i++) {
        for (int j = 0; j < glyph_width; j++) {
            if (glyph[i * glyph_width + j] == 0xFF) { // On pixel
                gl_draw_pixel(x + j, y + i, c);
            }
        }
    }
}

void gl_draw_string(int x, int y, const char* str, color_t c) {
    int char_width = gl_get_char_width();

    while (*str) {
        gl_draw_char(x, y, *str++, c);
        x += char_width;
    }
}



int gl_get_char_height(void) {
    return font_get_glyph_height();
}

int gl_get_char_width(void) {
    return font_get_glyph_width();
}

// Helper functions
static int abs(int x) {
    return x < 0 ? -x : x;
}

static float round(float x) {
    return x >= 0.0f ? (int)(x + 0.5f) : (int)(x - 0.5f);
}

static float fmod(float x, float y) {
    return x - (int)(x / y) * y;
}

static int floor(float x) {
    return (int)x - (x < (int)x);
}


// Plot algorithm
// Takes in coordinates, brightess, and color for anti-aliasing
void plot(int x, int y, float brightness, color_t c) {
    // Check if in bounds
    if (x < 0 || x >= gl_get_width() || y < 0 || y >= gl_get_height()) {
        return;
    }

    // Read the current color at x, y
    color_t current_color = gl_read_pixel(x, y);
    unsigned char *current_pixel = (unsigned char *)&current_color;
    unsigned char *new_pixel = (unsigned char *)&c;
    unsigned char blended_pixel[4];

    // Blend the colors depending on brightness
    for (int i = 0; i < 4; i++) {
        blended_pixel[i] = (unsigned char)((1 - brightness) * current_pixel[i] + brightness * new_pixel[i]);
    }
    // Revert blended_color to color_t
    color_t blended_color = *(color_t *)blended_pixel;

    // Draw blended color
    gl_draw_pixel(x, y, blended_color);
}

// Draw line based on Xialin Wu's line algorithm
void gl_draw_line(int x0, int y0, int x1, int y1, color_t c) {
    // Check if it's steep
    int steep = abs(y1 - y0) > abs(x1 - x0);

    // If it's steep, switch the x and y coords for easier calculations
    if (steep) {
        int temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
    }

    // Drawing from the left to the right
    if (x0 > x1) {
        int temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    // Gradient calculation
    int dx = x1 - x0;
    int dy = y1 - y0;
    float gradient = dy / (float)dx;

    // First endpoint
    float xend = round(x0);
    float yend = y0 + gradient * (xend - x0);
    float xgap = 1.0 - fmod(x0 + 0.5, 1.0);
    int xpxl1 = xend;
    int ypxl1 = floor(yend);
    if (steep) {
        plot(ypxl1, xpxl1, (1.0 - fmod(yend, 1.0)) * xgap, c);
        plot(ypxl1 + 1, xpxl1, fmod(yend, 1.0) * xgap, c);
    } else {
        plot(xpxl1, ypxl1, (1.0 - fmod(yend, 1.0)) * xgap, c);
        plot(xpxl1, ypxl1 + 1, fmod(yend, 1.0) * xgap, c);
    }
    float intery = yend + gradient;

    // Second endpoint
    xend = round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fmod(x1 + 0.5, 1.0);
    int xpxl2 = xend;
    int ypxl2 = floor(yend);
    if (steep) {
        plot(ypxl2, xpxl2, (1.0 - fmod(yend, 1.0)) * xgap, c);
        plot(ypxl2 + 1, xpxl2, fmod(yend, 1.0) * xgap, c);
    } else {
        plot(xpxl2, ypxl2, (1.0 - fmod(yend, 1.0)) * xgap, c);
        plot(xpxl2, ypxl2 + 1, fmod(yend, 1.0) * xgap, c);
    }

    // Draw the whole line
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            plot(floor(intery), x, 1.0 - fmod(intery, 1.0), c);
            plot(floor(intery) + 1, x, fmod(intery, 1.0), c);
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            plot(x, floor(intery), 1.0 - fmod(intery, 1.0), c);
            plot(x, floor(intery) + 1, fmod(intery, 1.0), c);
            intery += gradient;
        }
    }
}

// Not filled in triangle
void gl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, color_t c) {
    // Just 3 lines lol
    gl_draw_line(x1, y1, x2, y2, c);
    gl_draw_line(x2, y2, x3, y3, c);
    gl_draw_line(x3, y3, x1, y1, c);
}


// Circle drawing using the Midpoint Circle Algorithm
void gl_draw_circle(int x0, int y0, int radius, color_t c) {
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while (x >= y) {
        gl_draw_pixel(x0 + x, y0 + y, c);
        gl_draw_pixel(x0 + y, y0 + x, c);
        gl_draw_pixel(x0 - y, y0 + x, c);
        gl_draw_pixel(x0 - x, y0 + y, c);
        gl_draw_pixel(x0 - x, y0 - y, c);
        gl_draw_pixel(x0 - y, y0 - x, c);
        gl_draw_pixel(x0 + y, y0 - x, c);
        gl_draw_pixel(x0 + x, y0 - y, c);
        y++;

        if (radiusError < 0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

