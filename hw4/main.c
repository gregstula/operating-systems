#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 2048

// All memory will be owned by this struct for easy of use
// smsh_ is a smallsh prefix acting as our namespace
typedef struct smsh_state {
    bool is_running;
    char* line_buffer;
} smsh_state;

// initalizes a shell state struct
smsh_state smsh_init_state(void)
{
    smsh_state s;
    s.is_running = true;
    s.line_buffer = NULL;
    return s;
}

// deinitializes and frees members of a shell state struct
void smsh_destroy_state(smsh_state* state)
{
    // If not NULL free the buffers
    if (state->line_buffer != NULL) {
        free(state->line_buffer);
        state->line_buffer = NULL;
    }
}


// smsh_flush_print
// prints to stdout and flushes the output stream
void smsh_flush_print(const char* ostr)
{
    fprintf(stdout, "%s", ostr);
    fflush(stdout);
}


// smsh_get_input
// prints a prompt to the user to get a command line
// it removes the first newline character
// and adds the command line information to the shell state
void smsh_get_input(smsh_state* shell)
{
    // print the prompt
    const char* prompt = ": ";
    smsh_flush_print(prompt);

    // allocate the shell's line buffer
    char* tmp_buffer = calloc(sizeof(char), MAX_LINE);

    // get the input
    fgets(tmp_buffer, MAX_LINE, stdin);

    // remove the newline character
    // strcspn returns the position of the first occuring newline
    uint32_t pos = (uint32_t) strcspn(tmp_buffer, "\n");

    // set the newline to null char
    char* new_line = tmp_buffer + pos;
    *new_line = '\0';

    // add to the shell state
    shell->line_buffer = tmp_buffer;
}


void process_input(smsh_state* shell)
{
    char* input = shell->line_buffer;
    if (strcmp(input, "exit") == 0)
    {
        shell->is_running = false;
        return;
    }
}


// entry point
int main(void)
{
    smsh_state shell = smsh_init_state();

    while (shell.is_running) {
        smsh_get_input(&shell);
        process_input(&shell);
    }

    smsh_destroy_state(&shell);
    return 0;
}

