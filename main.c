#include <stdio.h>
#include "estruturas.h"

// valores que nao modificam, se quiserem depois ponham verificações ($_$)
#define NUM_PISTAS 5
#define NUM_COMBOIOS 10
#define LINHAS_MATRIZ 10
#define COLUNAS_MATRIZ 10

int main()
{
    // definir arrays estaticos
    Pista ArrayPistas[NUM_PISTAS];
    Comboio ArrayComboios[NUM_COMBOIOS];
    int matriz[LINHAS_MATRIZ][COLUNAS_MATRIZ];
}

void lerDados(char *trabalhoSO, Pista *pistas, Comboio *arraycomboios)
{
    FILE *file = fopen(trabalhoSO, "r");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro");
        return;
    }
}