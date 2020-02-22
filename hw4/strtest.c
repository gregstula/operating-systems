#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t find_next(char* str, char c) {
    char* itr = str;

    int distance = 0;
    while (*itr != '\0') {
        if (*itr == c) {
            return distance;
        }
        distance++;
        itr++;
    }
    return distance;
}



int main (void)
{
    char* s = "this is a test  string I want to seee if it splits";
    size_t slen = strlen(s) + 1;


    size_t arg_start = 0;
    while (arg_start < slen) {
        size_t next_space = find_next(s + arg_start, ' ');
        char buff[50];
        for (int i = 0; i < next_space; i++) {
            buff[i] = *((s + arg_start) + i);
        }
        buff[next_space] = '\0';
        printf("%s, ", buff);
        arg_start += next_space + 1;
    }
    puts("");
}
