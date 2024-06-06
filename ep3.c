#include "ep3.h"

/* o bitmap tem tamanho TOTAL_BLOCKS / 8 
 * pois cada posição do vetor unsigned char
 * ocupa 1 byte (8 bits)
 * cada bit no bitmap representa um bloco de 4KB
 * como temos 25600 blocos, podemos armazenar
 * 25600 blocos * 4KB = 102400KB = 100MB
 * analogamente, 4KB * 3200bytes * 8 bits = 102400KB
 */

/* 
 * bytesSize / 4096 retorna o numero de blocos desse arquivo 
 * quando esse valor for maior que o total_blocks, criar um 
 * novo bloco na FAT e armazenar o conteudo extra lá
 * a cada 8 posições na FAT, alocar uma nova no bitmap
 */

int isSystemMounted = 0;
int total_dirs = 0;
char* mount_file;

FileInfo dirTree[1000][255];
int fat[TOTAL_BLOCKS];
uint8_t bitmap[TOTAL_BLOCKS / 8];

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

    // free(args);
    free(command);

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
        mount_file = args[1];
        initializeFileSystem(args);
        isSystemMounted = 1;

        return 0;
    }

    /* salvar estado e sair do simulador */
    else if(strcmp(args[0], "sai") == 0){
        return 1;
    }

    if(isSystemMounted){
        /* criar um arquivo ou acessar um arquivo */
        if(strcmp(args[0], "toca") == 0){ 
            /* arquivo nao existe, criar um novo */ 
            if(!fileExists(args[1])){
                if(create_file(args[1], 0))
                    printf("Arquivo criado!\n");
            }
            /* arquivo existe, modificar seu ultimo acesso */
            else
                update_access_time(args[1]);
        }

        /* criar um arquivo ou modificar seu acesso */
        else if(strcmp(args[0], "criadir") == 0){ 
            if(!fileExists(args[1])){
                if(create_file(args[1], 1))
                    printf("Diretório criado!\n");
            }
        }

        /* mostrar informações de um arquivo */
        else if(strcmp(args[0], "mostra") == 0){
            show_file(args[1]);
        }

        /* listar arquivos abaixo de um diretorio */
        else if(strcmp(args[0], "lista") == 0){
            if(args[1] == NULL)
                return 0;
            list_directory(args[1]);
        }

        /* apagar um arquivo regular */
        else if(strcmp(args[0], "apaga") == 0){
            int index = fileExists(args[1]);
            if(index != -1){
                if(erase_file(args[1], index))
                    printf("Arquivo apagado!\n");
                else
                    printf("[ERRO]: Não foi possível apagar o arquivo!\n");
            }
        }

        /* desmontar o sistema de arquivos */
        else if(strcmp(args[0], "desmonta") == 0)
            unmount_file_system();

        else if(strcmp(args[0], "imprime") == 0)
            imprime_diretorios();
    }
    else
        printf("Por favor, monte um sistema de arquivos antes!\n");

    return 0;
}

/* inicializar a tabela FAT e o bitmap */
void initializeFileSystem(char* args[]){
    int i;
    char* c = "/";

    FILE* file = fopen(args[1], "rb");
    if(!file){
        for(i = 0; i < TOTAL_BLOCKS; i++)
        fat[i] = 0;
    
        for(i = 0; i < TOTAL_BLOCKS/8; i++)
            bitmap[i] = 0;

        /* criar diretorio '/' */
        create_file(c, 1);
    }
    /* montar o sistema de arquivos com base no binario
    else{

    }*/
    
}

