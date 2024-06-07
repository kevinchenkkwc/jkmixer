/* File: console.c
 * ---------------
 *  Console implementation :)
 */
#include "console.h"
#include "gl.h"

// Add malloc, strings, printf
#include "malloc.h"
#include "strings.h"
#include "printf.h"

// module-level variables, you may add/change this struct as you see fit!
static struct {
    color_t bg_color, fg_color;
    int line_height;
    int nrows, ncols;
    int cursor_row, cursor_col;
    // Keep track of content
    char **contents;
} module;

// declare void functions
static void clear_contents(void);
static void draw_console(void);
static void process_char(char ch);

void console_init(int nrows, int ncols, color_t foreground, color_t background) {
    // Please use this amount of space between console rows
    const static int LINE_SPACING = 5;

    // Assign color, height, and spacing
    module.line_height = gl_get_char_height() + LINE_SPACING;
    module.fg_color = foreground;
    module.bg_color = background;

    // Assign number of rows and cursor
    module.nrows = nrows;
    module.ncols = ncols;
    module.cursor_row = 0;
    module.cursor_col = 0;

    // Allocate space for console contents
    module.contents = (char **)malloc(nrows * sizeof(char *));
    for (int i = 0; i < nrows; i++) {
        module.contents[i] = (char *)malloc(ncols * sizeof(char));
        memset(module.contents[i], ' ', ncols);
    }

    // Initialize the graphics library and clear the console
    gl_init(ncols * gl_get_char_width(), nrows * module.line_height, GL_DOUBLEBUFFER);

    // Call console_clear to clear
    console_clear();
}

void console_clear(void) {
    // Reset console
    clear_contents();
    module.cursor_row = 0;
    module.cursor_col = 0;
    draw_console();
}


int console_printf(const char *format, ...) {
    // Allocate space
    char buffer[1024];
    va_list args;
    va_start(args, format);
    int n = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Format output by calling process_char
    for (int i = 0; i < n; i++) {
        process_char(buffer[i]);
    }

    draw_console();
    return n;
}

// Helper to clear content
static void clear_contents(void) {
    for (int i = 0; i < module.nrows; i++) {
        memset(module.contents[i], ' ', module.ncols);
    }
}

// Draw onto console
static void draw_console(void) {
    gl_clear(module.bg_color);
    for (int row = 0; row < module.nrows; row++) {
        for (int col = 0; col < module.ncols; col++) {
            gl_draw_char(col * gl_get_char_width(), row * module.line_height, module.contents[row][col], module.fg_color);
        }
    }
    gl_swap_buffer();
}

// Helper to process inputs
static void process_char(char ch) {
    switch (ch) {
        // newline
        case '\n':
            module.cursor_row++;
            module.cursor_col = 0;
            break;
        // backspace
        case '\b':
            if (module.cursor_col > 0) {
                module.cursor_col--;
            } else if (module.cursor_row > 0) {
                module.cursor_row--;
                module.cursor_col = module.ncols - 1;
            }
            module.contents[module.cursor_row][module.cursor_col] = ' ';
            break;
        // form feed
        case '\f':
            console_clear();
            break;
        // Move cursor
        default:
            module.contents[module.cursor_row][module.cursor_col] = ch;
            module.cursor_col++;
            // Horizontal wrapping
            if (module.cursor_col >= module.ncols) {
                module.cursor_col = 0;
                module.cursor_row++;
            }
            break;
    }
    // New row if filled (Vertical scrolling)
    if (module.cursor_row >= module.nrows) {
        for (int i = 1; i < module.nrows; i++) {
            memcpy(module.contents[i - 1], module.contents[i], module.ncols);
        }
        memset(module.contents[module.nrows - 1], ' ', module.ncols);
        // move cursor
        module.cursor_row = module.nrows - 1;
    }
}
