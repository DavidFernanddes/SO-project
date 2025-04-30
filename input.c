#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "input.h"

// Function to read the data from the file and fill the track and train structures
int read_file(char *file_name, Track tracks[], Train trains[], int file_type) {
    // Open the file for reading
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 0;
    }

    int num_tracks, num_trains;
    fscanf(file, "%d %d", &num_tracks, &num_trains);

    // Read track data
    for (int t = 0; t < num_tracks; t++) {
        if (file_type == 1) {
            // Format for phase 1 (with stops)
            fscanf(file, "%d %d %d", &tracks[t].num, &tracks[t].size, &tracks[t].num_stops);
        } else {
            // Format for preliminary delivery (no stops)
            fscanf(file, "%d %d", &tracks[t].num, &tracks[t].size);
            tracks[t].num_stops = 0;
        }

        // Read positions for the track
        for (int p = 0; p < tracks[t].size; p++) {
            fscanf(file, "%d %d", &tracks[t].position[p].x, &tracks[t].position[p].y);
        }

        // Read stops for the track (only for phase 1)
        if (file_type == 1) {
            for (int s = 0; s < tracks[t].num_stops; s++) {
                fscanf(file, "%d %d", &tracks[t].stops[s].stop_index, &tracks[t].stops[s].semaphore_index);
            }
        }
    }

    // Read train data
    for (int c = 0; c < num_trains; c++) {
        fscanf(file, "%d %d %d %d", &trains[c].num, &trains[c].track, 
               &trains[c].position, &trains[c].speed);
        
        trains[c].counter = 0;       // Initialize the train's counter
        trains[c].state = 'M';       // Initialize the train's state to Moving
        
        // If phase 1, initialize the train's section
        if (file_type == 1) {
            find_train_stops(&trains[c], &tracks[trains[c].track]);
        }
    }

    fclose(file);
    return num_trains;
}

// Function to fill the board with tracks, stops and trains
void fill_board(char board[BOARD_SIZE][BOARD_SIZE], Track tracks[], int num_tracks, Train trains[], int num_trains) {
    // Initialize the board with empty spaces
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }

    // Fill the board with tracks
    for (int t = 0; t < num_tracks; t++) {
        if (t < 0 || t >= MAX_TRACKS) continue; // Check track limits

        int size = tracks[t].size;
        if (size < 0 || size > MAX_POSITIONS) continue; // Check position limits

        for (int p = 0; p < size; p++) {
            int x = tracks[t].position[p].x;
            int y = tracks[t].position[p].y;

            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                // Check if this position is a stop
                int is_stop = 0;
                for (int s = 0; s < tracks[t].num_stops; s++) {
                    if (tracks[t].stops[s].stop_index == p) {
                        board[y][x] = 'S'; // Mark as stop
                        is_stop = 1;
                        break;
                    }
                }
                
                // If not a stop, mark as track
                if (!is_stop) {
                    board[y][x] = 'X';
                }
            }
        }
    }

    // Fill the board with trains
    for (int c = 0; c < num_trains; c++) {
        if (c < 0 || c >= MAX_TRAINS) continue; // Check train limits

        int track_idx = trains[c].track;
        int pos_idx = trains[c].position;

        if (track_idx < 0 || track_idx >= num_tracks) continue; // Check if track is valid

        int track_size = tracks[track_idx].size;
        if (pos_idx < 0 || pos_idx >= track_size) continue; // Check if position is valid

        int x = tracks[track_idx].position[pos_idx].x;
        int y = tracks[track_idx].position[pos_idx].y;

        // Check if the position is within the board limits
        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
            board[y][x] = '0' + (trains[c].num % 10);
        }
    }
}

