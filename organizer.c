#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MIN_JUDGE_NUM 1
#define MAX_JUDGE_NUM 12
#define MIN_PLAYER_NUM 8
#define MAX_PLAYER_NUM 16
#define MAX_BUFFER_SIZE 10000

typedef struct judge {
    int judge_id;
    // 0 for read, 1 for write
    int pipe_to_judge_fd[2];
    int pipe_from_judge_fd[2];
    pid_t pid;
} JUDGE;

typedef struct player {
    int score;
} PLAYER;

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
        fprintf(stderr, "Please enter player_num : %d <= player_num <= %d\n", MIN_PLAYER_NUM, MAX_PLAYER_NUM);
        exit(EXIT_FAILURE);
    }

    // initiate judges processes and pipes
    int i, j;
    char buffer[MAX_BUFFER_SIZE];
    JUDGE judges[MAX_JUDGE_NUM];
    PLAYER players[MAX_PLAYER_NUM];

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
            /* GOAL : make judge's stdin & stdout are from/to organizer */
            /* handle to_judge first */
            // close read end
            close(judges[i].pipe_to_judge_fd[1]);
            dup2(judges[i].pipe_to_judge_fd[0], STDIN_FILENO);
            close(judges[i].pipe_to_judge_fd[0]);

            /* handle from_judge */
            // close write end of org to listen from judge
            close(judges[i].pipe_from_judge_fd[0]);
            dup2(judges[i].pipe_from_judge_fd[1], STDOUT_FILENO);
            close(judges[i].pipe_from_judge_fd[1]);

            /* call judge */
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "%d", i + 1);
            // use (char *)0 instead of NULL,
            // due to the fact that in some system NULL is not equal to 0.
            execl("./judge", "./judge", buffer, (char *)0);

            /* should never run to here */
            perror("execl");
            _exit(EXIT_FAILURE);
        } else {
            judges[i].pid = cpid;
            // close read end cuz dont need it.
            close(judges[i].pipe_to_judge_fd[0]);
            // write
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "%d %d %d %d\n", i*4, i*4 + 1, i*4 + 2, i*4 + 3);
            write(judges[i].pipe_to_judge_fd[1], buffer, sizeof(buffer));
        }
    }
    for (i = 0; i < judge_num; i++) {
        int stauts;
        wait(&stauts);
    }

    return EXIT_SUCCESS;
}
