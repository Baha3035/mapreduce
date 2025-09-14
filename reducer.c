#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fnmatch.h>

#define MAX_LINE_SIZE 256
#define FREQ_ARRAY_SIZE 1000

typedef struct {
    char word[256];
    int count;
} WordFreq;

int unique_words = 0;

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

void read_mapper_files(WordFreq* freq_array, char* filename, int reducer_id, int total_reducers) {
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
    struct dirent *entry;
    char pattern[] = "output_file_*.txt";

    WordFreq freq_array[FREQ_ARRAY_SIZE];
    
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Error opening directory");
    }

    while((entry = readdir(dir)) != NULL) {
        if (fnmatch(pattern, entry->d_name, 0) == 0) {
            printf("%s\n", entry->d_name);
            read_mapper_files(freq_array, entry->d_name, reducer_id, total_reducers);
        }
    }

    closedir(dir);

    // Write final results after processing all files
    char output_filename[256];
    sprintf(output_filename, "reducer_output_%d.txt", reducer_id);

    FILE* output_file = fopen(output_filename, "w");
    if (output_file) {
        fprintf(output_file, "# Reducer %d: Final word frequencies\n", reducer_id);
        fprintf(output_file, "# Total unique words: %d\n", unique_words);
        
        for (int i = 0; i < unique_words; i++) {
            fprintf(output_file, "%s %d\n", freq_array[i].word, freq_array[i].count);
        }
        
        fclose(output_file);
        printf("Results written to %s\n", output_filename);
    }
}