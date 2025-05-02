#ifndef INPUT_H
#define INPUT_H

#include "structures.h"

int read_file(char *file_name, Track tracks[], Train trains[], int file_type);

void fill_board(char board[BOARD_SIZE][BOARD_SIZE], Track tracks[], int num_tracks, Train trains[], int num_trains);

void print_state(char board[BOARD_SIZE][BOARD_SIZE], Train trains[], int num_trains, Track tracks[]);

void find_train_stops(Train *train, Track *track);

void train_process(Train *train, Track tracks[], sem_t *semaphores[], sem_t *mutex, int wait_time);

void cleanup(Resources *resources);

void init_semaphores(Resources *resources);

void init_shared_memory(Resources *resources);

void spawn_train_processes(Resources *resources, int num_trains, int wait_time, int file_type);

void run_simulation(Resources *resources, int wait_time);

#endif