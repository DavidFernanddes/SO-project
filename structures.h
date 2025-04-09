#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#define MAX_TRACKS 5
#define MAX_TRAINS 10
#define MAX_POSITIONS 50
#define BOARD_SIZE 10
#define MAX_POSITIONS 50


typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int num;
    int size;
    Position positionTack[MAX_POSITIONS];
} Track;

typedef struct {
    int num;
    int track;
    int position;
    int speed;
    int counter;     
} Train;

#endif