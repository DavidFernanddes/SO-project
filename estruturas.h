

// Definição de estruturas

typedef struct
{
    int coordenadaX;
    int coordenadaY;
} Posição;

typedef struct
{
    int numeroDaPista;
    int tamanhoPista;
    Posição posicoesPista[50];
} Pista;

typedef struct
{
    int idComboio;
    int indicePosicaoInicial; // mambo da posicao inicial no array pocicao pista
    int velocidadeComboio;    // valor para saber quando comboio anda em turnos
    int contadorTurnos;
    int pista;              //indice da pista onde o train esta          
} Comboio;
