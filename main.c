#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "structures.h"
#include "input.h"

void lerDados(char *fileTxt);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: %s <ficheiro_de_entrada> <tempo_de_espera_ms>\n", argv[0]);
        return 1;
    }

    char* file_name = argv[1];
    int wait_time = atoi(argv[2]) * 1000;

    Track tracks[MAX_TRACKS];
    Train trains[MAX_TRAINS];
    char board[BOARD_SIZE][BOARD_SIZE];

    int num_trains = read_file(file_name, tracks, trains);
    if (num_trains == 0) {
        return 1; 
    }

    while(1) {
        for (int i = 0; i < num_trains; i++) {
            update_train_position(&trains[i], tracks);
        }   

        fill_board(board, tracks, MAX_TRACKS, trains, num_trains);

        print_state(board, trains, num_trains, tracks);

        usleep(wait_time);
    }

    return 0;
}