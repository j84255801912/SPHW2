#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "include.h"

int main(int argc, char *argv[])
{
    // ./organizer judge_num player_num
    if (argc != 3) {
        fprintf(stderr, "Wrong argument format\n");
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
    int i, j, a, b, c, d;
    char buffer[MAX_BUFFER_SIZE];

    // init players
    PLAYER players[MAX_PLAYER_NUM];
    for (i = 0; i < player_num; i++) {
        players[i].id = i;
        players[i].score = 0;
    }
    // init judges
    JUDGE judges[MAX_JUDGE_NUM];
    for (i = 0; i < judge_num; i++)
        judges[i].busy = 0;

    /* */
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
            sprintf(buffer, "%d", judges[i].judge_id);
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
        }
    } // for (i = 0; i < judge_num; i++)

    fd_set the_set, temp_set;
    FD_ZERO(&the_set);
    int max_fd = 0;
    for (i = 0; i < judge_num; i++) {
        // add the fd to the fdset
        FD_SET(judges[i].pipe_from_judge_fd[0], &the_set);
        /* handle pipe_from_judge_fd[0] */
        if (judges[i].pipe_from_judge_fd[0] > max_fd)
            max_fd = judges[i].pipe_from_judge_fd[0];
    }

    int counting = 0;

    /* distribute all possible competitions to idle judges */
    for (a = player_num - 1; a >= 0; a--) {
        for (b = player_num - 1; b > a; b--) {
            for (c = player_num - 1; c > b; c--) {
                for (d = player_num - 1; d > c; d--) {
                    counting += 1;
                    /* find an idle judge */
                    int flag = 0;
                    for (i = 0; i < judge_num; i++)
                        if (judges[i].busy == 0) {
                            flag = 1;
                            break;
                        }
                    // case 1: if someone is available
                    if (flag) {
                        fprintf(stderr, "judge %d : %dth game is starting.\n", i + 1, counting + 1);
                        /* the judge[i] is assigned by these players */
                        // write players id to judges
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%d %d %d %d\n", a, b, c, d);
                        write(judges[i].pipe_to_judge_fd[1], buffer, sizeof(buffer));
                        judges[i].busy = 1;
                    } else {
                    // case 2: no one is available,
                    // wait for someone and write to him.
                        /* if there is anything to read from judge, handle them */
                        int result;
                        memcpy(&temp_set, &the_set, sizeof(temp_set));
                        result = select(max_fd + 1, &temp_set, NULL, NULL, NULL);
                        if (result > 0) { // success
                            for (i = 0; i < judge_num; i++) {
                                if (FD_ISSET(judges[i].pipe_from_judge_fd[0], &temp_set)) {
                                    /* deduct points for loser */
                                    bzero(buffer, sizeof(buffer));
                                    read(judges[i].pipe_from_judge_fd[0], buffer, sizeof(buffer));
                                    buffer[strlen(buffer) - 1] = '\0';
                                    players[atoi(buffer)].score -= 1;
                                    // change the judge's state to idle
                                    judges[i].busy = 0;
                                } // if (FD_ISSET
                            } // for (i = 0; i < judge
                        } // if (result > 0)
                        // when run to here, must be someone is available.
                        for (i = 0; i < judge_num; i++)
                            if (judges[i].busy == 0) {
                                flag = 1;
                                break;
                            }
                        fprintf(stderr, "%dth competition start, judge %d is busy now\n", counting, i+1);
                        /* the judge[i] is assigned by these players */
                        // write players id to judges
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%d %d %d %d\n", a, b, c, d);
                        write(judges[i].pipe_to_judge_fd[1], buffer, sizeof(buffer));
                        judges[i].busy = 1;
                    }
                } // for d
            } // for c
        } // for b
    } // for a
    /* tell judges to stop and close their pipes*/
    for (i = 0; i < judge_num; i++) {
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "0 0 0 0\n");
        write(judges[i].pipe_to_judge_fd[1], buffer, sizeof(buffer));
    //    close(judges[i].pipe_to_judge_fd[1]);
    //    close(judges[i].pipe_from_judge_fd[0]);
    }
    /* wait for childs, in case of zombies */
    for (i = 0; i < judge_num; i++) {
        int status;
        wait(&status);
    }

    // sort players by score and id
    qsort(&players, player_num, sizeof(PLAYER), compare);

    /* scoreboard */
    fprintf(stderr, "\n[RESULT] : %d competition completed\n", counting);
    fprintf(stderr, "\n- scores -\n");
    for (i = 0; i < player_num; i++) {
        fprintf(stderr, "%d %d\n", players[i].id, players[i].score);
    }
    return EXIT_SUCCESS;
}
