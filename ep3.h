#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>

#define FAT_FREE 0
#define FAT_EOF -1
#define FILENAME_LENGTH 256
#define TOTAL_BLOCKS 25600

typedef struct{
    int bytesSize;  /* diretorios nao tem esse atributo */
    char fileName[FILENAME_LENGTH];
    char creationTime[20];
    char modificationTime[20];
    char acessTime[20];
}FileS;

typedef struct{
    int nextBlock;
    int currentBlock;
    char fileName[FILENAME_LENGTH];
}FATEntry;

char* displayPrompt();
void initializeFileSystem();
int validName(char* fileName);
int process_command(char* args[], int total_parameters);
int find_free_file_entry();
void get_current_date_time(char* buffer, size_t size);