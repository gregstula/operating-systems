#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

int hw_random(int limit) {
    srand((unsigned) time(NULL));
     return rand() % limit;
}

int main(void) {
    /* create rooms directory */
    /* buffers */
    char room_dir[100];

    /* generate dir dame fro PID */
    char dir_prefix[] = "stulag.rooms";
    int pid = getpid();
    sprintf(room_dir, "%s.%d", dir_prefix, pid);

    /* debug */
    printf("%s", room_dir);

    if (mkdir(room_dir,0777) && errno != EEXIST) {
        fprintf(stderr,"Directory creation error\n");
        return -1;
    }

    return 0;
}
