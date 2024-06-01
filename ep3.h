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
    int isDirectory;
    int nextBlock;
    int currentBlock;
    char fileName[FILENAME_LENGTH];
    char creationTime[20];
    char modificationTime[20];
    char acessTime[20];
}FileInfo;

char* displayPrompt();
int find_free_FAT_position();
int find_free_bitmap_position();
int fileExists(char* filename);
int create_file(char* filename, int isDir);
int process_command(char* args[], int total_parameters);
void initializeFileSystem();
void get_current_date_time(char* buffer, size_t size);