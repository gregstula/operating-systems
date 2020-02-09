/* Gregory D Stula
* Assignment 2
* 2020-02-07
* CS 344 Winter 2020
*
*/
#include <pthread.h>
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
#define SMALL_ARRAY 20
#define MAX_ARRAY 256

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

/* utility function because I could not verify that calloc
 * and memset zero is guranteed to = NULL
 */
void null_array(char** arr, size_t size)
{
    int i;
    for (i = 0; i < size; i++) {
        arr[i] = NULL;
    }
}

/* utility function to free array of pointers */
void free_array(char** arr, size_t size)
{
    /* free steps tracking array */
    int i;
    for (i=0; i < size; i++) {
        if(arr[i] == NULL) {
            break;
        }
        free(arr[i]);
        arr[i] = NULL;
    }
    free(arr);
}

/* Process room data and print according to
 * assignment specs
 * returns the next room to be processed
 */
char* process_room(char* room_name)
{
    /* open the start room file */
    FILE* fptr = NULL;
    char line_buffer[ENOUGH_SPACE];

    char tmp1[ENOUGH_SPACE]; /* not used */
    char tmp2[ENOUGH_SPACE]; /* not used */
    char rn[ENOUGH_SPACE]; /* room name */
    char ans[ENOUGH_SPACE];
    bool valid = FALSE;

    /* just a loop counter */
    int i;
    /* array of lines */
    char** lines = malloc(sizeof(char*) * SMALL_ARRAY);
    /* connections array */
    char** connections = malloc(sizeof(char*) * SMALL_ARRAY);
    /* array size counter */
    int line_count = 0;
    int conn_count = 0;

    /* null the pointers */
    null_array(lines, SMALL_ARRAY);
    null_array(connections, SMALL_ARRAY);

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
    else {
        fprintf(stderr, "File error with %s\n", room_name);
    }

    fclose(fptr); /* close file */

    /* get name of room */
    /* it is first line */
    sscanf(lines[0], "%s %s %s\n", tmp1, tmp2, rn);
    /* print current location */
    printf("CURRENT LOCATION: %s\n", rn);
    /* Get connections */
    /* will be in between first and last lines */
    for (i = 1; i < line_count - 1; i++) {

        char conn_name[ENOUGH_SPACE];
        /* copy into connections */
        connections[conn_count] = malloc(sizeof(char) * ENOUGH_SPACE);
        /* last one is connection name */
        sscanf(lines[i], "%s %s %s\n", tmp1, tmp2, conn_name);
        /* store connextions in array */
        strcpy(connections[conn_count], conn_name);
        /*inc count for next loop size */
        conn_count++;
    }

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

        /* Handle time command by letting main thread know it occured */
        /* main function is responsible for dealing with this
         * this function does no tracking, only processing */
        if (strcmp("time",ans) == 0) {
            /* caller must free */
            char* next = malloc(sizeof(char) * ENOUGH_SPACE);
            strcpy(next, ans);
            /* clean up */
            free_array(lines, SMALL_ARRAY);
            free_array(connections, SMALL_ARRAY);
            /* exit with time command */
            return next;
        }

        /* check if connection was calid */
        for (i = 0; i < conn_count; i++) {
            if (strcmp(ans, connections[i]) == 0) {
                /* validation */
                valid = TRUE;
                break;
            }
        }
        if (valid) {
            /* return the next room selected */
            /* caller must free */
            char* next = malloc(sizeof(char) * ENOUGH_SPACE);
            strcpy(next, ans);
            /* clean up */
            free_array(lines, SMALL_ARRAY);
            free_array(connections, SMALL_ARRAY);
            return next;
        }
        /* err prompt */
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    }
}

/* get_newest_dirname()
* Adapted from:
* https://oregonstate.instructure.com/courses/1780106/pages/2-dot-4-manipulating-directories
* caller must free
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

                if ((int)dir_attrs.st_mtime > newest_dir_time) {
                    newest_dir_time = (int)dir_attrs.st_mtime;
                    memset(newest_dir_name, '\0', sizeof(char) * ENOUGH_SPACE);
                    strcpy(newest_dir_name, file_in_dir->d_name);
                }
            }
        }
    }
    closedir(dir_to_check);
    return newest_dir_name;
}


/* * * * * * * * * *
 * GLOBAL MUTEX    *
 * * * * * * * * * */
pthread_mutex_t lock;

/* * * * * * * * * * * * * * * * * * * *
* get_time()                            *
* desc: Run in a seperate thread        *
* it writes the current time to a file  *
* * * * * * * * * * * * * * * * * * * * */
void* write_time(void* arg)
{
    FILE* fptr;
    /* buffer*/
    char time_string[256];
    /* time data */
    time_t rawtime;
    struct tm *time_info;

    /* unlock the mutex */
    pthread_mutex_lock(&lock);
    /* get newest directory and open it */
    fptr = fopen("currentTime.txt","w+");

    time(&rawtime);
    time_info = localtime(&rawtime);
    strftime(time_string, 256, "%I:%M%P, %A, %B %d, %Y",time_info);
    /* write the time */
    fprintf(fptr, "%s\n", time_string);
    /* close the file */
    fclose(fptr);
    /* unlock the mutex */
    pthread_mutex_unlock(&lock);
    return NULL;
}

