#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define OUTPUT_SUFFIX "_result.txt"

// Data passed to each thread
typedef struct {
    char filename[256]; // input file path
    long lines;
    long words;
    long characters;
    int success; // 1 if the file was processed successfully, 0 if not
} FileTask;

// Utility functions

// Builds the output filename by appending "_result.txt" to the input name
void buildOutputFilename(const char *inputName, char *outputName, size_t size) {
    snprintf(outputName, size, "%s%s", inputName, OUTPUT_SUFFIX);
}

// Core analysis logic

// Reads the given file and counts lines, words, and characters.
// Returns 1 on success, 0 if the file could not be opened.
int analyzeFile(FileTask *task) {
    FILE *fp = fopen(task->filename, "r");
    if (fp == NULL) {
        task->success = 0;
        return 0;
    }

    long lines = 0, words = 0, characters = 0;
    int inWord = 0;
    int c;

    while ((c = fgetc(fp)) != EOF) {
        characters++;

        if (c == '\n') {
            lines++;
        }

        // A "word" is any run of non-whitespace characters
        if (c == ' ' || c == '\n' || c == '\t') {
            inWord = 0;
        } else if (!inWord) {
            inWord = 1;
            words++;
        }
    }

    // If the file does not end with a newline, the last line still counts
    if (characters > 0) {
        fseek(fp, -1, SEEK_END);
        int lastChar = fgetc(fp);
        if (lastChar != '\n') {
            lines++;
        }
    }

    fclose(fp);

    task->lines = lines;
    task->words = words;
    task->characters = characters;
    task->success = 1;
    return 1;
}

// Writes the analysis results for one file to its own output file
void writeResults(FileTask *task) {
    char outputName[300];
    buildOutputFilename(task->filename, outputName, sizeof(outputName));

    FILE *fp = fopen(outputName, "w");
    if (fp == NULL) {
        printf("Error: Could not create output file for '%s'.\n", task->filename);
        return;
    }

    fprintf(fp, "Analysis of file: %s\n", task->filename);
    fprintf(fp, "Lines: %ld\n", task->lines);
    fprintf(fp, "Words: %ld\n", task->words);
    fprintf(fp, "Characters: %ld\n", task->characters);

    fclose(fp);
}

// Thread entry point

// This is the function each thread runs. It analyzes one file, prints a status message, and writes the results to disk.
void *processFile(void *arg) {
    FileTask *task = (FileTask *)arg;

    printf("[Thread] Started processing: %s\n", task->filename);

    if (!analyzeFile(task)) {
        printf("[Thread] Error: Could not open '%s'. Skipping.\n", task->filename);
        return NULL;
    }

    writeResults(task);

    printf("[Thread] Finished '%s' -> Lines: %ld, Words: %ld, Characters: %ld\n",
           task->filename, task->lines, task->words, task->characters);

    return NULL;
}

// Main program

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return 1;
    }

    int fileCount = argc - 1;

    // One thread and one task struct per input file
    pthread_t threads[fileCount];
    FileTask tasks[fileCount];

    int i;
    for (i = 0; i < fileCount; i++) {
        strncpy(tasks[i].filename, argv[i + 1], sizeof(tasks[i].filename) - 1);
        tasks[i].filename[sizeof(tasks[i].filename) - 1] = '\0';
        tasks[i].lines = 0;
        tasks[i].words = 0;
        tasks[i].characters = 0;
        tasks[i].success = 0;
    }

    printf("Starting processing of %d file(s) using %d thread(s)...\n\n", fileCount, fileCount);

    // Create one thread per file
    for (i = 0; i < fileCount; i++) {
        int result = pthread_create(&threads[i], NULL, processFile, &tasks[i]);
        if (result != 0) {
            printf("Error: Failed to create thread for '%s'.\n", tasks[i].filename);
        }
    }

    // Wait for all threads to finish
    for (i = 0; i < fileCount; i++) {
        pthread_join(threads[i], NULL);
    }

    // Summary
    printf("\n===== Processing Summary =====\n");
    int successCount = 0;
    for (i = 0; i < fileCount; i++) {
        if (tasks[i].success) {
            printf("%s: OK (output -> %s%s)\n", tasks[i].filename, tasks[i].filename, OUTPUT_SUFFIX);
            successCount++;
        } else {
            printf("%s: FAILED (file missing or unreadable)\n", tasks[i].filename);
        }
    }
    printf("Successfully processed %d of %d file(s).\n", successCount, fileCount);

    return 0;
}

