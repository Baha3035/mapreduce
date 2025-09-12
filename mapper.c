#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int start_byte, end_byte, mapper_id;
char* filename;
FILE* input_file;
int FREQ_ARRAY_SIZE = 1000;

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

// STEP 3: Function to add a new word to our array
// This gets called when we encounter a word for the first time
void add_new_word(WordFreq* freq_array, int* size, char* word) {
    strcpy(freq_array[*size].word, word);  // Copy the word
    freq_array[*size].count = 1;           // Set initial count
    (*size)++;                             // Increment size counter
    printf("✅ Added new word: '%s' at index %d\n", word, *size - 1);
}

// STEP 4: Function to increment count of existing word
// This gets called when we see a word we've seen before
void increment_word_count(WordFreq* freq_array, int index) {
    freq_array[index].count++;             // Increment the count
    printf("📈 Incremented '%s' count to %d\n", 
           freq_array[index].word, freq_array[index].count);
}

// STEP 5: Function to print our frequency table (for testing)
void print_frequency_table(WordFreq* freq_array, int size) {
    printf("\n=== WORD FREQUENCY TABLE ===\n");
    for (int i = 0; i < size; i++) {
        printf("%s: %d\n", freq_array[i].word, freq_array[i].count);
    }
    printf("=============================\n\n");
}

int is_word_separator(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == ',' ||
        c == '.' || c == ';' || c == '!' || c == '?' ||
        c == '"' || c == '\'' || c == '(' || c == ')');
}

void find_word_boundary() {
    if (start_byte == 0) {
        return;
    }

    printf("🔍 Mapper %d: Adjusting start from byte %d...\n", mapper_id, start_byte);

    input_file = fopen(filename, "r");
    if (!input_file) {
        printf("❌ Mapper %d: Cannot open file %s\n", mapper_id, filename);
        exit(1);
    }

    while(start_byte > 0) {
        start_byte--;
        fseek(input_file, start_byte, SEEK_SET);

        char c = fgetc(input_file);

        if (is_word_separator(c)) {
            // if we find the separator, next character starts a word
            start_byte++;
            break;
        }
    }

    fclose(input_file);
    printf("✅ Mapper %d: Adjusted start to byte %d\n", mapper_id, start_byte);
}

void count_words_in_chunk() {
    input_file = fopen(filename, "r");
    if (!input_file) {
        printf("❌ Mapper %d: Cannot open file %s\n", mapper_id, filename);
        exit(1);
    }

    fseek(input_file, start_byte, SEEK_SET);

    char word[256];
    WordFreq freq_array[FREQ_ARRAY_SIZE];
    int unique_words = 0; 
    int current_pos = start_byte;

    while(fscanf(input_file, "%255s", word) == 1) {
        current_pos = ftell(input_file);
        if (current_pos > end_byte) {
            break;  // Stop if we've gone past our boundary
        }
        
        // Convert to lowercase
        for(int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
        }

        int word_index = find_word_in_array(freq_array, unique_words, word);
        if (word_index != -1) {
            increment_word_count(freq_array, word_index);
        } else {
            add_new_word(freq_array, &unique_words, word);
        }

        printf("📝 Mapper %d found word: '%s' at position %d (end_byte: %d)\n", 
       mapper_id, word, current_pos, end_byte);
    }
    fclose(input_file);

    // Write results to output file
    char output_filename[256];
    sprintf(output_filename, "output_file_%d.txt", mapper_id);

    FILE* output_file = fopen(output_filename, "w");
    if (output_file) {
        fprintf(output_file, "Mapper %d processed %d unique words from bytes %d-%d\n", 
                mapper_id, unique_words, start_byte, end_byte);

        for (int i = 0; i < unique_words; i++) {
            fprintf(output_file, "%s %d\n", freq_array[i].word, freq_array[i].count);
        }
        
        printf("Success");
        fclose(output_file);
    }
    print_frequency_table(freq_array, unique_words);
}

int main(int argc, char* argv[]) {
    filename = argv[1];
    start_byte = atoi(argv[2]);
    end_byte = atoi(argv[3]);
    mapper_id = atoi(argv[4]);

    printf("🚀 Mapper %d starting: file=%s, bytes=%d-%d\n", 
           mapper_id, filename, start_byte, end_byte);

    find_word_boundary();
    count_words_in_chunk();

    printf("🎉 Mapper %d completed!\n", mapper_id);
    
    return 0;
}