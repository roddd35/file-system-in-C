#include "ep3.h"

/* o bitmap tem tamanho TOTAL_BLOCKS / 8 
 * pois cada posição do vetor unsigned char
 * ocupa 1 byte (8 bits)
 * cada bit no bitmap representa um bloco de 4KB
 * como temos 25600 blocos, podemos armazenar
 * 25600 blocos * 4KB = 102400KB = 100MB
 * analogamente, 4KB * 3200bytes * 8 bits = 102400KB
 */

int isSystemMounted = 0;
int total_files = 0;
int files_list_capacity;

FileInfo* filesList;
unsigned char bitmap[TOTAL_BLOCKS / 8];
int fat[TOTAL_BLOCKS];

int main(){
    int i;
    int exit = 0;
    char* command;
    char* args[10];

    filesList = (FileInfo*)malloc(sizeof(FileInfo) * 1);
    files_list_capacity = 1;

    while(1){
        /* ler comando */
        command = readline(displayPrompt());
        if(command == NULL)
            continue;

        /* separar o comando em args */
        i = 0;
        args[i] = strtok(command, " ");
        while (args[i] != NULL && i < 10){
            i++;
            args[i] = strtok(NULL, " ");
        }

        exit = process_command(args, i);
        if(exit)
            return 0;
        command = NULL;
    }

    free(args);
    free(command);
    free(filesList);

    return 0;
}

/* mostrar o prompt */
char* displayPrompt(){
    return "{ep3}: ";
}

/* processar o comando que o usuário informou */
int process_command(char* args[], int total_parameters){
    /* criar ou carregar um sistema de arquivos */
    if(strcmp(args[0], "monta") == 0){
        /* criar um sistema de arquivos */
        initializeFileSystem();
        isSystemMounted = 1;

        /* criar o sistema de arquivos */

        /* carregar o sistema de arquivos */
        return 0;
    }

    /* salvar estado e sair do simulador */
    else if(strcmp(args[0], "sai") == 0){
        return 1;
    }

    if(isSystemMounted){
        /* criar um arquivo ou acessar um arquivo */
        if(strcmp(args[0], "toca") == 0){
            int index = fileExists(args[1]);

            /* arquivo nao existe, criar um novo */
            if(index == -1){
                if(create_file(args[1], 0))
                    printf("Arquivo criado!\n");
            }
            /* arquivo existe, modificar seu ultimo acesso */
            else
                get_current_date_time(filesList[index].acessTime, sizeof(filesList[index].acessTime));
            /* printf("Tempo de acesso atualizado para: %s\n", filesList[index].acessTime); */
        }

        /* criar um arquivo ou modificar seu acesso */
        else if(strcmp(args[0], "criadir") == 0){
            if(create_file(args[1], 1))
                printf("Diretório criado!\n");
            
        }
    }
    else
        printf("Por favor, monte um sistema de arquivos antes!\n");

    return 0;
}

/* inicializar a tabela FAT e o bitmap */
void initializeFileSystem(){
    int i;

    for(i = 0; i < TOTAL_BLOCKS; i++)
        fat[i] = 0;
    
    for(i = 0; i < TOTAL_BLOCKS/8; i++)
        bitmap[i] = 0;
}

/* retornar o horario no formato DD/MM/YY HH:MM:SS */
void get_current_date_time(char* buffer, size_t size){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%02d/%02d/%02d %02d:%02d:%02d", 
                            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
                            tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void realloc_files_list(){
    FileInfo* v = malloc(sizeof(FileInfo) * 2*total_files);

    memcpy(v, filesList, sizeof(FileInfo) * total_files);
    free(filesList);

    filesList = v;
}

/* procurar uma posição livre na tabela FAT para criar um arquivo vazio */
int find_free_FAT_position(){
    int i;
    for(i = 0; i < TOTAL_BLOCKS; i++){
        if(fat[i] == 0)
            return i;
    }
    return -1;
}

/* procurar uma posicao livre no bitmap para alocar um bloco */
int find_free_bitmap_position(){
    int i;
    for(i = 0; i < TOTAL_BLOCKS / 8; i++){
        if(bitmap[i] == 0)
            return i;
    }
    return -1;
}

/* criar um novo arquivo */
int create_file(char* filename, int isDir){
    FileInfo new_file;
    int file_index = find_free_FAT_position();    /* procurar na tabela FAT um espaço livre */
    int bitmap_index = find_free_bitmap_position();
    /* é possível criar o arquivo? */
    if(file_index == -1 || bitmap_index == -1){
        printf("[ERRO]: espaço insuficiente.\n");
        return 0;
    }

    bitmap[bitmap_index] = 1;

    /* preencher as informações do arquivo */
    get_current_date_time(new_file.creationTime, sizeof(new_file.creationTime));
    strcpy(new_file.fileName, filename);
    strcpy(new_file.acessTime, new_file.creationTime);
    strcpy(new_file.modificationTime, new_file.creationTime);

    new_file.fat_block = file_index;
    new_file.bitmap_block = bitmap_index;
    new_file.bytesSize = 0; 
    new_file.total_blocks = 1;
    new_file.is_directory = isDir;

    fat[file_index] = -1;       // marcar que um arquivo ocupa essa posicao
    bitmap[bitmap_index] = 1;   // marcar bitmap ocupado

    /* 
     * bytesSize / 4096 retorna o numero de blocos desse arquivo 
     * quando esse valor for maior que o total_blocks, criar um 
     * novo bloco na FAT e armazenar o conteudo extra lá
     * a cada 8 posições na FAT, alocar uma nova no bitmap
    */

    filesList[total_files] = new_file;
    total_files += 1;

    if(total_files == files_list_capacity){
        realloc_files_list();
        files_list_capacity *= 2;
    }
    
    /*printf("Nome do arquivo: %s\n", fat[file_index].fileName);
    printf("Ultimo acesso: %s\n", fat[file_index].acessTime);
    printf("Hora de criacao: %s\n", fat[file_index].creationTime);
    printf("Tamanho do arquivo: %d\n", fat[file_index].bytesSize);
    printf("Diretorio: %d\n", fat[file_index].isDirectory);
    printf("Bloco de inicio: %d\n", fat[file_index].currentBlock);*/

    return 1;
}

/* verifica se um arquivo existe */
int fileExists(char* filename){
    int i;
    for(i = 0; i < total_files; i++)
        if(strcmp(filesList[i].fileName, filename) == 0)
            return i;
    return -1;
}