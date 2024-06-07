/*
 * UI Implementation
 */
#include "gpio.h"
#include "i2s.h"
#include "audio.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "gl.c"
#include "strings.h"
#include "keyboard.h"
#include "ps2_keys.h"

#define WIDTH 1280
#define HEIGHT 720

#define KNOB_RADIUS 50
#define NUM_KNOBS 5

// Output values
static struct {
    int level;
    int compression_threshold;
    int compression_ratio;
    bool backing_track_level; 
    int length_of_recording;
    bool reverb;
} config = {
    .level = 0,
    .compression_threshold = 20,
    .backing_track_level = false,
    .length_of_recording = 3,
    .reverb = false
};

// Next
void wait_for_spacebar() {
    while (1) {
        unsigned char key = keyboard_read_next();
        if (key == ' ') {
            break;
        }
    }
}

void welcome() {
    // Gray
    gl_clear(gl_color(0x30, 0x30, 0x30)); // create dark gray color 
    // Draw welcome text
    const char *welcome_text = "Welcome to the JK Audio mixer!";
    int text_x = WIDTH / 2 - (strlen(welcome_text) * 14) / 2; // Adjusted for character width
    gl_draw_string(text_x, 50, welcome_text, GL_WHITE); // white text

    // Draw press space text
    const char *press_space_text = "Press Space to Continue";
    int press_space_text_x = WIDTH / 2 - (strlen(press_space_text) * 14) / 2; // Adjusted for character width
    gl_draw_string(press_space_text_x, HEIGHT - 100, press_space_text, GL_WHITE); // white text

    // Center coordinates for the note head
    int note_head_x = WIDTH / 2 - 50; // Adjusted for better alignment
    int note_head_y = HEIGHT / 2;

    // Draw the first note head
    int note_head_radius = 30;
    gl_draw_circle(note_head_x, note_head_y, note_head_radius, GL_WHITE);

    // Draw the stem (vertical line)
    int stem_height = 100;
    gl_draw_line(note_head_x + note_head_radius, note_head_y, note_head_x + note_head_radius, note_head_y - stem_height, GL_WHITE);

    // Draw the second note head
    int second_note_head_x = note_head_x + 80; // Adjusted for better alignment
    gl_draw_circle(second_note_head_x, note_head_y, note_head_radius, GL_WHITE);

    // Draw the stem for the second note head
    gl_draw_line(second_note_head_x + note_head_radius, note_head_y, second_note_head_x + note_head_radius, note_head_y - stem_height, GL_WHITE);

    // Draw the beam connecting the two stems
    gl_draw_line(note_head_x + note_head_radius, note_head_y - stem_height, second_note_head_x + note_head_radius, note_head_y - stem_height, GL_WHITE);
}

void base() {
    // Background is dark gray
    gl_clear(gl_color(0x30, 0x30, 0x30)); // create dark gray color

    // Draw the mixer interface
    int knob_radius = 50;
    int num_knobs = 5;
    int spacing = WIDTH / (num_knobs + 1);

    // Labels for knobs
    const char *labels[] = {"Volume", "Compression", "Backing Track", "Length", "Echo"};

    for (int i = 1; i <= num_knobs; i++) {
        int knob_x = i * spacing;
        int knob_y = HEIGHT / 2;

        // Draw the knob circle
        gl_draw_circle(knob_x, knob_y, knob_radius, gl_color(0x80, 0x80, 0x80)); // light gray knobs

        // Draw the knob indicator
        gl_draw_line(knob_x, knob_y, knob_x, knob_y - knob_radius, GL_WHITE); // white indicator

        // Draw the label above the knob
        gl_draw_string(knob_x - (strlen(labels[i-1]) * 14) / 2, knob_y - knob_radius - 20, labels[i-1], GL_WHITE); // white text
    }

    // White rectangle in center of screen for additional interface elements
    gl_draw_rect(WIDTH / 2 - 150, HEIGHT / 2 + 100, 300, 50, GL_WHITE);

    // Draw press enter text
    const char *press_enter_text = "Press Insert to start recording";
    int press_enter_text_x = WIDTH / 2 - (strlen(press_enter_text) * 14) / 2; // Adjusted for character width
    gl_draw_string(press_enter_text_x, HEIGHT - 50, press_enter_text, GL_WHITE); // white text
}

