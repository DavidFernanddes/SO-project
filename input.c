#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "input.h"

int read_file(char *file_name, Track tracks[], Train trains[], int file_type) {
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 0;
    }

    int num_tracks, num_trains;
    fscanf(file, "%d %d", &num_tracks, &num_trains);

    for (int t = 0; t < num_tracks; t++) {
        if (file_type == 1) {
            fscanf(file, "%d %d", &tracks[t].num, &tracks[t].size);
            tracks[t].num_stops = 0;
        } else if (file_type == 2) {
            fscanf(file, "%d %d %d", &tracks[t].num, &tracks[t].size, &tracks[t].num_stops);
        }

        for (int p = 0; p < tracks[t].size; p++) {
            fscanf(file, "%d %d", &tracks[t].position[p].x, &tracks[t].position[p].y);
        }

        if (file_type == 2) {
            for (int s = 0; s < tracks[t].num_stops; s++) {
                fscanf(file, "%d %d", &tracks[t].stops[s].stop_index, &tracks[t].stops[s].semaphore_index);
            }
        }
    }

    for (int c = 0; c < num_trains; c++) {
        fscanf(file, "%d %d %d %d", &trains[c].num, &trains[c].track, 
               &trains[c].position, &trains[c].speed);
        
        trains[c].counter = 0;
        trains[c].state = 'M';
        
        if (file_type == 2) {
            find_train_stops(&trains[c], &tracks[trains[c].track]);
        }
    }

    fclose(file);
    return num_trains;
}

void fill_board(char board[BOARD_SIZE][BOARD_SIZE], Track tracks[], int num_tracks, Train trains[], int num_trains) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }

    for (int t = 0; t < num_tracks; t++) {
        if (t < 0 || t >= MAX_TRACKS) continue;

        int size = tracks[t].size;
        if (size < 0 || size > MAX_POSITIONS) continue;

        for (int p = 0; p < size; p++) {
            int x = tracks[t].position[p].x;
            int y = tracks[t].position[p].y;

            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                int is_stop = 0;
                for (int s = 0; s < tracks[t].num_stops; s++) {
                    if (tracks[t].stops[s].stop_index == p) {
                        board[y][x] = 'S';
                        is_stop = 1;
                        break;
                    }
                }
                
                if (!is_stop) {
                    board[y][x] = 'X';
                }
            }
        }
    }

    for (int c = 0; c < num_trains; c++) {
        if (c < 0 || c >= MAX_TRAINS) continue;

        int track_idx = trains[c].track;
        int pos_idx = trains[c].position;

        if (track_idx < 0 || track_idx >= num_tracks) continue;

        int track_size = tracks[track_idx].size;
        if (pos_idx < 0 || pos_idx >= track_size) continue;

        int x = tracks[track_idx].position[pos_idx].x;
        int y = tracks[track_idx].position[pos_idx].y;

        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
            board[y][x] = '0' + (trains[c].num % 10);
        }
    }
}

void print_state(char board[BOARD_SIZE][BOARD_SIZE], Train trains[], int num_trains, Track tracks[]) {
    printf("Tracks\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
        
    printf("Trains:\n");
    for (int c = 0; c < num_trains; c++) {
        int track_idx = trains[c].track;
        int pos_idx = trains[c].position;
        int x = tracks[track_idx].position[pos_idx].x;
        int y = tracks[track_idx].position[pos_idx].y;
        
        printf("T%d (%d,%d) - %d - %c\n", 
               trains[c].num, x, y, 
               trains[c].section, trains[c].state);
    }
    printf("\n");
}

void find_train_stops(Train *train, Track *track) {
    int pos = train->position;
    int last_stop_idx = -1;
    int next_stop_idx = -1;
    
    for (int s = 0; s < track->num_stops; s++) {
        if (track->stops[s].stop_index <= pos) {
            if (last_stop_idx == -1 || track->stops[s].stop_index > track->stops[last_stop_idx].stop_index) {
                last_stop_idx = s;
            }
        }
    }
    
    if (last_stop_idx == -1) {
        last_stop_idx = track->num_stops - 1;
    }
    
    for (int s = 0; s < track->num_stops; s++) {
        if (track->stops[s].stop_index > pos) {
            if (next_stop_idx == -1 || track->stops[s].stop_index < track->stops[next_stop_idx].stop_index) {
                next_stop_idx = s;
            }
        }
    }
    
    if (next_stop_idx == -1) {
        next_stop_idx = 0;
    }
    
    train->last_stop = last_stop_idx;
    train->next_stop = next_stop_idx;
    train->section = last_stop_idx;
    
    for (int s = 0; s < track->num_stops; s++) {
        if (track->stops[s].stop_index == pos) {
            train->state = 'S';
            break;
        }
    }
}

void train_process(Train *train, Track tracks[], sem_t *semaphores[], sem_t *mutex, int wait_time) {
    Track *track = &tracks[train->track];
    
    while (1) {
        usleep(wait_time * train->speed);
        
        sem_wait(mutex);
        train->counter++;
        
        if (train->counter % train->speed == 0) {
            if (train->state == 'S' || train->state == 'W') {
                train->state = 'W';
                
                int next_sem_idx = track->stops[train->next_stop].semaphore_index;
                
                sem_post(mutex);
                
                sem_wait(semaphores[next_sem_idx]);
                
                sem_wait(mutex);
                
                train->last_stop = train->next_stop;
                train->next_stop = (train->next_stop + 1) % track->num_stops;
                
                train->section = train->last_stop;
                train->state = 'M';
                
                train->position++;
                if (train->position >= track->size) {
                    train->position = 0;
                }
                
                sem_post(mutex);
            } else {
                train->position++;
                if (train->position >= track->size) {
                    train->position = 0;
                }
                
                int at_stop = 0;
                for (int s = 0; s < track->num_stops; s++) {
                    if (track->stops[s].stop_index == train->position) {
                        int prev_sem_idx = track->stops[train->last_stop].semaphore_index;
                        
                        train->state = 'S';
                        at_stop = 1;
                        
                        sem_post(semaphores[prev_sem_idx]);
                        break;
                    }
                }
                
                sem_post(mutex);
            }
        } else {
            sem_post(mutex);
        }
    }
}