// Function to print the current state of the board and trains
void print_state(char board[BOARD_SIZE][BOARD_SIZE], Train trains[], int num_trains, Track tracks[]) {
    // Print the board state with tracks and trains
    printf("Tracks\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
        
    // Print the state of each train
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

// Initialize semaphores for train synchronization
void init_semaphores(sem_t *semaphores[], int num_semaphores) {
    for (int i = 0; i < num_semaphores; i++) {
        semaphores[i] = (sem_t*)malloc(sizeof(sem_t));
        if (semaphores[i] == NULL) {
            perror("Error allocating semaphore");
            exit(1);
        }
        if (sem_init(semaphores[i], 1, 1) != 0) {
            perror("Error initializing semaphore");
            exit(1);
        }
    }
}

// Free semaphores
void free_semaphores(sem_t *semaphores[], int num_semaphores) {
    for (int i = 0; i < num_semaphores; i++) {
        if (semaphores[i] != NULL) {
            sem_destroy(semaphores[i]);
            free(semaphores[i]);
        }
    }
}

// Find the current section, last stop and next stop for a train
void find_train_stops(Train *train, Track *track) {
    int pos = train->position;
    int last_stop_idx = -1;
    int next_stop_idx = -1;
    
    // Find the last stop before the current position
    for (int s = 0; s < track->num_stops; s++) {
        if (track->stops[s].stop_index <= pos) {
            if (last_stop_idx == -1 || track->stops[s].stop_index > track->stops[last_stop_idx].stop_index) {
                last_stop_idx = s;
            }
        }
    }
    
    // If no stop before the current position, the last stop is the last one in the track
    if (last_stop_idx == -1) {
        last_stop_idx = track->num_stops - 1;
    }
    
    // Find the next stop after the current position
    for (int s = 0; s < track->num_stops; s++) {
        if (track->stops[s].stop_index > pos) {
            if (next_stop_idx == -1 || track->stops[s].stop_index < track->stops[next_stop_idx].stop_index) {
                next_stop_idx = s;
            }
        }
    }
    
    // If no stop after the current position, the next stop is the first one in the track
    if (next_stop_idx == -1) {
        next_stop_idx = 0;
    }
    
    train->last_stop = last_stop_idx;
    train->next_stop = next_stop_idx;
    train->section = last_stop_idx;
    
    // Check if train is at a stop position
    for (int s = 0; s < track->num_stops; s++) {
        if (track->stops[s].stop_index == pos) {
            train->state = 'S';  // At stop
            break;
        }
    }
}

// Train process function
void train_process(Train *train, Track tracks[], sem_t *semaphores[], sem_t *mutex, int wait_time) {
    Track *track = &tracks[train->track];
    
    while (1) {
        // Wait based on train speed
        usleep(wait_time * train->speed);
        
        // Lock the mutex to safely update shared memory
        sem_wait(mutex);
        
        train->counter++;
        
        // Check if it's time to move the train
        if (train->counter % train->speed == 0) {
            // If train is at a stop
            if (train->state == 'S' || train->state == 'W') {
                // Change state to W (waiting) - will be changed to M if it can move
                train->state = 'W';
                
                // Try to get the semaphore for the next section
                int next_sem_idx = track->stops[train->next_stop].semaphore_index;
                
                sem_post(mutex); // Release mutex before waiting on another semaphore
                
                // Try to acquire the next section
                if (sem_trywait(semaphores[next_sem_idx]) == 0) {
                    // Successfully acquired the semaphore
                    sem_wait(mutex); // Re-acquire mutex
                    
                    // Update stop indices
                    train->last_stop = train->next_stop;
                    train->next_stop = (train->next_stop + 1) % track->num_stops;
                    
                    // Update section and state
                    train->section = train->last_stop;
                    train->state = 'M';
                    
                    // Update train position
                    train->position++;
                    if (train->position >= track->size) {
                        train->position = 0;
                    }
                } else {
                    // Could not acquire semaphore, keep waiting
                    sem_wait(mutex); // Re-acquire mutex
                }
            } else { // Train is moving
                // Update train position
                train->position++;
                if (train->position >= track->size) {
                    train->position = 0;
                }
                
                // Check if the train arrived at a stop
                int at_stop = 0;
                for (int s = 0; s < track->num_stops; s++) {
                    if (track->stops[s].stop_index == train->position) {
                        // Release the semaphore of the last section
                        int prev_sem_idx = track->stops[train->last_stop].semaphore_index;
                        sem_post(semaphores[prev_sem_idx]);
                        
                        // Update state to S (at stop)
                        train->state = 'S';
                        at_stop = 1;
                        break;
                    }
                }
            }
        }
        
        sem_post(mutex); // Release the mutex
    }
}