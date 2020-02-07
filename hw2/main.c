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

typedef enum room_type { START_ROOM, END_ROOM, MID_ROOM } room_type;

typedef struct Room Room;

struct Room {
    /* max room name per directions is 8 chars + null */
    char* name;
    //Room** connections;
    room_type type;

};

/* random_roooms
 * input: array of 10 strings
 * output: dynamically allocated array of 7 randomly selected strings
 *
 * Assumptions: Caller will free
 */

char** random_rooms(char* rooms[MAX_ROOMS]) {
    /* loop counter */
    int max = MAX_CHOSEN; /* choose 7 out 10 rooms */
    int count = 0;

    char** chosen = malloc(sizeof(char*) * MAX_ROOMS); /* output buffer */
    char** room_strings = rooms; /* for iteration */

    bool is_selected[MAX_ROOMS]; /* track selected via index */
    memset(is_selected, FALSE, sizeof(bool) * MAX_ROOMS);

    /* seed rng */
    srand((unsigned) time(NULL));

    while (count < max) {
        int index = rand() % MAX_ROOMS;
        /* if we haven't already picked this room
         * then add it to the chosen rooms
         * otherwise try again...
         * potentially non-halting, but not really in practice
         */
        if (!is_selected[index]) {
            /* pointer to current string */
            char** curr = room_strings + index;
            /* size of current string plus null character*/
            size_t memsize = sizeof(char) * (strlen(*curr) + 1);

            chosen[count] = malloc(memsize);
            strcpy(chosen[count], *curr);
            count++;

            /* don't pick the same name twice */
            is_selected[index] = TRUE;
        }
    }
    return chosen;
}


/**
 * create_room_map
 * input: array of 7 chosen rooms
 * output: array of room struct pointers that have been randomly ordered
 *
 * Creates START_ROOM and END_ROOM randomly
 * Adds random connections
 *
 * Assumptions: caller will free
 *
 */
Room** create_room_map(char** chosen) {
    /* loop counter */
    int max = MAX_CHOSEN; /* we chose 7 rooms */
    int count = 0;
    int i; /* used for room array malloc */

    /* allocate array of 7 room structs */
    Room** rooms = malloc(sizeof(Room*) * max);
    for (i = 0; i < max; i++) {
        rooms[i] = malloc(sizeof(Room));
    }

    bool is_initalized[MAX_ROOMS]; /* track selected */
    memset(is_initalized, FALSE, sizeof(bool) * MAX_ROOMS);

    /* seed rng */
    srand((unsigned) time(NULL));
    while (count < max) {
        /* note: index is used for tracking random strings
         * count is used for the index of the Room array */
        int index = rand() % max;

        /** set room type ***/
        /* if not initialized then initialize
         * similar to random room selection above */
        if (!is_initalized[index]) {
            /* set room type so first position is start_room
             * and end position is end_room */
           if (count == 0) {
                rooms[count]->type = START_ROOM;
            }
            else if (count == 6) {
                rooms[count]->type = END_ROOM;
            }
            else {
                rooms[count]->type = MID_ROOM;
            }

             /*** set name ***/
            /* pointer to randomly chosen name */
            char** curr_name = chosen + index;

            /* allocate space for name in struct */
            size_t memsize = sizeof(char) * (strlen(*curr_name) + 1);
            rooms[count]->name = malloc(memsize);
            strcpy(rooms[count]->name, *curr_name);


            /* increment counter, set index flag */
            count++;
            is_initalized[index] = TRUE;
        }

    }

    return rooms;
}

/*
 * room_type_to_string
 * input: Room struct (copy)
 * output: c-string
 * converts room type to string
 */
char* room_type_to_string(Room r) {
    room_type t = r.type;

    char* name;

    char start[] = "START_ROOM";
    char mid[] = "MID_ROOM";
    char end[] = "END_ROOM";

    switch (t) {
        case START_ROOM:
            name = malloc(sizeof(char) * (strlen(start) + 1));
            strcpy(name, start);
            break;

        case MID_ROOM:
            name = malloc(sizeof(char) * (strlen(mid) + 1));
            strcpy(name, mid);
            break;

        case END_ROOM:
            name = malloc(sizeof(char) * (strlen(end) + 1));
            strcpy(name, end);
            break;

        default:
            name = NULL;
    }

    return name;
}


/*
 * free_room_map()
 * input: an array of struct Room
 * output: nothing
 *
 * side effects: frees sets everything to null after free
 *
 * desc: Memory clean up function for room_struct array
 * to be called after make_room_map
 *
 */
void free_room_map(Room** room_map) {
    int i;
    for(i = 0; i < MAX_CHOSEN; i++) {
        free(room_map[i]);
        room_map[i] = NULL;
    }
    free(room_map);
    room_map = NULL;
}


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
    Room** room_map = create_room_map(chosen_strs);

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
        sprintf(room_filename, "%s/%s%s", room_dir, room_map[i]->name, filename_postfix);

        /* DEBUG */
        printf("%s\n", room_filename);

        /* open file
         * remember to close at end of scope */
        fptr = fopen(room_filename, "w");
        if (fptr == NULL) {
            /* print to stderr if problem */
            fprintf(stderr,"Failed to create %s :(\n", room_filename);
        } else {
            /* write name*/
            fprintf(fptr, "ROOM NAME: %s\n", room_map[i]->name);

            // TODO: loop connections and print

            /* write room type*/
            fprintf(fptr, "ROOM TYPE: %s\n", room_type_to_string(*room_map[i]));
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


