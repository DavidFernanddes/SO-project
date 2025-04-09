#ifndef INPUT_H
#define INPUT_H

#include "structures.h"

int read_file(char *file_name, Track tracks[], Train trains[]);

void fill_board(char board[BOARD_SIZE][BOARD_SIZE], Track tracks[], int num_tracks, Train trains[], int num_trains);

void print_state(char board[BOARD_SIZE][BOARD_SIZE], Train trains[], int num_trains, Track tracks[]);

void update_train_position(Train* train, Track tracks[]);

#endif