#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: keygen keylength\n");
        exit(1);
    }

    int length = atoi(argv[1]);

    srand(time(NULL));

    for (int i = 0; i < length; i++) {
        int r = rand() % 27;

        if (r == 26)
            printf(" ");
        else
            printf("%c", 'A' + r);
    }

    printf("\n");

    return 0;
}