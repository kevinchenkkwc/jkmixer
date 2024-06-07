/* File: shell.c
 * -------------
 *  My code for my shell extension implementation + fix test 33
 */
#include "shell.h"
#include "shell_commands.h"
#include "uart.h"

// Include malloc, strings, and mango
#include "malloc.h"
#include "strings.h"
#include "mango.h"
#include "ps2_keys.h"

// Line length
#define LINE_LEN 256

// Module-level global variables for shell
static struct {
    input_fn_t shell_read;
    formatted_fn_t shell_printf;
} module;

// Declare a global variable command number
static int cmd_number = 1;

// NOTE TO STUDENTS:
// Your shell commands output various information and respond to user
// error with helpful messages. The specific wording and format of
// these messages would not generally be of great importance, but
// in order to streamline grading, we ask that you aim to match the
// output of the reference version.
//
// The behavior of the shell commands is documented in "shell_commands.h"
// https://cs107e.github.io/header#shell_commands
// The header file gives example output and error messages for all
// commands of the reference shell. Please match this wording and format.
//
// Your graders thank you in advance for taking this care!

// Added reboot, peek, poke, and history

int cmd_history(int argc, const char *argv[]);

static const command_t commands[] = {
    {"help", "help [cmd]", "print command usage and description", cmd_help},
    {"echo", "echo [args]", "print arguments", cmd_echo},
    {"clear", "clear", "clear screen (if your terminal supports it)", cmd_clear},
    {"reboot", "reboot", "reboot the Mango Pi", cmd_reboot},
    {"peek", "peek [addr]", "print contents of memory at address", cmd_peek},
    {"poke", "poke [addr] [val]", "store value into memory at address", cmd_poke},
    {"history", "history", "lists the history of commands entered", cmd_history},
};

// How much history to give
#define HISTORY_SIZE 10

// Declare history variables
static char *history[HISTORY_SIZE];
static int history_count = 0;

// Current position in history for display/edit
static int history_pos = -1; 

// Custom string copy function
void my_strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    // Null-terminate destination
    *dst = '\0';  
}

// Custom memory set function
void my_memset(void *s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
}

// Add to history function
void add_to_history(const char *command) {
    if (history_count < HISTORY_SIZE) {
        // Allocate memory for the new command
        history[history_count] = malloc(strlen(command) + 1); 
        my_strcpy(history[history_count++], command);
    } else {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[HISTORY_SIZE - 1] = malloc(strlen(command) + 1);
        my_strcpy(history[HISTORY_SIZE - 1], command);
    }
}

// Echo implementation (modified to help lines)
int cmd_echo(int argc, const char *argv[]) {
    for (int i = 1; i < argc; ++i)
        module.shell_printf("%s ", argv[i]);
    module.shell_printf("\n");
    return 0;
}


// History implementation
int cmd_history(int argc, const char *argv[]) {
    for (int i = 0; i < history_count; ++i) {
        module.shell_printf("%d %s\n", i + 1, history[i]);
    }
    return 0;
}

// Help implementation
int cmd_help(int argc, const char *argv[]) {
    // No command specified, print all commands
    if (argc == 1) { 
        for (int i = 0; i < sizeof(commands) / sizeof(command_t); i++) {
            // Print command and description
            module.shell_printf("%s\t\t%s\n", commands[i].name, commands[i].description);
        }
    // Command specified, print help for that command
    } else { 
        for (int i = 0; i < sizeof(commands) / sizeof(command_t); i++) {
            if (strcmp(commands[i].name, argv[1]) == 0) {
                // Just print help for that command
                // 3 extra spaces
                module.shell_printf("   %s\t\t    %s\n", commands[i].usage, commands[i].description);
                return 0;
            }
        }
        // Command doesn't exist
        module.shell_printf("error: no such command '%s'\n", argv[1]);
        return 1;
    }
    return 0;
}

