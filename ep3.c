#include "ep3.h"

/* o bitmap tem tamanho TOTAL_BLOCKS / 8 
 * pois cada posição do vetor unsigned char
 * ocupa 1 byte (8 bits)
 * cada bit no bitmap representa um bloco de 4KB
 * como temos 25600 blocos, podemos armazenar
 * 25600 blocos * 4KB = 102400KB = 100MB
 * analogamente, 4KB * 3200bytes * 8 bits = 102400KB
 */

unsigned char bitmap[TOTAL_BLOCKS / 8];
FATEntry fat[TOTAL_BLOCKS];

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
    /* salvar estado e sair do simulador */
    if(strcmp(args[0], "sai") == 0){
        return 1;
    }

    /* criar um arquivo ou acessar um arquivo */
    else if(strcmp(args[0], "toca") == 0){
        if(!validName(args[1]))
            return 0;
    }
    
    /* criar ou carregar um sistema de arquivos */
    else if(strcmp(args[0], "monta") == 0){
        /* criar um sistema de arquivos */
        initializeFileSystem();

        /* criar o sistema de arquivos */

        /* carregar o sistema de arquivos */
        return 0;
    }

    /* criar um arquivo ou modificar seu acesso */
    else if(strcmp(args[0], "toca") == 0){
        FileS new_file;
        int file_index = find_free_file_entry();    /* procurar na tabela FAT um espaço livre */
        
        /* é possível criar o arquivo? */
        if(file_index == -1){
            printf("[ERRO]: espaço insuficiente.\n");
            return 0;
        }
        if(!validName(args[1]))
            return 0;

        strcpy(new_file.fileName, args[1]);
        get_current_date_time(new_file.creationTime, sizeof(new_file.creationTime));
        strcpy(new_file.acessTime, new_file.creationTime);
        strcpy(new_file.modificationTime, new_file.creationTime);
        new_file.bytesSize = 0;
    }

    return 0;
}

/* verificar se o nome de um arquivo é válido */
int validName(char* fileName){
    int i = 0;

    while(fileName[i] != 0){
        if(fileName[i] == '/'){
            printf("[ERRO]: Nome de arquivo inválido!\n");
            return 0;
        }
        i++;
    }
    return 1;
}

/* inicializar a tabela FAT e o bitmap */
void initializeFileSystem(){
    int i;
    /* char dateTime[20];
    
    get_current_date_time(dateTime, sizeof(dateTime)); */

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
int find_free_file_entry(){
    int i;
    for(i = 0; i < TOTAL_BLOCKS; i++){
        if(strcmp(fat[i].fileName, "") == 0)
            return i;
    }
    return -1;
}