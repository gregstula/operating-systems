/* Gregory D Stula
 * 2020-02-26
 * CS 344 Winter 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// entry point
int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: keygen [keylength]\n");
        return 1;
    }

    int keylen = atoi(argv[1]); // convert arg to integer
    // buffer for key
    char buffer[keylen + 1]; // make room for null character
    // generate random sequence between [A-Z]
    int lowerbound = 65; // ascii value for capital A
    int upperbound = 90; // ascii value for capital Z

    srand(time(NULL)); // seed rng

    for (int i = 0; i < keylen; i++) {
        // aquire random ascii character
        buffer[i] = (char)(rand() % (upperbound - lowerbound + 1)) + lowerbound;
    }
    buffer[keylen] = '\0'; // apend null char
    fprintf(stdout, "%s\n", buffer); // print to stdout
    return 0;
}
