#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Wrong argu format\n");
        exit(EXIT_FAILURE);
    }
    char *temp;
    unsigned int judge_num = strtol(argv[1], &temp, 10);
    if (*temp != '\0') {
        fprintf(stderr, "Wrong argus type");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "judge : %d\n", judge_num);
    exit(EXIT_SUCCESS);
}
