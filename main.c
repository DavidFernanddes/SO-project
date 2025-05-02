#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#ifdef __linux__
    #include <sys/prctl.h>
#endif
#include "structures.h"
#include "input.h"

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Usage: %s <input_file> <wait_time_ms> [file_type]\n", argv[0]);
        return 1;
    }

    char* file_name = argv[1];
    int wait_time = atoi(argv[2]) * 1000;
    int file_type = (argc == 4) ? atoi(argv[3]) : 0;

    Resources resources = {0};

    init_shared_memory(&resources);
    init_semaphores(&resources);

    int num_trains = read_file(file_name, resources.tracks, resources.trains, file_type);
    if (num_trains == 0) {
        cleanup(&resources);
        return 1;
    }

    spawn_train_processes(&resources, num_trains, wait_time, file_type);
    run_simulation(&resources, wait_time);

    for (int i = 0; i < num_trains; i++) {
        wait(NULL);
    }

    cleanup(&resources);
    return 0;
}
