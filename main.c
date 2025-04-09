#include <stdio.h>
#include "structures.h"

void lerDados(char *fileTxt);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: %s <ficheiro_de_entrada> <tempo_de_espera_ms>\n", argv[0]);
        return 1;
    }

    Track ArrayTracks[MAX_TRACKS];
    Train ArrayTrains[MAX_TRAINS];
    //int matrix[LINES_MATRIX][COLUMN_MATRIX];
    printf("It works");
    return 0;
}

void lerDados(char *fileTxt /*, Track *tracks, Train *matrixTrains*/)
{
    FILE *file = fopen(fileTxt, "r");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro");
        return;
    }
    char myString[100];
    fgets(myString, 100, file);
    printf("%s", myString);
    fclose(file);
}