void draw_knobs(int selected_knob) {
    // Background is dark gray
    gl_clear(gl_color(0x30, 0x30, 0x30)); // create dark gray color

    // Draw the mixer interface
    int spacing = WIDTH / (NUM_KNOBS + 1);

    // Labels for knobs
    const char *labels[] = {"Volume", "Compression", "Backing Track", "Length", "Echo"};

    for (int i = 1; i <= NUM_KNOBS; i++) {
        int knob_x = i * spacing;
        int knob_y = HEIGHT / 2;

        // Draw the knob circle
        gl_draw_circle(knob_x, knob_y, KNOB_RADIUS, gl_color(0x80, 0x80, 0x80)); // light gray knobs

        // Draw the knob indicator
        gl_draw_line(knob_x, knob_y, knob_x, knob_y - KNOB_RADIUS, GL_WHITE); // white indicator

        // Draw the label above the knob
        gl_draw_string(knob_x - (strlen(labels[i-1]) * 14) / 2, knob_y - KNOB_RADIUS - 20, labels[i-1], GL_WHITE); // white text

        // Draw the selection rectangle border if this knob is selected
        if (i - 1 == selected_knob) {
            int rect_x = knob_x - KNOB_RADIUS - 10;
            int rect_y = knob_y - KNOB_RADIUS - 24;
            int rect_width = 2 * KNOB_RADIUS + 20;
            int rect_height = 2 * KNOB_RADIUS + 28; // Increased height by 8 pixels

            gl_draw_line(rect_x, rect_y, rect_x + rect_width, rect_y, GL_WHITE); // Top border
            gl_draw_line(rect_x, rect_y + rect_height, rect_x + rect_width, rect_y + rect_height, GL_WHITE); // Bottom border
            gl_draw_line(rect_x, rect_y, rect_x, rect_y + rect_height, GL_WHITE); // Left border
            gl_draw_line(rect_x + rect_width, rect_y, rect_x + rect_width, rect_y + rect_height, GL_WHITE); // Right border
        }
    }

    // White rectangle in center of screen for additional interface elements
    gl_draw_rect(WIDTH / 2 - 150, HEIGHT / 2 + 100, 300, 50, GL_WHITE);

    // Draw press enter text
    const char *press_enter_text = "Press Insert to start recording";
    int press_enter_text_x = WIDTH / 2 - (strlen(press_enter_text) * 14) / 2; // Adjusted for character width
    gl_draw_string(press_enter_text_x, HEIGHT - 50, press_enter_text, GL_WHITE); // white text
}

void draw_value(int selected_knob) {
    int text_x = WIDTH / 2;
    int text_y = HEIGHT / 2 + 120;
    char value_str[50];

    switch (selected_knob) {
        case 0:
            if (config.level == -2) {
                snprintf(value_str, sizeof(value_str), "Level: 0.5");
            } else {
                snprintf(value_str, sizeof(value_str), "Level: %d", config.level);
            }
            gl_draw_string(text_x - (strlen(value_str) * 14) / 2, text_y, value_str, GL_BLACK);
            break;
        case 1:
            snprintf(value_str, sizeof(value_str), "Threshold: %d", config.compression_threshold);
            gl_draw_string(text_x - (strlen(value_str) * 14) / 2, text_y, value_str, GL_BLACK);
            break;
        case 2:
            snprintf(value_str, sizeof(value_str), "Backing Track: %s", config.backing_track_level ? "On" : "Off");
            gl_draw_string(text_x - (strlen(value_str) * 14) / 2, text_y, value_str, GL_BLACK);
            break;
        case 3:
            snprintf(value_str, sizeof(value_str), "Length: %d seconds", config.length_of_recording);
            gl_draw_string(text_x - (strlen(value_str) * 14) / 2, text_y, value_str, GL_BLACK);
            break;
        case 4:
            snprintf(value_str, sizeof(value_str), "Reverb: %s", config.reverb ? "Yes" : "No");
            gl_draw_string(text_x - (strlen(value_str) * 14) / 2, text_y, value_str, GL_BLACK);
            break;
    }
}

