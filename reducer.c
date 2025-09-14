#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 256
#define FREQ_ARRAY_SIZE 1000
int unique_words = 0;

typedef struct {
    char word[256];
    int count;
} WordFreq;

int find_word_in_array(WordFreq* freq_array, int size, char* word) {
    for(int i = 0; i < size; i++) {
        if (strcmp(freq_array[i].word, word) == 0) {
            return i;  // Found it! Return the index
        }
    }
    return -1;  // Not found
}

void add_new_word(WordFreq* freq_array, int* size, char* word, int count) {
    strcpy(freq_array[*size].word, word);  // Copy the word
    freq_array[*size].count = count;           // Set initial count
    (*size)++;                             // Increment size counter
    printf("✅ Added new word: '%s' at index %d\n", word, *size - 1);
}

void increment_word_count(WordFreq* freq_array, int index, int count) {
    freq_array[index].count += count;             // Increment the count
    printf("📈 Incremented '%s' count to %d\n", 
           freq_array[index].word, freq_array[index].count);
}

void print_frequency_table(WordFreq* freq_array, int size) {
    printf("\n=== WORD FREQUENCY TABLE ===\n");
    for (int i = 0; i < size; i++) {
        printf("%s: %d\n", freq_array[i].word, freq_array[i].count);
    }
    printf("=============================\n\n");
}

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
    WordFreq freq_array[FREQ_ARRAY_SIZE];

    while(fgets(line, sizeof(line), fd) != NULL) {
        if (line[0] == '#') continue;

        char word[256];
        int count;

        if (sscanf(line, "%s %d", word, &count) == 2) {
            int word_reducer = hash_word_to_reducer(word, total_reducers);

            if (word_reducer == reducer_id) {
                printf("✅ MINE! word='%s', count=%d\n", word, count);

                int word_index = find_word_in_array(freq_array, unique_words, word);

                if (word_index != -1) {
                    increment_word_count(freq_array, word_index, count);
                } else {
                    add_new_word(freq_array, &unique_words, word, count);
                }
            } else {
                printf("❌ word='%s' belongs to reducer %d\n", word, word_reducer);
            }
            printf("Found: word='%s', count=%d\n", word, count);
        }
    }
    fclose(fd);
}

int main(int argc, char* argv[]) {
    int reducer_id = atoi(argv[1]);
    int total_reducers = atoi(argv[2]);

    // read all the files that have a pattern output_file_*.txt and put them as variables into read_mapper_files()
}