// Reboot implentation
int cmd_reboot(int argc, const char *argv[]) {
    // Text indication
    module.shell_printf("Rebooting...\n");
    // Calls from mango.h
    mango_reboot();
    return 0;
}

// Peek implementation
int cmd_peek(int argc, const char *argv[]) {
    // Error if not 1 arguments
    if (argc != 2) {
        module.shell_printf("error: peek expects 1 argument [addr]\n");
        return 1;
    }
    // Initialize address
    unsigned long address = 0; 
    const char *endptr;
    address = strtonum(argv[1], &endptr);
    // If argument cannot be converted
    if (*endptr != '\0') {
        module.shell_printf("error: peek cannot convert '%s'\n", argv[1]);
        return 1;
    }
    // Must be a multiple of 4
    if (address % 4 != 0) {
        module.shell_printf("error: peek address must be 4-byte aligned\n");
        return 1;
    }
    // Prints value
    unsigned int value = *((unsigned int*)address);
    module.shell_printf("0x%08lx:   %08x\n", address, value);
    return 0;
}

// Poke implementation
int cmd_poke(int argc, const char *argv[]) {
    // 2 Arguments
    if (argc != 3) {
        module.shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
        return 1;
    }
    // Initialize variables
    unsigned long address = 0, value = 0; 
    const char *endptr;

    address = strtonum(argv[1], &endptr);
    // Cannot convert address (invalid)
    if (*endptr != '\0') {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[1]);
        return 1;
    }

    value = strtonum(argv[2], &endptr);
    // Cannot convert value
    if (*endptr != '\0') {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[2]);
        return 1;
    }

    // Address must be 4-byte aligned
    if (address % 4 != 0) {
        module.shell_printf("error: poke address must be 4-byte aligned\n");
        return 1;
    }

    *((unsigned int*)address) = (unsigned int)value;
    return 0;
}

// Clear
int cmd_clear(int argc, const char* argv[]) {
    //const char *ANSI_CLEAR = "\033[2J"; // if your terminal does not handle formfeed, can try this alternative?

    module.shell_printf("\f");   // minicom will correctly respond to formfeed character
    return 0;
}

void shell_init(input_fn_t read_fn, formatted_fn_t print_fn) {
    module.shell_read = read_fn;
    module.shell_printf = print_fn;
}

void shell_bell(void) {
    uart_putchar('\a');
}


