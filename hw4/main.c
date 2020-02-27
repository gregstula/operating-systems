/* Gregory D Stula
 * 2020-02-26
 * CS 344 Winter 2020
 * Program 3
 */
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LINE 2048
#define MAX_ARGS 512
#define ENOUGH_SPACE 256
// global state needed for signal handler functions
// true until SIGSTP signal is received
volatile bool global_allow_background = true;

// All memory will be owned by this struct for easy of use
// NOTE: smsh_ is a smallsh prefix acting as our namespace
typedef struct smsh_state {
    // true if shell is running
    bool is_running;
    // true if shell is in background
    bool send_to_background;
    bool ignore_all;
    // pointer to a line buffer
    char* line_buffer;
    // status string
    char* status_buffer;

    // input redirection
    bool should_redirect_input;
    // non-owning
    char* input_file;

    // output redirection
    bool should_redirect_output;
    // non-owning
    char* output_file;

    // array of strings for args
    char** args;
    size_t args_size;

    pid_t* background_procs;
    int* proc_count;

    // pid of self
    pid_t self;

} smsh_state;

// function: init_state
// initalizes a shell state struct
// requires sigaction information for sigint
// this in order to change it's behavior across processes
smsh_state smsh_init_state()
{
    smsh_state s;
    // run on init
    s.is_running = true;
    s.send_to_background = false;
    s.ignore_all = false;

    // point buffers to null
    s.line_buffer = NULL;
    s.status_buffer = NULL;
    // point to null
    s.args = NULL;
    // set array size to 0
    s.args_size = 0;

    s.should_redirect_input = false;
    s.input_file = NULL;

    s.should_redirect_output = false;
    s.output_file = NULL;

    // point background array to null
    s.background_procs = NULL;
    s.proc_count = 0;

    // set pid of the current shell
    s.self = getpid();

    // return by value
    return s;
}

// function: clean_state
// deinitializes and frees dynamically allocated members in the state struct
void smsh_clean_state(smsh_state* state)
{
    free(state->line_buffer);

    // if we transfered ownership, free
    if (state->input_file) {
        free(state->input_file);
        state->input_file = NULL;
    }

    // if we transfered ownership, free
    if (state->output_file) {
        free(state->output_file);
        state->output_file = NULL;
    }

    // Free args strings
    for (size_t i = 0; i < state->args_size; i++) {
        free(state->args[i]);
        state->args[i] = NULL;
    }
    // free args array
    free(state->args);
    state->args = NULL;


    // set bools to default
    state->send_to_background = false;
    state->ignore_all = false;
    state->should_redirect_input = false;
    state->should_redirect_output = false;
}


