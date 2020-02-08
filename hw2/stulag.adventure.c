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


char* get_newest_dirname()
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

int main(void)
{
    char* dir_name = get_newest_dirname();

    DIR* dptr = opendir(dir_name);
    struct dirent* file_in_dir;
    FILE* fptr;

    char file_name[ENOUGH_SPACE];


    if (dptr != NULL) {
        while((file_in_dir = readdir(dptr)) != NULL) {
            char line_buffer[ENOUGH_SPACE];
            if(file_in_dir->d_type != DT_DIR) { /* looking for non-directories */
                sprintf(file_name, "%s/%s", dir_name, file_in_dir->d_name);
                printf("%s\n", file_name);
                fptr = fopen(file_name, "r+");
                if (fptr) {
                    /* Get the last line to know the room type */
                    while (fgets(line_buffer, sizeof(line_buffer), fptr) != NULL) continue;
                }
                printf("%s\n", line_buffer);
                break;
            }
        }
    }

    /* free dyn alloc strind */
    free(dir_name);
    return 0;
}
