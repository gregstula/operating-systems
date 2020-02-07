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
#define MAX_CHOSEN 8
typedef int bool;

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
    int i;
    for (i = 0; i < MAX_CHOSEN; i++) {
        printf("%s\n", strs[i]);
        free(strs[i]);
    }
    free(strs);
    return 0;
}
