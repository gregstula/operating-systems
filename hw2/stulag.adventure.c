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
            if(strstr(file_in_dir->d_name, target_prefix) != NULL) { /* does entry have prefix? */
                stat(file_in_dir->d_name, &dir_attrs);
            }

            if ((int) dir_attrs.st_mtime > newest_dir_time) {
                newest_dir_time = (int)dir_attrs.st_mtime;
                memset(newest_dir_name, '\0', sizeof(char) * ENOUGH_SPACE);
                strcpy(newest_dir_name, file_in_dir->d_name);
            }
        }
    }

    closedir(dir_to_check);
    return newest_dir_name;
}


void process_room(char* room_name) {
    /* open the start room file */
    FILE* fptr = NULL;
    /* array of lines */
    char** lines = calloc(20, sizeof(char*));
    char line_buffer[ENOUGH_SPACE];
    int line_count = 0;

    fptr = fopen(room_name, "r+");
    if (fptr) {
    /* Get the last line to know the room type */
        while (fgets(line_buffer, sizeof(line_buffer), fptr) != NULL) {
            /* allocate for line_buff + nul char */
            lines[line_count] = malloc(sizeof(char) * ENOUGH_SPACE);
            strcpy(lines[line_count], line_buffer);
            line_count++;
        }
    }
    int i;
    /* Get connections */
    /* will be in between first and last lines */
    for (i =1 ; i < line_count - 1; i++) {
        printf("%s\n",lines[i]);
    }

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


    /* First find the start room */
    if (dptr != NULL) {
        while((file_in_dir = readdir(dptr)) != NULL) {
            if(file_in_dir->d_type != DT_DIR) { /* looking for non-directories */
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
                if(strcmp("ROOM TYPE: START_ROOM\n", line_buffer) == 0) {
                    /* save the start file name */
                    sprintf(start_file, "%s", file_name);
                }
                /* close file at end of scopt */
                fclose(fptr);
            }
        }
    }

    process_room(start_file);

   /* close what we oppened */
    closedir(dptr);
    /* free dyn alloc string */
    free(dir_name);
    return 0;
}
