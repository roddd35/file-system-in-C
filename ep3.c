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
unsigned char bitmap[TOTAL_BLOCKS / 8];
FileInfo fat[TOTAL_BLOCKS];

int main(){
    int i;
    int exit = 0;
    char* command;
    char* args[10];

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
    free(command);
    free(args);

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
            int created;
            int index = fileExists(args[1]);

            /* arquivo nao existe, criar um novo */
            if(index == -1){
                created = create_file(args[1], 0);
                if(!created)
                    printf("[ERRO]: criação do arquivo\n");
                else
                    printf("Arquivo criado!\n");
                return 0;
            }
            /* arquivo existe, modificar seu ultimo acesso */
            get_current_date_time(fat[index].acessTime, sizeof(fat[index].acessTime));
            /* printf("Tempo de acesso atualizado para: %s\n", fat[index].acessTime); */
        }

        /* criar um arquivo ou modificar seu acesso */
        else if(strcmp(args[0], "criadir") == 0){
            int created;
            created = create_file(args[1], 1);
            if(!created){
                printf("[ERRO]: criação do arquivo\n");
                return 0;
            }
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

    for(i = 0; i < TOTAL_BLOCKS; i++){
        fat[i].currentBlock = i;
        fat[i].nextBlock = FAT_FREE;
        strcpy(fat[i].fileName, "");
    }
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

/* procurar uma posição livre na tabela FAT para criar um arquivo vazio */
int find_free_FAT_position(){
    int i;
    for(i = 0; i < TOTAL_BLOCKS; i++){
        if(strcmp(fat[i].fileName, "") == 0)
            return i;
    }
    return -1;
}

/* procurar uma posicao livre no bitmap para alocar um bloco */
int find_free_bitmap_position(){
    int i;
    for(i = 0; i < TOTAL_BLOCKS; i++){
        if(bitmap[i] == 0)
            return i;
    }
    return -1;
}

/* criar um novo arquivo */
int create_file(char* filename, int isDir){
    FileInfo new_file;
    int file_index = find_free_FAT_position();    /* procurar na tabela FAT um espaço livre */
    
    /* é possível criar o arquivo? */
    if(file_index == -1){
        printf("[ERRO]: espaço insuficiente.\n");
        return 0;
    }

    /* preencher as informações do arquivo */
    get_current_date_time(new_file.creationTime, sizeof(new_file.creationTime));
    strcpy(new_file.fileName, filename);
    strcpy(new_file.acessTime, new_file.creationTime);
    strcpy(new_file.modificationTime, new_file.creationTime);
    new_file.bytesSize = 0;
    new_file.currentBlock = file_index;
    new_file.nextBlock = -1;    /* arquivo vazio, acaba ali mesmo */
    new_file.isDirectory = isDir;

    fat[file_index] = new_file;

    
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
    for(i = 0; i < TOTAL_BLOCKS; i++)
        if(strcmp(fat[i].fileName, filename) == 0)
            return i;
    return -1;
}