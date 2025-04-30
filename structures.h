#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#define MAX_TRACKS 5
#define MAX_TRAINS 10
#define MAX_POSITIONS 50
#define BOARD_SIZE 10
#define MAX_STOPS 10

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int stop_index;        // Index of the position in the track where the stop is located
    int semaphore_index;   // Index of the semaphore used for synchronization
} Stop;

typedef struct {
    int num;                     // Track number
    int size;                    // Number of positions in the track
    Position position[MAX_POSITIONS]; // Array of positions
    int num_stops;               // Number of stops in the track
    Stop stops[MAX_STOPS];       // Array of stops
} Track;

typedef struct {
    int num;         // Train number
    int track;       // Track index where the train is located
    int position;    // Current position index in the track
    int speed;       // Train speed (move every 'speed' cycles)
    int counter;     // Counter for train movement
    char state;      // Train state: 'M' (moving), 'S' (at stop), 'W' (waiting)
    int section;     // Current section (corresponds to the last stop passed)
    int last_stop;   // Index of the last stop passed
    int next_stop;   // Index of the next stop
} Train;

#endif