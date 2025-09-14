#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>

int get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

void spawn_mapper(const char* input_file, int start, int end, int mapper_id) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child: become mapper
        char start_str[32], end_str[32], id_str[32];
        sprintf(start_str, "%d", start);
        sprintf(end_str, "%d", end);
        sprintf(id_str, "%d", mapper_id);

        char* args[] = {"./mapper", (char*)input_file, start_str, end_str, id_str, NULL};

        execv("./mapper", args);

        // if we get here, exec failed!
        perror("Failed to execute mapper");
        exit(1);
    } else if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
}

void spawn_reducer(int reducer_id, int total_reducers) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child: become reducer

        char id_str[32], total_str[32];
        sprintf(id_str, "%d", reducer_id);
        sprintf(total_str, "%d", total_reducers);

        execv("./reducer", (char*[]){"./reducer", id_str, total_str, NULL});
        perror("Failed to exec reducer");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf ("Usage: %s <input_file> <num_mappers>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    int num_mappers = atoi(argv[2]);

    // Step 1: get file size for work division
    int total_bytes = get_file_size(input_file);
    if (total_bytes < 0) {
        printf("❌ Error: Cannot read file %s\n", input_file);
        return 1;
    }

    // Step 2: calculate work chunks
    int chunk_size = total_bytes / num_mappers;

    // Step 3: spawn all mapper processes
    for(int i = 0; i < num_mappers; i++) {
        int start = i * chunk_size;
        int end = i == (num_mappers - 1) ? total_bytes : start + chunk_size;

        spawn_mapper(input_file, start, end, i);

    for(int i = 0; i < num_mappers; i++) {
        int status;
        pid_t finished_pid = wait(&status);

        if (WIFEXITED(status)) {
            printf("✅ Mapper process %d completed successfully\n", finished_pid);
        } else {
            printf("❌ Mapper process %d failed\n", finished_pid);
        }
    }

    int num_reducers = 3;

    for(int i = 0; i < num_reducers; i++) {
        spawn_reducer(i, num_reducers);
    }

    // Wait for all reducers to complete
    for(int i = 0; i < num_reducers; i++) {
        int status;
        wait(&status);
        printf("Reducer process completed\n");
    }
    
    return 0;
    }
}