#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#ifdef __linux__
    #include <sys/prctl.h>
#endif
#include "structures.h"
#include "input.h"

#define MAX_SEMAPHORES 10
#define SEM_NAME_SIZE 64

typedef struct {
    Track* tracks;
    Train* trains;
    char (*board)[BOARD_SIZE];
    sem_t *semaphores[MAX_SEMAPHORES];
    sem_t *mutex;
    int num_processes;
    pid_t *pids;
    char sem_names[MAX_SEMAPHORES+1][SEM_NAME_SIZE];
} Resources;

void cleanup(Resources *resources);
void sigint_handler(int sig);

Resources *global_resources = NULL;

void cleanup(Resources *resources) {
    if (resources == NULL) {
        return;
    }

    if (resources->mutex != NULL) {
        sem_post(resources->mutex);
        sem_close(resources->mutex);
    }
    
    if (strlen(resources->sem_names[MAX_SEMAPHORES]) > 0) {
        sem_unlink(resources->sem_names[MAX_SEMAPHORES]);
    }
    
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (resources->semaphores[i] != NULL) {
            sem_post(resources->semaphores[i]);
            sem_close(resources->semaphores[i]);
        }
        
        if (strlen(resources->sem_names[i]) > 0) {
            sem_unlink(resources->sem_names[i]);
        }
    }
    
    if (resources->tracks != NULL) {
        munmap(resources->tracks, sizeof(Track) * MAX_TRACKS);
    }
    
    if (resources->trains != NULL) {
        munmap(resources->trains, sizeof(Train) * MAX_TRAINS);
    }
    
    if (resources->board != NULL) {
        munmap(resources->board, sizeof(char) * BOARD_SIZE * BOARD_SIZE);
    }
    
    if (resources->pids != NULL) {
        free(resources->pids);
    }
}

void sigint_handler(int sig) {
    printf("\nReceived SIGINT. Cleaning up and exiting...\n");
    
    if (global_resources != NULL) {
        for (int i = 0; i < global_resources->num_processes; i++) {
            if (global_resources->pids[i] > 0) {
                kill(global_resources->pids[i], SIGKILL);
            }
        }
        
        cleanup(global_resources);
    }
    
    exit(0);
}

void init_semaphores(Resources *resources) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        snprintf(resources->sem_names[i], SEM_NAME_SIZE, "/train_sem_%d_%d", i, (int)getpid());
        sem_unlink(resources->sem_names[i]);
        resources->semaphores[i] = sem_open(resources->sem_names[i], O_CREAT | O_EXCL, 0644, 1);
        
        if (resources->semaphores[i] == SEM_FAILED) {
            perror("Error creating semaphore");
            exit(1);
        }
    }
    
    snprintf(resources->sem_names[MAX_SEMAPHORES], SEM_NAME_SIZE, "/train_mutex_%d", (int)getpid());
    sem_unlink(resources->sem_names[MAX_SEMAPHORES]);
    resources->mutex = sem_open(resources->sem_names[MAX_SEMAPHORES], O_CREAT | O_EXCL, 0644, 1);
    
    if (resources->mutex == SEM_FAILED) {
        perror("Error creating mutex");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Usage: %s <input_file> <wait_time_ms> [file_type]\n", argv[0]);
        return 1;
    }

    char* file_name = argv[1];
    int wait_time = atoi(argv[2]) * 1000;
    int file_type = (argc == 4) ? atoi(argv[3]) : 0;
    
    Resources resources;
    memset(&resources, 0, sizeof(Resources));
    
    global_resources = &resources;

    signal(SIGINT, sigint_handler);

    resources.tracks = mmap(NULL, sizeof(Track) * MAX_TRACKS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    resources.trains = mmap(NULL, sizeof(Train) * MAX_TRAINS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    resources.board = mmap(NULL, sizeof(char) * BOARD_SIZE * BOARD_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (resources.tracks == MAP_FAILED || resources.trains == MAP_FAILED || resources.board == MAP_FAILED) {
        perror("Error allocating shared memory");
        cleanup(&resources);
        return 1;
    }

    init_semaphores(&resources);

    int num_trains = read_file(file_name, resources.tracks, resources.trains, file_type);
    if (num_trains == 0) {
        cleanup(&resources);
        return 1;
    }

    resources.pids = (pid_t *)malloc(sizeof(pid_t) * num_trains);
    if (resources.pids == NULL) {
        perror("Error allocating PIDs array");
        cleanup(&resources);
        return 1;
    }
    resources.num_processes = num_trains;

    for (int i = 0; i < num_trains; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error creating process");
            cleanup(&resources);
            return 1;
        }

        if (pid == 0) {
            #ifdef __linux__
                if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1) {
                    perror("prctl failed");
                    exit(1);
                }
            #endif
            
            if (file_type == 2) {
                train_process(&resources.trains[i], resources.tracks, resources.semaphores, resources.mutex, wait_time);
            } else if (file_type == 1) {
                while (1) {
                    sem_wait(resources.mutex);
                    resources.trains[i].counter++;
                    if (resources.trains[i].counter % resources.trains[i].speed == 0) {
                        resources.trains[i].position++;
                        if (resources.trains[i].position >= resources.tracks[resources.trains[i].track].size) {
                            resources.trains[i].position = 0;
                        }
                    }
                    sem_post(resources.mutex);
                    usleep(wait_time);
                }
            }
            
            for (int j = 0; j < MAX_SEMAPHORES; j++) {
                if (resources.semaphores[j] != NULL) {
                    sem_close(resources.semaphores[j]);
                }
            }
            if (resources.mutex != NULL) {
                sem_close(resources.mutex);
            }
            exit(0);
        } else {
            resources.pids[i] = pid;
        }
    }

    while (1) {
        sem_wait(resources.mutex);
        fill_board(resources.board, resources.tracks, MAX_TRACKS, resources.trains, num_trains);
        print_state(resources.board, resources.trains, num_trains, resources.tracks);
        sem_post(resources.mutex);
        usleep(wait_time);
    }

    for (int i = 0; i < num_trains; i++) {
        wait(NULL);
    }

    cleanup(&resources);
    return 0;
}
