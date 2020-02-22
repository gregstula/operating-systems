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
    // point to null
    s.args = NULL;
    // set array size to 0
    s.args_size = 0;

    // return by value
    return s;
}

// deinitializes and frees dynamically allocated members in the state struct
void smsh_destroy_state(smsh_state* state)
{
    free(state->line_buffer);

    // Free args strings
    for (size_t i = 0; i < state->args_size; i++) {
        free(state->args[i]);
        state->args[i] = NULL;
    }
    // free args array
    free(state->args);
    state->args = NULL;
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

// find next
// utility function that finds the distance of the next
// char from a string iterator
// largely inspired by how I *wanted* strcspn to work
size_t smsh_find_next(char* itr, char c) {

    int distance = 0;
    // count the number of increments
    // it takes to find the char and return it
    while (*itr != '\0') {
        if (*itr == c) {
            return distance;
        }
        distance++;
        itr++;
    }
    // return the distance to null char othereise
    return distance;
}




void smsh_parse_input(smsh_state* shell)
{

    char* line = shell->line_buffer;
    size_t len = strlen(line) + 1;

    // allocate array for arg strings
    char** arg_arr = calloc(sizeof(char*), MAX_ARGS);

    // iterate through the line and search for a space
    size_t arg_start = 0;
    int arg_count = 0;
    // esentially we are splitting by a delimiter of space
    // and getting an array of strings to add to to the shell state struct
    while(arg_start < len) {
        // get distance of next space from the current position
        // this is how we delimit args
        size_t next_space = smsh_find_next(line + arg_start, ' ');

        // create buffer in memory for arg
        // calloc gives us null char for free
        char* buffer = calloc(sizeof(char), next_space + 1);
        for (size_t i = 0; i < next_space; i++) {
            // copy arg string to buffer
            // off set the begining of the line with the start of the next arg
            // then add the distance from the next space to it
            buffer[i] = *((line + arg_start) + i);
        }

        // set index to arg buffer memory
        arg_arr[arg_count] = buffer;
        // increment arg count
        arg_count++;

        // increment arg start to one position after the space we found
        arg_start += next_space + 1;
    }
    // set shell pointer as owner of array
    shell->args = arg_arr;
    // save number of args
    shell->args_size = arg_count;
}

// main command processing logic
void smsh_process_command(smsh_state* shell)
{
    // first arg is the command
    char* command = shell->args[0];

    // exit command
    if (strcmp(command, "exit") == 0) {
        shell->is_running = false;
        return;
    }
    // cd command
    else if (strcmp(command, "cd") == 0) {
        if (shell->args_size == 1) {
            // cd by itself takes you home
            chdir(getenv("HOME"));
        }
        else {
            chdir(shell->args[1]);
        }
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
        smsh_process_command(&shell);
        smsh_destroy_state(&shell);
    }
    return 0;
}

