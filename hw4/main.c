#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 2048
#define MAX_ARGS 512
#define ENOUGH_SPACE 256

// All memory will be owned by this struct for easy of use
// smsh_ is a smallsh prefix acting as our namespace
typedef struct smsh_state {
    bool is_running;

    // pointer to a line buffer
    char* line_buffer;

    // array of strings
    char** args;
    size_t args_size;
} smsh_state;

// initalizes a shell state struct
smsh_state smsh_init_state(void)
{
    smsh_state s;
    // run on init
    s.is_running = true;
    // point to null
    s.line_buffer = NULL;
    // allocate array that can fit MAX args
    s.args = calloc(sizeof(char*), MAX_ARGS);
    // set array size to 0
    s.args_size = 0;

    // return by value
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
    // strcspn returns the position of the first occuring char
    size_t pos = strcspn(tmp_buffer, "\n");

    // set the newline to null char
    char* new_line = tmp_buffer + pos;
    *new_line = '\0';

    // add to the shell state
    shell->line_buffer = tmp_buffer;
}


void smsh_parse_input(smsh_state* shell)
{

    char* line = shell->line_buffer;
    size_t len = strlen(line);
    char* end = line + len;

    char* arg_start = line;
    int arg_count = 0;

    // iterate through the line and search for a space
    for (char* pos = line; pos != end; pos++ ) {
        // when we hit a space
        // add arg to the arg array
        if (*pos == ' ') {
            // get length of arg
            size_t arg_len = (len - strlen(pos)) + 1;

            // copy the argument to an area in memory
            char* tmp = calloc(sizeof(char), arg_len);
            for (size_t i = 0;  i < arg_len - 1; i++) {
                char* c = arg_start + i;
                tmp[i] = *c;
            }
            // assign the arg to it's index in the array
            shell->args[arg_count] = tmp;
            // new arg starts after the current space
            arg_start = pos + 1;
            arg_count++;
        }
    }
    // save number of args
    shell->args_size = arg_count;
}


// main command processing logic
void smsh_process_args(smsh_state* shell)
{
    char* input = shell->line_buffer;

    // exit command
    if (strcmp(input, "exit") == 0) {
        shell->is_running = false;
        return;
    }
    // cd command
    else if (strcmp(input, "cd") == 0) {
        //TODO
    } else {
        // print args
        for(int i = 0; i < shell->args_size; i++) {
            printf("%s\n", shell->args[i]);
        }
        printf("Arg count: %d\n", (int)shell->args_size);
    }
}

// entry point
int main(void)
{
    smsh_state shell = smsh_init_state();

    while (shell.is_running) {
        smsh_get_input(&shell);
        smsh_parse_input(&shell);
        smsh_process_args(&shell);
    }

    smsh_destroy_state(&shell);
    return 0;
}

