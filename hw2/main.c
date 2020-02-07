#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* c89 means old style comments and bool defs */
#define TRUE 1
#define FALSE 0
#define MAX_ROOMS 10
typedef int bool;

char** random_rooms(char* rooms[MAX_ROOMS]) {
    int max = 7; /* choose 7 out 10 rooms */
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
            chosen[count] = malloc(sizeof(char) * strlen(*(room_strings + index)));
            strcpy(chosen[count], *(room_strings + index));
            count++;
        }
    }
    return chosen;
}

int main(void) {
    /* create rooms directory */
    /* room names */
    char* rooms[10] = {"dungeon", "twisty", "stulas", "hellish", "bjarne", "dennis", "ascii", "unicode", "standard", "holy" };
    /* buffers */
    char room_dir[100];

    /* generate dir dame fro PID */
    char dir_prefix[] = "stulag.rooms";
    int pid = getpid();

    /* file pointer */
    //FILE* fptr;

    sprintf(room_dir, "%s.%d", dir_prefix, pid);

    /* debug */
    printf("%s", room_dir);

    if (mkdir(room_dir,0777) && errno != EEXIST) {
        fprintf(stderr,"Directory creation error\n");
        return -1;
    }

    char** strs = random_rooms(rooms);
    while (*strs) {
        printf("%s\n", *strs++);
    }

    return 0;
}
