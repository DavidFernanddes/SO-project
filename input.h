#ifndef INPUT_H
#define INPUT_H

#include "structures.h"

// Function to read the file and fill the structures
int read_file(char *file_name, Track tracks[], Train trains[], int file_type);

// Function to fill the board with tracks and trains
void fill_board(char board[BOARD_SIZE][BOARD_SIZE], Track tracks[], int num_tracks, Train trains[], int num_trains);

// Function to print the current state of the board and trains
void print_state(char board[BOARD_SIZE][BOARD_SIZE], Train trains[], int num_trains, Track tracks[]);

// Initialize semaphores for train synchronization
void init_semaphores(sem_t *semaphores[], int num_semaphores);

// Free semaphores
void free_semaphores(sem_t *semaphores[], int num_semaphores);

// Train process function
void train_process(Train *train, Track tracks[], sem_t *semaphores[], sem_t *mutex, int wait_time);

// Find the current section, last stop and next stop for a train
void find_train_stops(Train *train, Track *track);

#endif