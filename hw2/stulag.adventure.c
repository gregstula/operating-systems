/* Gregory D Stula
* Assignment 2
* 2020-02-07
* CS 344 Winter 2020
*
*/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* compile time constants */
#define MAX_ROOMS 10
#define MAX_CHOSEN 7
#define MAX_CONNECTIONS 6
#define ENOUGH_SPACE 256

/* bools */
typedef enum bool {
    FALSE,
    TRUE
} bool;

typedef enum room_type {
    START_ROOM,
    END_ROOM,
    MID_ROOM
} room_type;

/* ENDS THE GAME */
bool end_game = FALSE;

/* Process room data and print according to
 * assignment specs
 * returns the next room to be processed
 */
char* process_room(char* room_name)
{
    /* open the start room file */
    FILE* fptr = NULL;
    /* array of lines */
    char** lines = calloc(20, sizeof(char*));
    char line_buffer[ENOUGH_SPACE];
    int line_count = 0;
    int i;
    char** connections = calloc(10, sizeof(char*));
    int conn_count = 0;
    char tmp1[ENOUGH_SPACE]; /* not used */
    char tmp2[ENOUGH_SPACE]; /* not used */
    char rn[ENOUGH_SPACE]; /* room name */

    fptr = fopen(room_name, "r+");
    if (fptr) {
        /*get each line */
        while (fgets(line_buffer, sizeof(line_buffer), fptr) != NULL) {
            /* allocate for line_buff + nul char */
            lines[line_count] = malloc(sizeof(char) * ENOUGH_SPACE);
            strcpy(lines[line_count], line_buffer);
            line_count++;
        }
    }

    /* get name of room */
    /* it is first line */
    sscanf(lines[0], "%s %s %s\n", tmp1, tmp2, rn);
    /* print current location */
    printf("CURRENT LOCATION: %s\n", rn);
    /* Get connections */
    /* will be in between first and last lines */
    for (i = 1; i < line_count - 1; i++) {
        char throwa1[ENOUGH_SPACE]; /* not used */
        char throwa2[ENOUGH_SPACE]; /* not used */

        char conn_name[ENOUGH_SPACE];
        /* copy into connections */
        connections[conn_count] = malloc(sizeof(char) * ENOUGH_SPACE);
        /* last one is connection name */
        sscanf(lines[i], "%s %s %s\n", throwa1, throwa2, conn_name);
        /* store connextions in array */
        strcpy(connections[conn_count], conn_name);
        /*inc count for next loop size */
        conn_count++;
    }
    char ans[ENOUGH_SPACE];
    int valid = FALSE;
    while (1) {
        /* print connections */
        printf("POSSIBLE CONNECTIONS: ");
        for (i = 0; i < conn_count - 1; i++) {
            printf("%s, ", connections[i]);
        }
        /* last one is a special case no comma */
        printf("%s.\n", connections[i]);
        printf("WHERE TO? > ");

        scanf("%s", ans); /* get input */

        for (i = 0; i < conn_count; i++) {
            if (strcmp(ans, connections[i]) == 0) {
                /* validation */
                valid = TRUE;
                break;
            }
        }
        if (valid) {
            /* return the next room selected */
            char* next = malloc(sizeof(char) * ENOUGH_SPACE);
            strcpy(next, ans);
            return next;
        }
        /* err prompt */
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    }
}

/* get_newest_dirname()
* Adapted from:
* https://oregonstate.instructure.com/courses/1780106/pages/2-dot-4-manipulating-directories
*/
char* get_newest_dirname(void)
{
    int newest_dir_time = -1;
    char target_prefix[32] = "stulag.rooms.";
    char* newest_dir_name = malloc(sizeof(char) * ENOUGH_SPACE);
    memset(newest_dir_name, '\0', sizeof(char) * ENOUGH_SPACE);

    DIR* dir_to_check;
    struct dirent* file_in_dir;
    struct stat dir_attrs;

    dir_to_check = opendir(".");

    if (dir_to_check != NULL) {
        while ((file_in_dir = readdir(dir_to_check)) != NULL) {
            if (strstr(file_in_dir->d_name, target_prefix) != NULL) { /* does entry have prefix? */
                stat(file_in_dir->d_name, &dir_attrs);
            }

            if ((int)dir_attrs.st_mtime > newest_dir_time) {
                newest_dir_time = (int)dir_attrs.st_mtime;
                memset(newest_dir_name, '\0', sizeof(char) * ENOUGH_SPACE);
                strcpy(newest_dir_name, file_in_dir->d_name);
            }
        }
    }

    closedir(dir_to_check);
    return newest_dir_name;
}

int main(void)
{
    char* dir_name = get_newest_dirname();

    DIR* dptr = opendir(dir_name);
    struct dirent* file_in_dir;
    FILE* fptr = NULL;

    /* buffers */
    char file_name[ENOUGH_SPACE];
    char line_buffer[ENOUGH_SPACE];
    char start_file[ENOUGH_SPACE];
    char next_room[ENOUGH_SPACE];
    int i; // loop counter

    /* steps tracking */
    char** step_strs = malloc(sizeof(char*) * 1000);
    int steps = 0;

    /* First find the start room */
    if (dptr != NULL) {
       while ((file_in_dir = readdir(dptr)) != NULL) {
            if (file_in_dir->d_type != DT_DIR) { /* looking for non-directories */
                /* sprintf to concat directory name with the file name
                 * this is for fopen later */
                sprintf(file_name, "%s/%s", dir_name, file_in_dir->d_name);

                /* open file, remember to close at end of scope */
                fptr = fopen(file_name, "r+");

                if (fptr) {
                    /* Get the last line to know the room type */
                    while (fgets(line_buffer, sizeof(line_buffer), fptr) != NULL) continue;
                }
                /* compare last line to see if it is the start room */
                if (strcmp("ROOM TYPE: START_ROOM\n", line_buffer) == 0) {
                    /* save the start file name */
                    sprintf(start_file, "%s", file_name);
                }
                /* close file at end of scopt */
        //        fclose(fptr);
            }
        }
    }

    /* process inital room */
    strcpy(next_room, process_room(start_file));
    printf("%s\n", next_room);
    step_strs[steps] = malloc(sizeof(char) * 100);

    /* add string to steps tracker and inc steps count */
    strcpy(step_strs[steps],next_room);
    steps++;

    /* game loop */
    while (1) {
        /* formatting */
        puts("");
        puts("");
        /* first check if we won */
        /* open file, remember to close at end of scope */
        sprintf(file_name, "%s/%s_room", dir_name, next_room);
        fptr = fopen(file_name, "r+");

        if (fptr) {
            /* Get the last line to know the room type */
            while (fgets(line_buffer, sizeof(line_buffer), fptr) != NULL) continue;
        }
        /* compare last line to see if it is the END room */
        if (strcmp("ROOM TYPE: END_ROOM\n", line_buffer) == 0) {
            /* break out of loop if we won*/
            printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            printf("YOU TOOK %d STEPS. YOUR PATCH TO VICTORY WAS:\n", steps);
            for (i = 0; i < steps; i++) {
                puts(step_strs[i]);
            }
            break;
        }
        /* close file at end of scopt */
        // process next room
        strcpy(next_room, process_room(file_name));

        /* add string to steps tracker and inc steps count */
        step_strs[steps] = malloc(sizeof(char) * 100);
        strcpy(step_strs[steps],next_room);
        steps++;
    }

    /* close what we oppened */
    closedir(dptr);
    /* free dyn alloc string */
    free(dir_name);
    return 0;
}
