#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* compile time constants */
#define MAX_ROOMS 10
#define MAX_CHOSEN 8
#define ENOUGH_SPACE 256

/* bools */
typedef enum bool { FALSE, TRUE} bool;

char** random_rooms(char* rooms[MAX_ROOMS]) {
    int max = MAX_CHOSEN; /* choose 7 out 10 rooms */
    int count = 0;
    char** chosen = malloc(sizeof(char*) * MAX_ROOMS); /* output buffer */
    char** room_strings = rooms; /* for iteration */

    bool is_selected[MAX_ROOMS]; /* track selected */
    memset(is_selected, FALSE, sizeof(bool) * MAX_ROOMS);

    /* seed rng */
    srand((unsigned) time(NULL));

    while (count < max) {
        int index = rand() % MAX_ROOMS;
        /* if we haven't already picked this room
         * then add it to the chosen rooms
         * otherwise try again...
         * potentially non-halting
         */
        if (!is_selected[index]) {
            /* pointer to current string */
            char** curr = room_strings + index;
            /* size of current string plus null character*/
            size_t memsize = sizeof(char) * (strlen(*curr) + 1);

            chosen[count] = malloc(memsize);
            strcpy(chosen[count], *curr);
            count++;
            is_selected[index] = TRUE;
        }
    }
    return chosen;
}

typedef enum room_type { START_ROOM, END_ROOM, MID_ROOM } room_type;

typedef struct room_struct {
    /* max room name per directions is 8 chars + null */
    char name[9];
    char** connections;
    room_type type;

} room_struct;


int main(void) {
    /* room names */
    char* rooms[MAX_ROOMS] = {"dungeon", "twisty", "stulas", "hellish", "bjarne", "dennis", "ascii", "unicode", "standard", "holy" };
    /* buffers */
    char room_dir[ENOUGH_SPACE];

    /* generate dir dame fro PID */
    char dir_prefix[] = "stulag.rooms";
    int pid =  getpid();

    /* file pointer */
    FILE* fptr;

    /* chosen rooms */
    /* must free strings and array*/
    char** chosen_strs = random_rooms(rooms);

    /* iterator and-or indexer*/
    int i;
    char room_filename[ENOUGH_SPACE];
    char* filename_postfix = "_room";

    /* clear array */
    memset(room_dir, '\0', sizeof(char) * ENOUGH_SPACE);

    sprintf(room_dir, "%s.%d", dir_prefix, pid);

    /* create rooms directory */
    if (mkdir(room_dir,0777) && errno != EEXIST) {
        fprintf(stderr,"Directory creation error\n");
        return -1;
    }

    /* create room files and free dynamically allocated string array */
    for (i = 0; i < MAX_CHOSEN; i++) {
        printf("%s\n", chosen_strs[i]);

        /* clear room_filename var*/
        memset(room_filename, '\0', sizeof(char) * ENOUGH_SPACE);

        /* create file name for each room*/
        sprintf(room_filename, "%s/%s%s", room_dir, chosen_strs[i], filename_postfix);

        /* DEBUG */
        printf("%s\n", room_filename);

        /* open file
         * remember to close at end of scope */
        fptr = fopen(room_filename, "w");
        if (fptr == NULL) {
            fprintf(stderr,"Failed to create %s :(\n", room_filename);
        } else {
            fprintf(fptr, "ROOM NAME: %s", chosen_strs[i]);
        }


        /* free string */
        free(chosen_strs[i]);
        /* close file */
        fclose(fptr);
    }
    /* free string array */
    free(chosen_strs);
    return 0;
}


