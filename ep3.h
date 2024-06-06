#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#define FAT_FREE 0
#define FAT_EOF -1
#define FILENAME_LENGTH 255
#define TOTAL_BLOCKS 25600

// estruturas de dados
typedef struct{
    char fileName[FILENAME_LENGTH];
    int fat_block;
    int bytesSize;
    int is_directory;
    char creationTime[20];
    char modificationTime[20];
    char accessTime[20];
}fileAux;

typedef struct{
    int fat_block;
    int bitmap_block[3200];
    int total_bits;
    int bytesSize;  /* diretorios nao tem esse atributo */
    int total_blocks;
    int total_files_this_row;
    int is_directory;
    char fileName[FILENAME_LENGTH];
    char creationTime[20];
    char modificationTime[20];
    char accessTime[20];
}FileInfo;

// funcoes
char* displayPrompt();

int find_free_FAT_position();
int find_free_bitmap_position();
int fileExists(char* filename);
int create_file(char* filename, int isDir);
int erase_file(char* filename, int dirIndex);
int process_command(char* args[], int total_parameters);

void imprime_diretorios();
void unmount_file_system();
void show_file(char* filename);
void free_fat_list(int firstPos);
void list_directory(char* dirname);
void initializeFileSystem(char* args[]);
void update_access_time(char *filename);
void free_bitmap(int bitmapList[], int total_bits);
void get_current_date_time(char* buffer, size_t size);
void getDirectoryPath(char* filepath, char* directory);
void save_file_info(FileInfo* fileInfo);

FileInfo set_file_config(char* filename, int isDir, int fi, int bi);