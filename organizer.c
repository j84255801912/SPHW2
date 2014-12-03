#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MIN_JUDGE_NUM 1
#define MAX_JUDGE_NUM 12
#define MIN_PLAYER_NUM 8
#define MAX_PLAYER_NUM 16
#define MAX_BUFFER_SIZE 10000

typedef struct judge {
    int judge_id;
    int pipe_to_judge_fd[2];
    int pipe_from_judge_fd[2];
    pid_t pid;
} JUDGE;

int main(int argc, char *argv[])
{
    // ./organizer judge_num player_num
    if (argc != 3) {
        fprintf(stderr, "Wrong format\n");
        exit(EXIT_FAILURE);
    }

    // handling param judge_num
    char *test = '\0';
    unsigned int judge_num = strtol(argv[1], &test, 10);
    if (*test != '\0') {
        fprintf(stderr, "Please give numeric argument for judge_num\n");
        exit(EXIT_FAILURE);
    } else if (judge_num < MIN_JUDGE_NUM || judge_num > MAX_JUDGE_NUM) {
        fprintf(stderr, "Please enter judge_num : %d <= judge_num <= %d\n", MIN_JUDGE_NUM, MAX_JUDGE_NUM);
        exit(EXIT_FAILURE);
    }

    // handling param player_num
    test = '\0';
    unsigned int player_num = strtol(argv[2], &test, 10);
    if (*test != '\0') {
        fprintf(stderr, "Please give numeric argument for player_num\n");
        exit(EXIT_FAILURE);
    } else if (player_num < MIN_PLAYER_NUM || player_num > MAX_PLAYER_NUM) {
        fprintf(stderr, "Please enter player_num : %d <= player_num <= %d\n", MIN_PLAYER_NUM, MAX_JUDGE_NUM);
        exit(EXIT_FAILURE);
    }

    // initiate judges processes and pipes
    int i, j;
    char buffer[MAX_BUFFER_SIZE];
    JUDGE judges[MAX_JUDGE_NUM];
    for (i = 0; i < judge_num; i++) {
        judges[i].judge_id = i + 1;
        // create two-direction pipe first.
        if (pipe(judges[i].pipe_from_judge_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(judges[i].pipe_to_judge_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        // successfully create pipe, then fork.
        pid_t cpid = fork();
        if (cpid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (cpid == 0) { // child
            //execl("./judge", "./judge", judges[i].judge_id, (char *)0);
            sprintf(buffer, "%d", judges[i].judge_id);
            execl("./judge", "./judge", buffer, (char *)0);
            perror("execl");
            _exit(EXIT_FAILURE);
        } else {
            judges[i].pid = cpid;
        }
    }
    printf("%d %d\n", judge_num, player_num);
    exit(EXIT_SUCCESS);
}