void shell_readline(char buf[], size_t bufsize) {
    // Number of characters currently in the buffer
    size_t count = 0;  

    // Current cursor position within the buffer
    size_t cursor = 0; 
    unsigned char ch;

    // Clear the buffer
    my_memset(buf, 0, bufsize);

    // Function to refresh the display line
    void refresh_line() {
        // Clear line and redraw prompt and buffer
        module.shell_printf("\r\033[K[%d] Pi> %s", cmd_number, buf);

        // Reposition cursor to the beginning
        for (size_t i = strlen(buf); i > cursor; i--) {
            module.shell_printf("\b");
        }
    }

    // Initial draw of the prompt
    refresh_line();  

    while (1) {
        // Read one character from input
        ch = module.shell_read(); 

        // Enter key, execute command
        if (ch == '\n') { 

            // Null-terminate the string
            buf[count] = '\0';  
            module.shell_printf("\r\n");
            return;

        // Handle backspace
        } else if (ch == '\b') {  
            if (cursor > 0) {
                for (size_t i = cursor - 1; i < count - 1; i++) {
                    buf[i] = buf[i + 1];
                }
                count--;
                cursor--;
                buf[count] = '\0'; // Maintain null termination
                refresh_line();
            } else {
                shell_bell();

            }       // Ctrl functionality
        } else if (ch == 0x01) { // Ctrl-A
            cursor = 0;
            refresh_line();

        } else if (ch == 0x05) { // Ctrl-E
            cursor = count;
            refresh_line();

        } else if (ch == 0x15) { // Ctrl-U
            my_memset(buf, 0, bufsize);
            count = cursor = 0;
            refresh_line();

        // Move cursor
        } else if (ch == PS2_KEY_ARROW_LEFT) { // Left arrow
            if (cursor > 0) {
                cursor--;
                module.shell_printf("\b");
            }

        } else if (ch == PS2_KEY_ARROW_RIGHT) { // Right arrow
            if (cursor < count) {
                module.shell_printf("%c", buf[cursor]);
                cursor++;
            }


        // Up arrow for history navigation
        } else if (ch == PS2_KEY_ARROW_UP) { 
            if (history_pos < history_count - 1) {
                history_pos++;
                
                // Fetch older history
                my_strcpy(buf, history[history_count - 1 - history_pos]);  
                count = cursor = strlen(buf);
                refresh_line();

            // Already at oldest history item
            } else {
                shell_bell(); 
            }

        // Down arrow for history navigation
        } else if (ch == PS2_KEY_ARROW_DOWN) { 
            if (history_pos > 0) {
                history_pos--;

                // Fetch newer history
                my_strcpy(buf, history[history_count - 1 - history_pos]); 
                count = cursor = strlen(buf);
                refresh_line();
            } else if (history_pos == 0) {
                
                // Return to editing new line
                history_pos = -1;
                my_memset(buf, 0, bufsize);
                count = cursor = 0;
                refresh_line();

            // Already at newest item
            } else {
                shell_bell();
            }
            
        // Printable characters
        } else if (ch >= 0x20 && ch <= 0x7E && count < bufsize - 1) { 
            for (size_t i = count; i > cursor; i--) {
                buf[i] = buf[i - 1];
            }
            buf[cursor] = ch;
            count++;
            cursor++;
            refresh_line();

        // Invalid input
        } else {
            shell_bell(); 
        }
    }
}


// Helper function to find the next token in a string
static char *find_next_token(char *line, char **next) {
    char *start = line;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n'))
        // Skip leading whitespace
        start++;  

    if (!*start) {
        *next = NULL;
        // No more tokens
        return NULL;  
    }

    char *end = start;
    while (*end && *end != ' ' && *end != '\t' && *end != '\n')
        // Move to the end of the token
        end++;  

    // Set next start point
    *next = *end ? end + 1 : NULL;  
    // Null terminate the current token
    *end = '\0';  
    return start;
}

// Helpter function for strncpy
void *my_strncpy(char *dst, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    for ( ; i < n; i++) {
        dst[i] = '\0';
    }
    return dst;
}


int shell_evaluate(const char *line) {
    // Make sure this buffer is large enough
    char buf[256];  
    my_strncpy(buf, line, sizeof(buf));

    // Ensure null termination
    buf[sizeof(buf) - 1] = '\0';  

    char *tokens[50];
    int argc = 0;
    char *next = buf;

    while (argc < 50 && next) {
        char *token = find_next_token(next, &next);
        if (token) {
            tokens[argc++] = token;
        }
    }

    if (argc == 0) {
        // No command
        return -1;
    }

    // Find command and execute
    for (int i = 0; i < sizeof(commands) / sizeof(command_t); i++) {
        if (strcmp(commands[i].name, tokens[0]) == 0) {
            return commands[i].fn(argc, (const char **)tokens);
        }
    }

    module.shell_printf("error: no such command '%s'\n", tokens[0]);
    // Command not found
    return 1;
}


// Modified shell_run for command number tracking
void shell_run(void) {
    module.shell_printf("Welcome \n");
    while (1) {
        char line[LINE_LEN];

        // Display the current command number
        //module.shell_printf("[%d] Pi> ", cmd_number); 
        
        // Read a line of input
        shell_readline(line, sizeof(line)); 
        
        // Only add non-empty lines to history
        if (strlen(line) > 0) {  
            add_to_history(line);
            
            // Evaluate the command
            shell_evaluate(line); 
            
            // Increment after the command is processed
            cmd_number++;  
        }
    }

}
