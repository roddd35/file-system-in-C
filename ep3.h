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
    int fat_block;
    int bitmap_block[3200];
    int total_bits;
    int bytesSize;  /* diretorios nao tem esse atributo */
    int total_blocks;
    int total_files_this_row;
    int row_capacity;
    int is_directory;
    char fileName[FILENAME_LENGTH];
    char creationTime[20];
    char modificationTime[20];
    char acessTime[20];
}FileInfo;

char* displayPrompt();
int find_free_FAT_position();
int find_free_bitmap_position();
int fileExists(char* filename, int isDir);
int create_file(char* filename, int isDir);
int process_command(char* args[], int total_parameters);
int erase_file(char* filename, int dirIndex);
void initializeFileSystem();
void realloc_dir_tree();
void realloc_dir_list(int index);
void get_current_date_time(char* buffer, size_t size);
void getDirectoryPath(char* filepath, char* directory);
void show_file(char* filename);
void list_directory(char* dirname);
void free_fat_list(int firstPos);
void free_bitmap(int bitmapList[], int total_bits);
void imprime_diretorios();
FileInfo set_file_config(char* filename, int isDir, int fi, int bi);