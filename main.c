#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
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

    // Allocate shared memory for tracks and trains
    Track* tracks = mmap(NULL, sizeof(Track) * MAX_TRACKS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    Train* trains = mmap(NULL, sizeof(Train) * MAX_TRAINS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    char (*board)[BOARD_SIZE] = mmap(NULL, sizeof(char) * BOARD_SIZE * BOARD_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (tracks == MAP_FAILED || trains == MAP_FAILED || board == MAP_FAILED) {
        perror("Erro ao alocar memória compartilhada");
        return 1;
    }

    int num_trains = read_file(file_name, tracks, trains);
    if (num_trains == 0) {
        return 1; 
    }

    // Fork a process for each train
    for (int i = 0; i < num_trains; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Erro ao criar processo");
            return 1;
        }

        if (pid == 0) {
            printf("Processo filho para o comboio %d (PID: %d)\n", trains[i].num, getpid());
            while (1) {
                update_train_position(&trains[i], tracks);
                usleep(wait_time);
            }
            exit(0); // Ensure child process exits after its loop
        }
    }

    // Parent process: update and print the board
    while (1) {
        fill_board(board, tracks, MAX_TRACKS, trains, num_trains);
        print_state(board, trains, num_trains, tracks);
        usleep(wait_time);
    }

    // Wait for all child processes to finish (this won't happen in this infinite loop)
    for (int i = 0; i < num_trains; i++) {
        wait(NULL);
    }

    // Unmap shared memory
    munmap(tracks, sizeof(Track) * MAX_TRACKS);
    munmap(trains, sizeof(Train) * MAX_TRAINS);
    munmap(board, sizeof(char) * BOARD_SIZE * BOARD_SIZE);

    return 0;
}