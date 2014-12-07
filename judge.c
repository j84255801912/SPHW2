#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUF_SIZE (1000)
#define TERMINATE "0 0 0 0\n"
int main(int argc, char *argv[]) {
    /* check for args */
    if (argc != 2) {
        fprintf(stderr, "[ERROR] judge : Wrong argu format\n");
        exit(EXIT_FAILURE);
    }
    char *temp, buffer[MAX_BUF_SIZE];
    unsigned int judge_num = strtol(argv[1], &temp, 10);
    if (*temp != '\0') {
        fprintf(stderr, "[ERROR] judge : Wrong argus type");
        exit(EXIT_FAILURE);
    }

    int i, j;
    srand(time(NULL));
    /* one game per loop */
    while (1) {
        /* read player num */
        bzero(buffer, sizeof(buffer));
        read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

        /* TERMINATE when 0 0 0 0\n */
        if (strcmp(TERMINATE, buffer) == 0)
            break;

        /* fetch player_num */
        int players[4];
        // trim '\n'
        buffer[strlen(buffer) - 1] = '\0';
        char *ptr;
        ptr = strtok(buffer, " ");
        players[0] = atoi(ptr);
        ptr = strtok(NULL, " ");
        players[1] = atoi(ptr);
        ptr = strtok(NULL, " ");
        players[2] = atoi(ptr);
        ptr = strtok(NULL, " ");
        players[3] = atoi(ptr);

        /* give cards */
        int cards[52] = {0}, player_cards[4][14];
        // distribute the joker
        player_cards[0][13] = 52;
        for (i = 0; i < 4; i++)
            for (j = 0; j < 13; j++) {
                int the_card;
                while (cards[the_card = rand() % 52]);
                // mark this card used
                cards[the_card] = 1;
                player_cards[i][j] = the_card;
            }
        fprintf(stderr, "judge %d : ", judge_num);
        for (i = 0; i < 14; i++) {
            fprintf(stderr, "%d ", player_cards[0][i]);
        }
        fprintf(stderr, "\n");
    }
    exit(EXIT_SUCCESS);
}
