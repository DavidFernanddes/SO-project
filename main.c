#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#ifdef __linux__
    #include <sys/prctl.h>
#endif
#include "structures.h"
#include "input.h"

#define MAX_SEMAPHORES 10

// Global pointers for cleanup on exit
Track* tracks = NULL;
Train* trains = NULL;
char (*board)[BOARD_SIZE] = NULL;
sem_t **semaphores = NULL;
sem_t *mutex = NULL;
int num_processes = 0;
pid_t *pids = NULL;

// Cleanup function
void cleanup() {
    // Free resources
    if (mutex != NULL) {
        sem_destroy(mutex);
        free(mutex);
    }
    
    if (semaphores != NULL) {
        free_semaphores(semaphores, MAX_SEMAPHORES);
        free(semaphores);
    }
    
    if (tracks != NULL) {
        munmap(tracks, sizeof(Track) * MAX_TRACKS);
    }
    
    if (trains != NULL) {
        munmap(trains, sizeof(Train) * MAX_TRAINS);
    }
    
    if (board != NULL) {
        munmap(board, sizeof(char) * BOARD_SIZE * BOARD_SIZE);
    }
    
    if (pids != NULL) {
        free(pids);
    }
}

// Signal handler for clean termination
void sigint_handler(int sig) {
    printf("\nReceived SIGINT. Cleaning up and exiting...\n");
    
    // Kill all child processes
    for (int i = 0; i < num_processes; i++) {
        if (pids[i] > 0) {
            kill(pids[i], SIGKILL);
        }
    }
    
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Usage: %s <input_file> <wait_time_ms> [file_type]\n", argv[0]);
        return 1;
    }

    char* file_name = argv[1];
    int wait_time = atoi(argv[2]) * 1000; // Convert to microseconds
    int file_type = (argc == 4) ? atoi(argv[3]) : 0; // If not specified, assume preliminary type

    // Set up signal handler
    signal(SIGINT, sigint_handler);

    // Allocate shared memory for tracks and trains
    tracks = mmap(NULL, sizeof(Track) * MAX_TRACKS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    trains = mmap(NULL, sizeof(Train) * MAX_TRAINS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    board = mmap(NULL, sizeof(char) * BOARD_SIZE * BOARD_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (tracks == MAP_FAILED || trains == MAP_FAILED || board == MAP_FAILED) {
        perror("Error allocating shared memory");
        cleanup();
        return 1;
    }

    // Initialize semaphores
    semaphores = (sem_t **)malloc(sizeof(sem_t *) * MAX_SEMAPHORES);
    if (semaphores == NULL) {
        perror("Error allocating semaphores array");
        cleanup();
        return 1;
    }
    init_semaphores(semaphores, MAX_SEMAPHORES);

    // Create mutex for protecting shared memory
    mutex = (sem_t *)malloc(sizeof(sem_t));
    if (mutex == NULL) {
        perror("Error allocating mutex");
        cleanup();
        return 1;
    }
    if (sem_init(mutex, 1, 1) != 0) {
        perror("Error initializing mutex");
        cleanup();
        return 1;
    }

    // Read the file and fill structures
    int num_trains = read_file(file_name, tracks, trains, file_type);
    if (num_trains == 0) {
        cleanup();
        return 1;
    }

    // Allocate memory for process IDs
    pids = (pid_t *)malloc(sizeof(pid_t) * num_trains);
    if (pids == NULL) {
        perror("Error allocating PIDs array");
        cleanup();
        return 1;
    }
    num_processes = num_trains;

    // Create a process for each train
    for (int i = 0; i < num_trains; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error creating process");
            cleanup();
            return 1;
        }

        if (pid == 0) {
            // Child process - train
            #ifdef __linux__
                if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1) {
                    perror("prctl failed");
                    exit(1);
                }
            #endif
            
            // Train process
            if (file_type == 1) {
                train_process(&trains[i], tracks, semaphores, mutex, wait_time);
            } else {
                // Simple movement for preliminary version
                while (1) {
                    sem_wait(mutex);
                    trains[i].counter++;
                    if (trains[i].counter % trains[i].speed == 0) {
                        trains[i].position++;
                        if (trains[i].position >= tracks[trains[i].track].size) {
                            trains[i].position = 0;
                        }
                    }
                    sem_post(mutex);
                    usleep(wait_time);
                }
            }
            exit(0);
        } else {
            // Parent process - save the child PID
            pids[i] = pid;
        }
    }

    // Parent process: update and print the board
    while (1) {
        sem_wait(mutex);
        fill_board(board, tracks, MAX_TRACKS, trains, num_trains);
        print_state(board, trains, num_trains, tracks);
        sem_post(mutex);
        usleep(wait_time);
    }

    // This code will never be reached due to the infinite loop above
    // It would only be reached if we added a termination condition

    // Wait for all child processes
    for (int i = 0; i < num_trains; i++) {
        wait(NULL);
    }

    cleanup();
    return 0;
}