void instructions(const char* text) {
    const int CHAR_WIDTH = 14;  // Width of a single character
    const int LINE_HEIGHT = 20; // Height between lines of text

    int max_chars_per_line = WIDTH / CHAR_WIDTH;
    int text_length = strlen(text);
    int line = 0;

    for (int start_index = 0; start_index < text_length; start_index += max_chars_per_line) {
        int end_index = start_index + max_chars_per_line;

        if (end_index > text_length) {
            end_index = text_length;
        }

        // Calculate the length of the current line
        int line_length = end_index - start_index;

        // Calculate the position to start drawing the current line
        int text_x = WIDTH / 2 - (line_length * CHAR_WIDTH) / 2;
        int text_y = 10 + line * LINE_HEIGHT;

        // Draw each character in the current line
        for (int i = start_index; i < end_index; i++) {
            gl_draw_char(text_x, text_y, text[i], GL_WHITE);
            text_x += CHAR_WIDTH;
        }

        // Move to the next line
        line++;
    }
}

void next() {
    gl_swap_buffer();
    wait_for_spacebar();
}

void move_selection(int *selected_knob, int direction) {
    *selected_knob += direction;
    if (*selected_knob < 0) {
        *selected_knob = NUM_KNOBS - 1;
    } else if (*selected_knob >= NUM_KNOBS) {
        *selected_knob = 0;
    }
}

void adjust_value(int selected_knob, int direction) {
    switch (selected_knob) {
        case 0: // Level
            if (direction > 0) {
                if (config.level == -2) config.level = 0;
                else if (config.level == 0) config.level = 1;
                else if (config.level == 1) config.level = 2;
                else if (config.level == 2) config.level = 4;
                else if (config.level == 4) config.level = 8;
            } else {
                if (config.level == 8) config.level = 4;
                else if (config.level == 4) config.level = 2;
                else if (config.level == 2) config.level = 1;
                else if (config.level == 1) config.level = 0;
                else if (config.level == 0) config.level = -2;
            }
            break;
        case 1: // Compression Threshold
            if (direction > 0 && config.compression_threshold < 20) config.compression_threshold += 5;
            else if (direction < 0 && config.compression_threshold > 0) config.compression_threshold -= 5;
            break;
        case 2: // Backing Track
            if (direction != 0) config.backing_track_level = !config.backing_track_level;
            break;
        case 3: // Length
            if (direction > 0 && config.length_of_recording < 10) config.length_of_recording += 1;
            else if (direction < 0 && config.length_of_recording > 1) config.length_of_recording -= 1;
            break;
        case 4: // Reverb
            config.reverb = !config.reverb;
            break;
    }
}

void print_config_values() {
    printf("Current configuration values:\n");
    printf("Level: %d\n", config.level);
    printf("Compression Threshold: %d\n", config.compression_threshold);
    printf("Backing Track Level: %d\n", config.backing_track_level);
    printf("Length of Recording: %d seconds\n", config.length_of_recording);
    printf("Reverb: %d\n", config.reverb);
}

void run(void) {
    // Initialize
    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    uart_init();

    // Welcome
    welcome();
    next();

    // Instructions
    base();
    instructions("Hello! This is the JK Mixer! This is how everything works!");
    next();
    base();
    instructions("Below we have 5 knobs, that control Volume, Compression, Backing Track, Recording Length,  and Reverb!");
    next();
    base();
    instructions("Use the left and right arrows to move along each control, and use the up and down arrows to change the values!");
    next();
    base();
    instructions("Once you're done, press Insert to begin your recording!");
    next();
    base();
    instructions("We hope you enjoy :)");
    next();

    int selected_knob = 0;

    while (1) {
        draw_knobs(selected_knob);
        draw_value(selected_knob);
        instructions("Use the arrow keys to select different knobs. Use up/down to change values.");
        gl_swap_buffer();

        unsigned char key = keyboard_read_next();
        if (key == PS2_KEY_ARROW_RIGHT) {
            move_selection(&selected_knob, 1);
        } else if (key == PS2_KEY_ARROW_LEFT) {
            move_selection(&selected_knob, -1);
        } else if (key == PS2_KEY_ARROW_UP) {
            adjust_value(selected_knob, 1);
        } else if (key == PS2_KEY_ARROW_DOWN) {
            adjust_value(selected_knob, -1);
        } else if (key == PS2_KEY_INSERT) {
            print_config_values();
            return;
        }
    }
}

