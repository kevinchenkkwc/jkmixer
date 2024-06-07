/* File: keyboard.c
 * -----------------
 *  my keyboard implementation (modified for extension) (modified for test 23)
 */
#include "keyboard.h"
#include "ps2.h"

static ps2_device_t *dev;

void keyboard_init(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    dev = ps2_new(clock_gpio, data_gpio);
}

unsigned char keyboard_read_scancode(void) {
    return ps2_read(dev);
}

key_action_t keyboard_read_sequence(void) {
    key_action_t action;

    // Label bytes
    unsigned char first_byte, second_byte, third_byte;

    // Read the first scancode
    first_byte = keyboard_read_scancode();  

    if (first_byte == 0xE0) { 
        // Check for extended key prefix
        second_byte = keyboard_read_scancode();  
        
        // Read the next byte
        if (second_byte == 0xF0) {  
            // Read the third byte
            third_byte = keyboard_read_scancode(); 
            action.what = KEY_RELEASE;
            action.keycode = third_byte;

        // Extended key press
        } else {  
            action.what = KEY_PRESS;
            action.keycode = second_byte;
        }

        // Normal key release
    } else if (first_byte == 0xF0) {  
        // Read the next byte
        second_byte = keyboard_read_scancode();  
        action.what = KEY_RELEASE;
        action.keycode = second_byte;

        // Normal key press
    } else {  
        action.what = KEY_PRESS;
        action.keycode = first_byte;
    }
    return action;
}

// Variable to hold the current modifier state
static keyboard_modifiers_t current_modifiers = 0;

// keyboard_read_event implementation
key_event_t keyboard_read_event(void) {
    key_action_t action;
    key_event_t event;

    while (1) {
        action = keyboard_read_sequence();

        // Check if the action is a modifier key and update state
        switch (ps2_keys[action.keycode].ch) {
            // Shift
            case PS2_KEY_SHIFT:
                if (action.what == KEY_PRESS)
                    // Enable
                    current_modifiers |= KEYBOARD_MOD_SHIFT;
                else if (action.what == KEY_RELEASE)
                    // Disable
                    current_modifiers &= ~KEYBOARD_MOD_SHIFT;

                // Continue reading next action since this isn't a key event
                continue;  

            // Alt
            case PS2_KEY_ALT:
                if (action.what == KEY_PRESS)
                    // Enable
                    current_modifiers |= KEYBOARD_MOD_ALT;
                else if (action.what == KEY_RELEASE)
                    // Disable
                    current_modifiers &= ~KEYBOARD_MOD_ALT;
                continue;

            // Ctrl
            case PS2_KEY_CTRL:
                if (action.what == KEY_PRESS)
                    // Enable
                    current_modifiers |= KEYBOARD_MOD_CTRL;
                else if (action.what == KEY_RELEASE)
                    // Disable
                    current_modifiers &= ~KEYBOARD_MOD_CTRL;
                continue;

            // Caps
            case PS2_KEY_CAPS_LOCK:
                if (action.what == KEY_RELEASE)
                    // Toggle CAPS LOCK state
                    current_modifiers ^= KEYBOARD_MOD_CAPS_LOCK; 
                continue;

            default:
            // Does nothing for special keys

                /*
                // If it's not a modifier key, process as a regular key event
                if (ps2_keys[action.keycode].ch >= 0x90) {
                    // Special keys
                    continue;
                }
                */

                break;
        }

        // If we reach here, a non-modifier key action has occurred
        event.action = action;
        event.key = ps2_keys[action.keycode];
        event.modifiers = current_modifiers;

        // Handle repeat press events
        if (action.what == KEY_PRESS || action.what == KEY_RELEASE) {
            return event;
        }

    }
}

// Helper function to determine if a character is alphabetic
static int is_alpha(char ch) {
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

unsigned char keyboard_read_next(void) {
    key_event_t event;

    // read events until a key press event
    do {
        event = keyboard_read_event();
    } 

    // Ignore key releases
    while (event.action.what != KEY_PRESS);  

        // Handle Ctrl for extension
        // Check if Ctrl is pressed
    if (event.modifiers & KEYBOARD_MOD_CTRL) { 
        switch (event.key.ch) {
            case 'a':
            case 'A':
                // Ctrl-A
                return 0x01;  
            case 'e':
            case 'E':
                // Ctrl-E
                return 0x05;  
            case 'u':
            case 'U':
                // Ctrl-U
                return 0x15; 
            default:
                // Handle other keys normally
                break;  
        }
    }

    // Backspace key
    if (event.action.keycode == PS2_KEY_DELETE) {
        return '\b'; 
    }

    // Escape key
    if (event.action.keycode == PS2_KEY_ESC) {
        return 0x76; 
    }

    // Handle special keys with codes >= 0x90
    if (event.key.ch >= 0x90) {
        // Return special key code directly (e.g., F1-F12)
        return event.key.ch;  
    }

    // Apply modifiers  (Shift and Caps Lock logic)
    if (event.modifiers & (KEYBOARD_MOD_SHIFT | KEYBOARD_MOD_CAPS_LOCK)) {
        if (event.modifiers & KEYBOARD_MOD_SHIFT) {
            // Shift over Caps Lock
            return event.key.other_ch;

        } else if (is_alpha(event.key.ch) && (event.modifiers & KEYBOARD_MOD_CAPS_LOCK)) {
            // Caps Lock affects only alphabetic keys, so use is_alpha
            return event.key.other_ch;
        }
    }

    // Default to unmodified
    return event.key.ch;
}

