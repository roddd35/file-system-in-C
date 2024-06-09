#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#define FAT_FREE 0
#define FAT_EOF -1
#define FILENAME_LENGTH 255
#define TOTAL_BLOCKS 25600
#define maxDir 1000 /* maximo de diretorios na arvore db */
#define maxFiles 255 /* maximo de arquivos em cada diretorio na arvore db */
#define KB 1024
#define maxBytesSize 4096
#define standardBytesSize 327
#define totalAvailableKB 102400

// estruturas de dados
typedef struct{
    char fileName[FILENAME_LENGTH];
    int fat_block;
    int bytesSize;
    int is_directory;
    char creationTime[20];
    char modificationTime[20];
    char accessTime[20];
    char content[maxBytesSize - standardBytesSize];
}FileInfo;

typedef struct{
    char fileName[FILENAME_LENGTH];
    int fat_block;
    int is_directory;
}shortFileInfo;

// funcoes
char* displayPrompt();

int find_free_FAT_position();
int find_free_bitmap_position();
int erase_dir(char* dirname);
int erase_file(char* filename);
int fileExists(char* filename);
int create_file(char* filename, int isDir);
int copy_file(char* original_filename, char* filename);
int process_command(char* args[], int total_parameters);

void update_db();
void print_status();
void print_dir_tree();
void free_bitmap(int i);
void unmount_file_system();
void show_file(char* filename);
void print_data(FileInfo fInfo);
void free_fat_list(int firstPos);
void search_string(char* string);
void list_directory(char* dirname);
void set_bitmap(int block, int value);
void initializeFileSystem(char* args[]);
void update_access_time(char *filename);
void save_file_info(FileInfo* fileInfo);
void get_filename(char *path, char *buffer);
void get_current_date_time(char* buffer, size_t size);
void getDirectoryPath(char* filepath, char* directory);
void print_directory(shortFileInfo f[], char dirName[], int file_count, int level, int currentIndex);

FileInfo set_file_config(char* filename, int isDir, int fi);

// copia origem destino (verificar o espaco livre antes de salvar o arquivo externo)
// mostra arquivo