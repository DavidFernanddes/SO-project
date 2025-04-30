#include <stdio.h>
#include <stdlib.h>
#include "input.h"

// Função para ler os dados do ficheiro e preencher as estruturas de pistas e comboios
int read_file(char *file_name, Track tracks[], Train trains[]) {
    // Open the file for reading
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Erro ao abrir o ficheiro\n");
        return 0;
    }

    int num_tracks, num_trains;
    fscanf(file, "%d %d", &num_tracks, &num_trains);

    // Read track data
    for (int t = 0; t < num_tracks; t++) {
        fscanf(file, "%d %d", &tracks[t].num, &tracks[t].size);

        // Read positions for the track
        for (int p = 0; p < tracks[t].size; p++) {
            fscanf(file, "%d %d", &tracks[t].position[p].x, &tracks[t].position[p].y);
        }
    }

    // Read train data
    for (int c = 0; c < num_trains; c++) {
        fscanf(file, "%d %d %d %d", &trains[c].num, &trains[c].track, 
               &trains[c].position, &trains[c].speed);
        trains[c].counter = 0; // Initialize the train's counter
    }

    fclose(file);
    return num_trains;
}
 
// Função para preencher o tabuleiro com as pistas e os comboios
void fill_board(char board[BOARD_SIZE][BOARD_SIZE], Track tracks[], int num_tracks, Train trains[], int num_trains) {
    // Inicializa o tabuleiro com espaços vazios
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }

    // Preenche o tabuleiro com as pistas
    for (int t = 0; t < num_tracks; t++) {
        if (t < 0 || t >= MAX_TRACKS) continue; // Verifica limites de pistas

        int size = tracks[t].size;
        if (size < 0 || size > MAX_POSITIONS) continue; // Verifica limites de posições

        for (int p = 0; p < size; p++) {
            int x = tracks[t].position[p].x;
            int y = tracks[t].position[p].y;

            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                board[y][x] = 'X';
            }
        }
    }

    // Preenche o tabuleiro com os comboios
    for (int c = 0; c < num_trains; c++) {
        if (c < 0 || c >= MAX_TRAINS) continue; // Verifica limites de comboios

        int track_idx = trains[c].track;
        int pos_idx = trains[c].position;

        if (track_idx < 0 || track_idx >= num_tracks) continue; // Verifica se a pista é válida

        int track_size = tracks[track_idx].size;
        if (pos_idx < 0 || pos_idx >= track_size) continue; // Verifica se a posição é válida

        int x = tracks[track_idx].position[pos_idx].x;
        int y = tracks[track_idx].position[pos_idx].y;

        // Verifica se a posição está dentro dos limites do tabuleiro
        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
            board[y][x] = '0' + (trains[c].num % 10);
        }
    }
}

// Função para imprimir o estado atual do tabuleiro e dos comboios
void print_state(char board[BOARD_SIZE][BOARD_SIZE], Train trains[], int num_trains, Track tracks[]) {
    // Imprime o estado do tabuleiro com as pistas e comboios
    printf("Tracks\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
        
    // Imprime o estado de cada comboio
    printf("Trains:\n");
    for (int c = 0; c < num_trains; c++) {
        int track_idx = trains[c].track;
        int pos_idx = trains[c].position;
        int x = tracks[track_idx].position[pos_idx].x;
        int y = tracks[track_idx].position[pos_idx].y;
        
        printf("T%d (%d,%d)\n", trains[c].num, x, y);
    }
    printf("\n");
}

void update_train_position(Train* train, Track tracks[]) {
    // Incrementa o contador
    train->counter++;
    
    // Verifica se é altura de mover o comboio
    if (train->counter % train->speed == 0) {
        // Incrementa a posição do comboio
        train->position++;
        
        // Se chegou ao final da pista, volta ao início
        if (train->position >= tracks[train->track].size) {
            train->position = 0;
        }
    }
}