/* * * * * * * * * * * * * * *
* main()
* Desc: entry point
*
* Responsible for:
* 1. Mutex and thread management
* 3. Finding the start room
* 3. running the game loop
* 4. Cleaning up
* * * * * * * * * * * * * * * * */
int main(void)
{
    /* directory management */
    struct dirent* file_in_dir;
    FILE* fptr = NULL;

    /* buffers */
    char file_name[ENOUGH_SPACE];
    char line_buffer[ENOUGH_SPACE];
    char start_file[ENOUGH_SPACE];
    char* next_room;
    char last_room[ENOUGH_SPACE];

    /* loop counter */
    int i;

    /* pthread */
    int result_code = 0;
    pthread_t time_thread;

    /* get newest directory and open it */
    char* dir_name = get_newest_dirname();
    DIR* dptr = opendir(dir_name);

    /* steps tracking */
    int steps = 0;
    char** step_strs = malloc(sizeof(char*) * MAX_ARRAY);

    /* null the array */
    null_array(step_strs, MAX_ARRAY);

    /* lock the mutex */
    pthread_mutex_lock(&lock);


    /* initialize the mutex */
    if (pthread_mutex_init(&lock, NULL) != 0) {
        /* OS error? */
        fprintf(stderr, "\nBasic mutex init has failed!\n Please try again\n");
        /* clean up */
        free_array(step_strs, MAX_ARRAY);
        closedir(dptr);
        return -1;
    }


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
                    /* compare last line to see if it is the start room */
                    if (strcmp("ROOM TYPE: START_ROOM\n", line_buffer) == 0) {
                        /* save the start file name */
                        sprintf(start_file, "%s", file_name);
                    }
                    /* close file at end of scopt */
                fclose(fptr);
                }
            }
        }
    }

    /* launch the time thread */
    result_code = pthread_create(&time_thread, NULL, &write_time, NULL);

    /* initialize the mutex */
    if (result_code != 0) {
        /* OS error? */
        fprintf(stderr, "\nInitial thread creation has failed!\n Please try again\n");
        /* clean up */
        free_array(step_strs, MAX_ARRAY);
        closedir(dptr);
        return -1;
    }

    /* process inital room */
    next_room = process_room(start_file);
    step_strs[steps] = malloc(sizeof(char) * 100);
    /* add string to steps tracker and inc steps count */
    strcpy(step_strs[steps], next_room);
    steps++;

    /* save last room */
    strcpy(last_room, next_room);

    /* game loop */
    while (1) {
        /* formatting */
        puts("");
        puts("");

        /* THREADED TIME WRITING */

        /*if the last input was time read the time file */
        if(strcmp("time",next_room) == 0) {
            /*unlock the mutex*/
            pthread_mutex_unlock(&lock);

            /* join the thread */
            pthread_join(time_thread, NULL);

            /* read time file */
            fptr = fopen("currentTime.txt","r+");
            if (fptr) {
                fgets(line_buffer, sizeof(line_buffer), fptr);
                printf("\n%s\n", line_buffer);
            }
            else  {
               fprintf(stderr, "ERROR READING TIME FILE :( \n");
            }

            /* relock the mutex */
            pthread_mutex_lock(&lock);

            /* relaunch the thread */
            result_code = pthread_create(&time_thread, NULL, &write_time, NULL);

            /* check for errors */
            if (result_code != 0) {
                /* OS error? */
                fprintf(stderr, "\nA thread creation has failed!\n Please try again\n");
                /* clean up */
                fclose(fptr);
                free_array(step_strs, MAX_ARRAY);
                free(next_room);
                closedir(dptr);
                return -1;
            }

            /* close file */
            fclose(fptr);
            /* free memory*/
            free(next_room);
            /* resore last location */
            strcpy(next_room, last_room);
        }



        /* first check if we won */
        /* open file, remember to close at end of scope */
        sprintf(file_name, "%s/%s_room", dir_name, next_room);
        /* close at end */
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
        fclose(fptr);

        /* free last room string */
        free(next_room);
        /* save last room */
        strcpy(last_room, next_room);
        /* process next room */
        next_room = process_room(file_name);

        /* add string to steps tracker and inc steps count */
        step_strs[steps] = malloc(sizeof(char) * 100);
        strcpy(step_strs[steps], next_room);
        steps++;
    }
    /* free end room string */
    free(next_room);
    /* close end room file */
    fclose(fptr);
    /* close directory  */
    closedir(dptr);
    /* free dyn alloc'd directory string */
    free(dir_name);
    /* free steps array */
    free_array(step_strs, MAX_ARRAY);
    /* destroy mutex */
    pthread_mutex_destroy(&lock);

   return 0;
}

