/* File: fb.c
 * ----------
 *  Framebuffer implementation
 */
#include "fb.h"
#include "de.h"
#include "hdmi.h"
#include "malloc.h"
#include "strings.h"

// module-level variables, you may add/change this struct as you see fit
static struct { 
    int width;             // count of horizontal pixels
    int height;            // count of vertical pixels
    int depth;             // num bytes per pixel

    fb_mode_t mode;        // buffering mode

    void *framebuffer[2];  // address of framebuffer memory

    int active_buffer;     // index of active buffer
} module;

void fb_init(int width, int height, fb_mode_t mode) {
    // Free previously allocated memory
    if (module.framebuffer[0] != NULL) {
        free(module.framebuffer[0]);
        module.framebuffer[0] = NULL;
    }
    if (module.framebuffer[1] != NULL) {
        free(module.framebuffer[1]);
        module.framebuffer[1] = NULL;
    }

    module.width = width;
    module.height = height;
    module.depth = 4;
    module.mode = mode;
    module.active_buffer = 0;
    int nbytes = module.width * module.height * module.depth;

    // Allocate memory for framebuffers
    module.framebuffer[0] = malloc(nbytes);

    if (!module.framebuffer[0]) {
        // Handle allocation failure
        return;
    }

    memset(module.framebuffer[0], 0x0, nbytes);

    // doublebuffer mode
    if (mode == FB_DOUBLEBUFFER) {
        module.framebuffer[1] = malloc(nbytes);
        if (!module.framebuffer[1]) {
            // If allocation fails
            free(module.framebuffer[0]);
            module.framebuffer[0] = NULL;
            return;
        }
        memset(module.framebuffer[1], 0x0, nbytes);
    }

    hdmi_resolution_id_t id = hdmi_best_match(width, height);
    hdmi_init(id);
    de_init(width, height, hdmi_get_screen_width(), hdmi_get_screen_height());
    de_set_active_framebuffer(module.framebuffer[module.active_buffer]);
}

int fb_get_width(void) {
    return module.width;
}

int fb_get_height(void) {
    return module.height;
}

int fb_get_depth(void) {
    return module.depth;
}

void* fb_get_draw_buffer(void) {
    if (module.mode == FB_DOUBLEBUFFER) {
        // return address if double
        return module.framebuffer[1 - module.active_buffer];
    }
    return module.framebuffer[0];
}

void fb_swap_buffer(void) {
    if (module.mode == FB_DOUBLEBUFFER) {
        module.active_buffer = 1 - module.active_buffer;
        de_set_active_framebuffer(module.framebuffer[module.active_buffer]);
    }
}
