#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "include.h"

#define TERMINATE "0 0 0 0\n"

enum {
    A = 0,
    B,
    C,
    D
};

int main(int argc, char *argv[]) {
    /* check for args */
    if (argc != 2) {
        fprintf(stderr, "[ERROR] judge : Wrong argu format\n");
        exit(EXIT_FAILURE);
    }
    char *temp, buffer[MAX_BUFFER_SIZE];
    unsigned int judge_id = strtol(argv[1], &temp, 10);
    if (*temp != '\0') {
        fprintf(stderr, "[ERROR] judge : Wrong argus type");
        exit(EXIT_FAILURE);
    }

    int i, j;

    /* one game per loop */
    while (1) {
        /* read player num */
        bzero(buffer, sizeof(buffer));
        read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

        /* TERMINATE when 0 0 0 0\n */
        if (strcmp(TERMINATE, buffer) == 0)
            break;

        /* fetch player_id */
        int players[4];
        // trim '\n'
        buffer[strlen(buffer) - 1] = '\0';
//        fetch_data(buffer, players);
        char *ptr;
        ptr = strtok(buffer, " ");
        players[0] = atoi(ptr);
        ptr = strtok(NULL, " ");
        players[1] = atoi(ptr);
        ptr = strtok(NULL, " ");
        players[2] = atoi(ptr);
        ptr = strtok(NULL, " ");
        players[3] = atoi(ptr);

        /* give out cards */
        int cards[52] = {0}, player_cards[4][14];
        // distribute the joker
        player_cards[A][13] = 0;
        for (i = 0; i < 4; i++)
            for (j = 0; j < 13; j++) {
                int the_card;
                while (cards[the_card = rand() % 52]);
                // mark this card used
                cards[the_card] = 1;
                // generalize the_card
                player_cards[i][j] = the_card % 13 + 1;
            }
//        fprintf(stderr, "judge %d : \n", judge_id);

        /* make fifo for player to write to judge*/
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "judge%d.FIFO", judge_id);
        mkfifo(buffer, 0644);
        int fd[4];

        /* intialize players */
        for (i = 0; i < 4; i++) {
            // make fifo to write from player to judge
            char fifoname[MAX_BUFFER_SIZE];
            bzero(fifoname, sizeof(fifoname));
            sprintf(fifoname, "judge%d_%c.FIFO", judge_id, 'A' + i);
            mkfifo(fifoname, 0644);

            // fork
            pid_t cpid = fork();
            if (cpid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (cpid == 0) { // child
                bzero(buffer, sizeof(buffer));
                sprintf(buffer, "%d", judge_id);
                char buffer1[MAX_BUFFER_SIZE];
                bzero(buffer1, sizeof(buffer1));
                sprintf(buffer1, "%c", 'A' + i);
                char buffer2[MAX_BUFFER_SIZE];
                bzero(buffer2, sizeof(buffer2));
                srand(time(NULL) + judge_id + i);
                sprintf(buffer2, "%d", rand() % 65536);
                // use (char *)0 instead of NULL,
                // due to the fact that in some system NULL is not equal to 0.
                execl("./player", "./player", buffer, buffer1, buffer2, (char *)0);

                /* here should never be execute */
                perror("execl");
                _exit(EXIT_FAILURE);
            } else {
                /* write cards to players */
                bzero(buffer, sizeof(buffer));
                char buffer1[MAX_BUFFER_SIZE];
                for (j = 0; j < 13; j++) {
                    bzero(buffer1, sizeof(buffer1));
                    sprintf(buffer1, "%d ", player_cards[i][j]);
                    strcat(buffer, buffer1);
                }
                if (i == A) {
                    bzero(buffer1, sizeof(buffer1));
                    sprintf(buffer1, "%d ", player_cards[i][13]);
                    strcat(buffer, buffer1);
                }
                buffer[strlen(buffer) - 1] = '\n';

                fd[i] = open(fifoname, O_WRONLY);
                if (fd[i] == -1) {
                    perror("open");
                    exit(EXIT_SUCCESS);
                }
                write(fd[i], buffer, sizeof(buffer));
            } // else
        } // for (i = 0 to 3)

        int fd_from_players;
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, "judge%d.FIFO", judge_id);
        fd_from_players = open(buffer, O_RDONLY);

        int random_key[4];
        int number_of_cards[4];
        for (i = 0; i < 4; i++) {
            bzero(buffer, sizeof(buffer));
            read(fd_from_players, buffer, sizeof(buffer));
            // trim '\n'
            buffer[strlen(buffer) - 1] = '\0';
            ptr = strtok(buffer, " ");
            // [player_index] [random_key] [number_of_cards]
            char player_index = ptr[0];
            ptr = strtok(NULL, " ");
            random_key[player_index - 'A'] = atoi(ptr);
            ptr = strtok(NULL, " ");
            number_of_cards[player_index - 'A'] = atoi(ptr);
        }
        fprintf(stderr, "%d %d %d %d\n", number_of_cards[0], number_of_cards[1], number_of_cards[2], number_of_cards[3]);

        /* the game starts */
        int turn = A;
        int next_turn = (A + 1 ) % 4;
        while (1) {
            fprintf(stderr, "haha");
            // stop the game when only one player have cards
            int flag = 0;
            for (i = 0; i < 4; i++)
                if (number_of_cards[i] == 0)
                    flag += 1;
            if (flag == 3)
                break;
            // send the card_count of next to now
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "< %d\n", number_of_cards[next_turn]);
            write(fd[turn], buffer, sizeof(buffer));
            // get the card_id from now player
            bzero(buffer, sizeof(buffer));
            // [player_index] [random_key] [card_ID]
            read(fd_from_players, buffer, sizeof(buffer));
            buffer[strlen(buffer) - 1] = '\0';
            ptr = strtok(buffer, " ");
            char player_index = ptr[0];
            ptr = strtok(NULL, " ");
            int rand_key = atoi(ptr);
            ptr = strtok(NULL, " ");
            int card_id = atoi(ptr);
            if (rand_key != random_key[player_index - 'A'])
                fprintf(stderr, "someone is cheating!\n");
            // send to next_player which card is to be drawn
            bzero(buffer, sizeof(buffer));
            // ????? not sure
            sprintf(buffer, "> %d\n", card_id);
            write(fd[next_turn], buffer, sizeof(buffer));
            // read from next_player his drawn card.
            bzero(buffer, sizeof(buffer));
            read(fd_from_players, buffer, sizeof(buffer));
            buffer[strlen(buffer) - 1] = '\0';
            // [player_index] [random_key] [the_card_number]
            ptr = strtok(buffer, " ");
            player_index = ptr[0];
            ptr = strtok(NULL, " ");
            rand_key = atoi(ptr);
            ptr = strtok(NULL, " ");
            int the_card_number = atoi(ptr);
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "%d\n", the_card_number);
            write(fd[turn], buffer, sizeof(buffer));
            // get response
            bzero(buffer, sizeof(buffer));
            read(fd_from_players, buffer, sizeof(buffer));
            buffer[strlen(buffer) - 1] = '\0';
            ptr = strtok(buffer, " ");
            player_index = ptr[0];
            ptr = strtok(NULL, " ");
            rand_key = atoi(ptr);
            ptr = strtok(NULL, " ");
            int eliminated = atoi(ptr);
            if (eliminated)
                number_of_cards[turn] -= 2;
            turn = (turn + 1) % 4;
            next_turn = (turn + 1) % 4;
        } // while (1)
    } // while (1)
    return EXIT_SUCCESS;
}
