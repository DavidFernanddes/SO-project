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
#define MAX_SEMAPHORES 10
#define SEM_NAME_SIZE 64

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int stop_index;
    int semaphore_index;
} Stop;

typedef struct {
    int num;
    int size;
    Position position[MAX_POSITIONS];
    int num_stops;
    Stop stops[MAX_STOPS];
} Track;

typedef struct {
    int num;
    int track;
    int position;
    int speed;
    int counter;
    char state;
    int section;
    int last_stop;
    int next_stop;
} Train;

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

#endif