// function: get_input
// prints a prompt to the user to get a command line
// it removes the first newline character
// and adds the command line information to the shell state
void smsh_get_input(smsh_state* shell)
{
    // print the prompt
    const char* prompt = ": ";
    printf("%s", prompt);
    fflush(stdout);

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

// function find_next
// utility function that finds the distance of the next
// char from a string iterator
// largely inspired by how I *wanted* strcspn to work
size_t smsh_find_next(char* itr, char c)
{

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


// function: parse_input
// splits the command line string by spaces
// parses the resulting array to prepare for proper handling
// of built in functionality such as io redirection and PID status
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
    while (arg_start < len) {
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

    int truncate_to = 0;
    // parse built in symbols
    for (size_t i = 0; i < arg_count; i++) {
        // replace any instance of $$ with the shell's pid
        if (strcmp(shell->args[i],"$$") == 0) {
            shell->args[i] = realloc(shell->args[i], sizeof(char) * 7);
            sprintf(shell->args[i], "%d", shell->self);
        }

        if (strcmp(shell->args[i],"#") == 0) {
            shell->ignore_all = true;
        }

        // handle input redirection
        if (strcmp(shell->args[i],"<") == 0) {
            // words followed by < or < must be after all args
            if (!truncate_to) {
                truncate_to = i;
            }
            // bounds check
            if ((i + 1) < arg_count) {
                shell->should_redirect_input = true;
                // set to pointer
                // transfer ownership of file name
                shell->input_file = shell->args[i + 1];
                shell->args[i+1] = NULL;
                shell->args[i+1] = calloc(sizeof(char), 1);
            }
        }
        // handle output redirection
        else if (strcmp(shell->args[i],">") == 0) {
            // words followed by < or < must be after all args
            if (!truncate_to) {
                truncate_to = i;
            }

            // bounds check
            if ((i + 1) < arg_count) {
                shell->should_redirect_output = true;

                // set to pointer
                shell->output_file = shell->args[i + 1];
                // transfer ownership of file name
                shell->args[i+1] = NULL;
                // replace with temp mem to not deref null next i
                shell->args[i+1] = calloc(sizeof(char), 1);
            }
        }
    }

    // if the last char is an ampersand, remove it and set background flag
    int endex = arg_count - 1;
    if (strcmp(shell->args[endex], "&") == 0) {
        free(shell->args[endex]);
        shell->args[endex] = NULL;
        shell->args_size--;
        shell->send_to_background = true;
    }

    // truncate
    if (truncate_to) {
        for(size_t i = truncate_to; i < arg_count; i++) {
            free(shell->args[i]);
            shell->args[i] = NULL;
        }
        shell->args_size = truncate_to;
    }
}

void execute_external_command(smsh_state* shell, bool background)
{

    int child_status;
    char* command = shell->args[0];
    pid_t spawn_pid = fork();
    if (spawn_pid == -1) {
        fprintf(stderr, "ERROR: Hull breach!\n");
        fflush(stderr);
        exit(1);
    }

    if (spawn_pid == 0) { // child process
        // turn on ^C
        struct sigaction sigint_child = { 0 };
        sigfillset(&sigint_child.sa_mask);
        sigint_child.sa_flags = 0;
        sigaction(SIGINT, &sigint_child, NULL);
        if (background) {
            // if no redirect given for stdin
            freopen("/dev/null", "r", stdin);

            // if no redirect given for stdout
            freopen("/dev/null", "w", stdout);
        }

        // input redirection
        if (shell->should_redirect_input) {
            int input = open(shell->input_file, O_RDONLY);
            if (input == -1) {
                perror("cannot open file for input\n");
                exit(1);
            }
            int dup = dup2(input, STDIN_FILENO);
            if (dup == -1) {
                perror("input redirect failed\n");
                exit(1);
            }
            // close
            fcntl(input, F_SETFD, FD_CLOEXEC);
        }

        // output redirection
        if (shell->should_redirect_output) {
            int output = open(shell->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output == -1) {
                perror("cannot open file for output\n");
                exit(1);
            }
            int dup = dup2(output, STDOUT_FILENO);
            if (dup == -1) {
                perror("output redirect failed\n");
                exit(1);
            }
            // close
            fcntl(output, F_SETFD, FD_CLOEXEC);
        }

        // execute
        execvp(command, shell->args);

        // these lines execute if it did not go well
        fprintf(stderr, "%s: no such file or directory", command);
        fflush(stderr);
        exit(1);
    }
    else { // parent process
        if (shell->send_to_background && global_allow_background) {
            // save background process in array
            shell->background_procs[*shell->proc_count] = spawn_pid;
            *shell->proc_count += 1;
            // wait in background mode
            waitpid(spawn_pid, &child_status, WNOHANG);
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
            return;
        }

        // wait for the process
        waitpid(spawn_pid, &child_status, 0);
        if (WIFEXITED(child_status)) {
            sprintf(shell->status_buffer, "exit value %d\n", WEXITSTATUS(child_status));
        }
        // in the case it was termninated by a signal
        else {
            sprintf(shell->status_buffer, "terminated by signal %d\n", WTERMSIG(child_status));
        }
    }
}

void smsh_check_background(smsh_state* state)
{
    int procs = *state->proc_count;
    int status;
    for (int i = 0; i < procs; i++) {
        if (waitpid(state->background_procs[i], &status, WNOHANG) > 0) {
            if (WIFEXITED(status)) {
                fprintf(stdout, "exit value %d\n", WEXITSTATUS(status));
                fflush(stdout);
            }
            // in the case it was termninated by a signal
            else {
                fprintf(stdout,"terminated by signal %d\n", WTERMSIG(status));
                fflush(stdout);
            }
        }
    }
}

// smsh_process_command
// main command processing logic
// executes build in commands
// else delegates to execute command function
// sets the state for the status string
void smsh_process_command(smsh_state* shell)
{
    // handle comments
    if (shell->ignore_all) return;

    int result = 0;
    // first arg is the command
    char* command = shell->args[0];
    // exit command
    if (strcmp(command, "exit") == 0) {
        fprintf(stdout, "Exiting...\n");
        fflush(stdout);
        shell->is_running = false;
        return;
    }
    // cd command
    else if (strcmp(command, "cd") == 0) {
        if (shell->args_size == 1) {
            // cd by itself takes you home
            result = chdir(getenv("HOME"));
        }
        else {
            // cd to path
            result = chdir(shell->args[1]);
        }
    }
    // status command
    else if (strcmp(command, "status") == 0) {
        fprintf(stdout, "%s", shell->status_buffer);
    }
    // handle non built in command
    // the function sets the status buffer per case
    else {
        // check for background command
        if (shell->send_to_background && global_allow_background) {
            execute_external_command(shell, true);
            return;
        }
        else {
            execute_external_command(shell, false);
            return;
        }
    }
    sprintf(shell->status_buffer, "exit value %d\n", result);
}

// catch_SIGTSTP
// used as sig action handler
// toggles the global allow backgroup flag
// and prints to stdout with syscall
void catch_SIGTSTP(int signo)
{
    if (global_allow_background) {
        char* message = "Entering foreground-only mode (& is now ignored)\n";
        // can't use printf during signal handlers
        write(STDOUT_FILENO, message, 49); // can't use strlen either
        global_allow_background = false;
    }
    else {
        char* message = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 29);
        global_allow_background = true;
    }
}

// entry point
int main(void)
{
    // signal handler for SIGINT
    struct sigaction sigint_action = { 0 };
    sigint_action.sa_handler = SIG_IGN; // ignore interupt
    sigfillset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);

    // signal handler for SIGSTP
    struct sigaction sigtstp_action = { 0 };
    // set to handler function
    sigtstp_action.sa_handler = catch_SIGTSTP;
    sigfillset(&sigtstp_action.sa_mask);
    sigtstp_action.sa_flags = 0;
    sigaction(SIGTSTP, &sigtstp_action, NULL);


    // stores command line data
    smsh_state shell = smsh_init_state();

    // exception to ownership, main will own this array
    // for tracking background processes
    pid_t* procs = calloc(sizeof(pid_t), ENOUGH_SPACE);
    int proc_count = 0;

    // shell data
    shell.background_procs = procs;
    shell.proc_count = &proc_count;
    // alocate status buffer
    shell.status_buffer = calloc(sizeof(char), ENOUGH_SPACE);
    sprintf(shell.status_buffer, "exit value 0\n");

    // command loop
    while (shell.is_running) {
        smsh_get_input(&shell);
        smsh_parse_input(&shell);
        smsh_process_command(&shell);
        smsh_check_background(&shell);
        smsh_clean_state(&shell);
    }

    // free array of pid_ts
    free(procs);
    free(shell.status_buffer);
    return 0;
}

