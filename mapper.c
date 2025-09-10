#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int start_byte, end_byte, mapper_id;
char* filename;
FILE* input_file;

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
    int word_count = 0;
    int current_pos = start_byte;

    while(fscanf(input_file, "%255s", word) == 1) {
        current_pos = ftell(input_file);
        if (current_pos > end_byte) {
            break;  // Stop if we've gone past our boundary
        }
        word_count++;
        // Convert to lowercase
        for(int i = 0; word[i]; i++) {
            word[i] = tolower(word[i]);
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
        fprintf(output_file, "Mapper %d processed %d words from bytes %d-%d\n", 
                mapper_id, word_count, start_byte, end_byte);
        printf("Success");
        fclose(output_file);
    }
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