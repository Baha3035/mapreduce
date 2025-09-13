#include <stdio.h>
#include <string.h>

#define MAX_LINE_SIZE 256

int hash_word_to_reducer(char* word, int num_reducers) {
    int sum = 0;
    for(int i = 0; word[i]; i++) {
        sum += word[i];
    }

    printf("sum %d\n", sum);
    return sum % num_reducers;
}

void read_mapper_files(char* filename, int reducer_id, int total_reducers) {
    FILE *fd = fopen(filename, "r");
    if (!fd) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }

    char line[MAX_LINE_SIZE];
    while(fgets(line, sizeof(line), fd) != NULL) {
        if (line[0] == '#') continue;

        char word[256];
        int count;

        if (sscanf(line, "%s %d", word, &count) == 2) {
            int word_reducer = hash_word_to_reducer(word, total_reducers);

            if (word_reducer == reducer_id) {
                printf("✅ MINE! word='%s', count=%d\n", word, count);
            } else {
                printf("❌ word='%s' belongs to reducer %d\n", word, word_reducer);
            }
            printf("Found: word='%s', count=%d\n", word, count);
        }
    }
    fclose(fd);
}

int main() {
    read_mapper_files("output_file_0.txt", 0, 2);
}