/* retornar o horario no formato DD/MM/YY HH:MM:SS */
void get_current_date_time(char* buffer, size_t size){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%02d/%02d/%02d %02d:%02d:%02d", 
                            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
                            tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/* slash no filename para obter um filepath */
void getDirectoryPath(char* filepath, char* directory) {
    char* lastSlash = strrchr(filepath, '/');
    if (lastSlash != NULL) {
        size_t dirLength = lastSlash - filepath;
        strncpy(directory, filepath, dirLength);
        directory[dirLength] = '\0';
    } else {
        strcpy(directory, "/");
    }
}

/* imprimir os dados de um arquivo */
void show_file(char* filename){
    int i, j;
    FileInfo aux;
    char dir_path[256];

    getDirectoryPath(filename, dir_path);
    for(i = 0; i < total_dirs; i++){
        if(strcmp(dir_path, dirTree[i][0].fileName) == 0)
            break;
    }

    for(j = 0; j < dirTree[i][0].total_files_this_row; j++){
        if(strcmp(filename, dirTree[i][j].fileName) == 0){
            aux = dirTree[i][j];
            printf("%s\n", dirTree[i][j].fileName);
            if(!aux.is_directory)
                printf("\tTamanho em bytes: %d\n", aux.bytesSize);
            printf("\tÚltimo acesso: %s\n", aux.accessTime);
            printf("\tData de criação: %s\n", aux.creationTime);
            printf("\tData de modificação: %s\n", aux.modificationTime);
        }
    }
}

/* imprimir os dados de um arquivo */
void list_directory(char* dirname){
    int i, j;
    FileInfo aux;

    for(i = 0; i < total_dirs; i++){
        if(strcmp(dirname, dirTree[i][0].fileName) == 0)
            break;
    }

    for(j = 1; j < dirTree[i][0].total_files_this_row; j++){
        aux = dirTree[i][j];
        if(aux.is_directory && strcmp(aux.fileName, "/"))
            printf("%s/\n", dirTree[i][j].fileName);
        else{
            printf("%s\n", dirTree[i][j].fileName);
            printf("\tTamanho em bytes: %d\n", aux.bytesSize);
        }
        printf("\tÚltimo acesso: %s\n", aux.accessTime);
        printf("\tData de criação: %s\n", aux.creationTime);
        printf("\tData de modificação: %s\n", aux.modificationTime);
    }
}

/* imprimir diretorios e o que esta dentro deles */
void imprime_diretorios(){
    int i, j;
    for(i = 0; i < total_dirs; i++){
        printf("Diretorio %s: ", dirTree[i][0].fileName);
        printf("%d ", dirTree[i][0].total_files_this_row);
        for(j = 0; j < dirTree[i][0].total_files_this_row; j++){
            printf("%s ", dirTree[i][j].fileName);
        }
        printf("\n");
    }
}

/* remover os blocos reservados a um arquivo na FAT */
void free_fat_list(int i){
    int aux;
    while(fat[i] != -1){
        aux = fat[i];
        fat[i] = 0;
        i = aux;
    }
    if(fat[i] == -1)
        fat[i] = 0;
}

/* remover os blocos reservados a um arquivo no bitmap */
void free_bitmap(int i){
    int aux;
    while(bitmap[i] != -1){
        aux = bitmap[i];
        bitmap[i] = 0;
        i = aux;
    }
    if(bitmap[i] == -1)
        bitmap[i] = 0;
}

/* desmontar o sistema de arquivos */
void unmount_file_system(){
    int i;
    total_dirs = 0;
    isSystemMounted = 0;


    for(i = 0; i < TOTAL_BLOCKS; i++)
        fat[i] = 0;
    
    for(i = 0; i < TOTAL_BLOCKS/8; i++)
        bitmap[i] = 0;
}

/* salvar informacoes do arquivo lido */
void save_file_info(FileInfo* fileInfo){
    int file = open(mount_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1) {
        perror("Failed to open file");
        return;
    }

    write(file, fileInfo->fileName, FILENAME_LENGTH);
    write(file, &fileInfo->fat_block, sizeof(int));
    write(file, &fileInfo->bitmap_block[0], sizeof(int));
    write(file, &fileInfo->bytesSize, sizeof(int));
    write(file, &fileInfo->is_directory, sizeof(int));
    write(file, fileInfo->creationTime, 20);
    write(file, fileInfo->modificationTime, 20);
    write(file, fileInfo->accessTime, 20);
    write(file, "\n", 1);

    close(file);
}

/* modificar no arquivo binário o access time */
void update_access_time(char *filename){
    int file = open(mount_file, O_RDWR);
    if (file == -1) 
        perror("Failed to open file");

    fileAux fInfo;

    while (read(file, &fInfo, sizeof(fileAux)) > 0) {
        if (strcmp(fInfo.fileName, filename) == 0) {
            char acTime[20];
            get_current_date_time(acTime, sizeof(acTime));
            strncpy(fInfo.accessTime, acTime, sizeof(acTime));
            lseek(file, -sizeof(fileAux), SEEK_CUR);
            write(file, &fInfo, sizeof(fileAux));
            close(file);
            return;
        }
    }

    close(file);
}

/* inicializar os valores do arquivo criado */
FileInfo set_file_config(char* filename, int isDir, int fi, int bi){
    FileInfo new_file;

    /* preencher as informações do arquivo */
    get_current_date_time(new_file.creationTime, sizeof(new_file.creationTime));
    strcpy(new_file.fileName, filename);
    strcpy(new_file.accessTime, new_file.creationTime);
    strcpy(new_file.modificationTime, new_file.creationTime);
    new_file.fat_block = fi;
    new_file.bitmap_block[0] = bi;
    new_file.total_bits = 1;
    new_file.total_blocks = 1;
    if(!isDir)
        new_file.bytesSize = 327; // ocupa 1 na FAT, 1 no bitmap, eh o total dos dados salvos
    else
        new_file.bytesSize = 0;
    new_file.is_directory = isDir;

    return new_file;
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

/* apagar um arquivo regular 
 * se for implementar contador de posicao livre
 * no bitmap e na FAT, diminuir aqui
 */
int erase_file(char* filename, int dirIndex){
    int readFile = open(mount_file, O_RDWR);
    int auxFile = open("aux", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(auxFile == -1 || readFile == -1){
        perror("Failed to open file");
        return 0;
    }

    fileAux fInfo;
    
    // copiar todos os tudo para o arquivo auxiliar
    while(read(readFile, &fInfo, sizeof(fileAux)) > 0){
        if(strcmp(fInfo.fileName, filename) != 0)
            write(auxFile, &fInfo, sizeof(fileAux)); 
        else{
            // liberar o bitmap e FAT aqui, mudar essa funcao no main amanha
            free_fat_list(fInfo.fat_block);
            free_bitmap(fInfo.bitmap_block);
        }  
    }

    if (rename("aux", mount_file) == -1) {
        perror("Failed to rename file");
        return 0; 
    }

    close(readFile);
    close(auxFile);

    return 1;
}

/* criar um novo arquivo */
int create_file(char* filename, int isDir){
    FileInfo new_file;
    int i, total_files;
    int file_index = find_free_FAT_position();
    int bitmap_index = find_free_bitmap_position();
    char dir_path[256];

    getDirectoryPath(filename, dir_path);
    if(strcmp(dir_path, "") == 0)
        strcpy(dir_path, "/");

    /* é possível criar o arquivo? */
    if(file_index == -1 || bitmap_index == -1){
        printf("[ERRO]: espaço insuficiente.\n");
        return 0;
    }

    new_file = set_file_config(filename, isDir, file_index, bitmap_index);

    if(isDir){
        new_file.total_files_this_row = 1;  

        /* adicionar o diretorio na arvore de diretorios */
        dirTree[total_dirs][0] = new_file;
        total_dirs += 1;
    }

    /* adicionar o arquivo embaixo de um diretorio na arvore */
    if(strcmp(filename, "/")){
        for(i = 0; i < total_dirs; i++){
            if(strcmp(dirTree[i][0].fileName, dir_path) == 0){  /* encontramos o diretorio, colocar o arquivo nele */
                total_files = dirTree[i][0].total_files_this_row;

                dirTree[i][total_files] = new_file;
                dirTree[i][0].total_files_this_row += 1;
                
                break;
            }
        }
    }

    save_file_info(&new_file);

    /* preencher a tabela fat e o bitmap */
    fat[file_index] = -1;
    bitmap[bitmap_index] = 1;

    return 1;
}

/* verifica se um arquivo existe */
int fileExists(char *filename){
    int file = open(mount_file, O_RDONLY);
    if (file == -1) {
        perror("Failed to open file");
        return -1;
    }

    fileAux fInfo;
    int found = 0;

    while (read(file, &fInfo, sizeof(fileAux)) > 0) {
        if (strcmp(fInfo.fileName, filename) == 0) {
            found = 1;
            break;
        }
    }

    close(file);

    if(found)
        return 1;
    return 0;
}