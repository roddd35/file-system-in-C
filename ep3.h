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
#define maxDir 1000 /* maximo de diretorios na arvore (db) */
#define maxFiles 255 /* maximo de arquivos em cada diretorio na arvore (db) */

// estruturas de dados
typedef struct{
    char fileName[FILENAME_LENGTH];
    int fat_block;
    int bytesSize;
    int is_directory;
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

void update_db();
void free_bitmap(int i);
void imprime_diretorios();
void unmount_file_system();
void show_file(char* filename);
void print_data(FileInfo fInfo);
void free_fat_list(int firstPos);
void list_directory(char* dirname);
void set_bitmap(int block, int value);
void initializeFileSystem(char* args[]);
void update_access_time(char *filename);
void save_file_info(FileInfo* fileInfo);
void get_current_date_time(char* buffer, size_t size);
void getDirectoryPath(char* filepath, char* directory);

FileInfo set_file_config(char* filename, int isDir, int fi);

// desmonta -> salvar a FAT e bitmap no arquivo binario
// monta -> imprimir a arvore e carregar o bitmap e FAT
// status
// atualizaDB e busca string
// apagadir
// copia origem destino (verificar o espaco livre antes de salvar o arquivo externo)