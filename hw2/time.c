#include <stdio.h>
#include <time.h>

int main(void) {
    char time_string[256];
    time_t rawtime;
    struct tm *time_info;
    time(&rawtime);
    time_info = localtime(&rawtime);
    strftime(time_string, 256, "%I:%M%P, %A, %B %d, %Y",time_info);
    printf("%s\n",time_string);
    return 0;
}
