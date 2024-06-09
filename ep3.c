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
char* mount_file;

char dirTree[maxDir][maxFiles][FILENAME_LENGTH];
int fat[TOTAL_BLOCKS];
int visited[9999];
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
        // else if(strcmp(args[0], "mostra") == 0){
        //     show_file(args[1]);
        // }

        /* listar arquivos abaixo de um diretorio */
        else if(strcmp(args[0], "lista") == 0){
            if(args[1] == NULL)
                return 0;
            list_directory(args[1]);
        }

        /* apagar um arquivo regular */
        else if(strcmp(args[0], "apaga") == 0){
            if(fileExists(args[1])){
                if(erase_file(args[1]))
                    printf("Arquivo apagado!\n");
                else
                    printf("[ERRO]: Não foi possível apagar o arquivo!\n");
            }
        }

        /* desmontar o sistema de arquivos */
        else if(strcmp(args[0], "desmonta") == 0)
            unmount_file_system();

        /* criar um db na memoria */
        else if(strcmp(args[0], "atualizadb") == 0){
            update_db();
            printf("Banco de Dados atualizado!\n");
        }

        /* procura uma substring no bd */
        else if(strcmp(args[0], "busca") == 0)
            search_string(args[1]);

        /* apagar um diretorio e todos arquivos abaixo */
        else if(strcmp(args[0], "apagadir") == 0){
            if(fileExists(args[1])){
                if(erase_dir(args[1]))
                    printf("Diretório apagado!\n");
                else
                    printf("[ERRO]: Não foi possível apagar o diretório!\n");
            }
        }

        /* imprimir os status do sistema de arquivos */
        else if(strcmp(args[0], "status") == 0)
            print_status();

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
    char c[FILENAME_LENGTH];

    strcpy(c, "/");

    FILE* file = fopen(args[1], "rb");
    if(!file){
        for(i = 0; i < TOTAL_BLOCKS; i++)
        fat[i] = 0;
    
        for(i = 0; i < TOTAL_BLOCKS/8; i++)
            bitmap[i] = 0;

        /* criar diretorio '/' */
        create_file(c, 1);
    }
    /* montar o sistema de arquivos com base no binario */
    else {
        /* mover o ponteiro para o inicio da FAT */
        if(fseek(file, -(TOTAL_BLOCKS*sizeof(int) + (TOTAL_BLOCKS/8)*sizeof(uint8_t)), SEEK_END) != 0){
            perror("[ERRO]: fseek para FAT");
            fclose(file);
            return;
        }

        /* ler a FAT */
        if (fread(fat, sizeof(int), TOTAL_BLOCKS, file) != TOTAL_BLOCKS){
            perror("[ERRO]: ler tabela FAT");
            fclose(file);
            return;
        }

        /* ler o bitmap */
        if (fread(bitmap, sizeof(uint8_t), TOTAL_BLOCKS / 8, file) != TOTAL_BLOCKS / 8){
            perror("[ERRO]: ler bitmap");
            fclose(file);
            return;
        }

        /* truncar o arquivo, tirando o bitmap e a fat */
        fseek(file, -(TOTAL_BLOCKS*sizeof(int) + (TOTAL_BLOCKS/8)*sizeof(uint8_t)), SEEK_END);
        long new_file_size = ftell(file);

        // fseek(file, 0, SEEK_END);

        fclose(file);

        if (truncate(args[1], new_file_size) == -1){
            perror("[ERRO]: truncar arquivo");
            exit(1);
        }

        print_dir_tree();
    }
}

/* imprimir a arvore de diretorios do sistema montado */
void print_dir_tree(){
    int file_count = 0, i;
    int file = open(mount_file, O_RDONLY);
    char filename[FILENAME_LENGTH];
    FileInfo files[9999];
    if (file == -1) {
        perror("[ERRO]: abrir arquivo");
        return;
    }

    for(i = 0; i < file_count; i++)
        visited[i] = 0;

    while(read(file, &files[file_count], sizeof(FileInfo)) > 0)
        file_count++;

    close(file);

    for(i = 0; i < file_count; i++){
        /* procurar todos os arquivos embaixo desse diretorio */
        if(files[i].is_directory && !visited[i]){
            visited[i] = 1;
            if(strcmp(files[i].fileName, "/")){
                get_filename(files[i].fileName, filename);
                printf("%s\n", filename);
            }
            else
                printf("%s\n", files[i].fileName);
            print_directory(files, files[i].fileName, file_count, 1, i);
        }
    }
}

void print_directory(FileInfo f[], char dirName[], int file_count, int level, int currentIndex){
    int i;
    int j;
    char path[FILENAME_LENGTH];
    char filename[FILENAME_LENGTH];

    for(j = 0; j < file_count; j++){
        getDirectoryPath(f[j].fileName, path);
        // printf("Comparando %s e %s\n", path, dirName);
        if(strcmp(path, dirName) == 0){
            for(i = 0; i < level && level > 1; i++)
                printf("  ");
            get_filename(f[j].fileName, filename);
            if(f[j].is_directory && strcmp(f[j].fileName, dirName) && strcmp(f[j].fileName, "") && !visited[j]){
                printf("L %s\n", filename);
                visited[j] = 1;
                print_directory(f, f[j].fileName, file_count, level+1, j);
            }
            else{
                if(strcmp(f[j].fileName, "/"))
                    printf("L %s\n", filename);
            }
        }
    }
}

/* retornar o nome do arquivo no formato /path/to/file */
void get_filename(char *path, char *buffer){
    char *last_slash = strrchr(path, '/');
    
    if (last_slash != NULL)
        strcpy(buffer, last_slash + 1);
    else
        strcpy(buffer, path);
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
    if (lastSlash != NULL && lastSlash != filepath) {
        size_t dirLength = lastSlash - filepath;
        strncpy(directory, filepath, dirLength);
        directory[dirLength] = '\0';
        return;
    }
    strcpy(directory, "/");
}

/* imprimir os dados de um arquivo */
// void show_file(char* filename){
//     int i, j;
//     FileInfo aux;
//     char dir_path[256];

//     getDirectoryPath(filename, dir_path);
//     for(i = 0; i < total_dirs; i++){
//         if(strcmp(dir_path, dirTree[i][0].fileName) == 0)
//             break;
//     }

//     for(j = 0; j < dirTree[i][0].total_files_this_row; j++){
//         if(strcmp(filename, dirTree[i][j].fileName) == 0){
//             aux = dirTree[i][j];
//             printf("%s\n", dirTree[i][j].fileName);
//             if(!aux.is_directory)
//                 printf("\tTamanho em bytes: %d\n", aux.bytesSize);
//             printf("\tÚltimo acesso: %s\n", aux.accessTime);
//             printf("\tData de criação: %s\n", aux.creationTime);
//             printf("\tData de modificação: %s\n", aux.modificationTime);
//         }
//     }
// }

/* imprimir de fato os dados da funcao abaixo */
void print_data(FileInfo fInfo){
    if(!fInfo.is_directory) {
        printf("%s\n", fInfo.fileName);
        printf("\tTamanho em bytes: %d\n", fInfo.bytesSize);
    } 
    else{
        if(strcmp(fInfo.fileName, "/") == 0)
            printf("%s\n", fInfo.fileName);
        else
            printf("%s/\n", fInfo.fileName);
    }
    printf("\tÚltimo acesso: %s\n", fInfo.accessTime);
    printf("\tHora de criação: %s\n", fInfo.creationTime);
    printf("\tHora de modificação: %s\n", fInfo.modificationTime);
}

/* imprimir os dados de um arquivo */
void list_directory(char* dirname) {
    int file = open(mount_file, O_RDWR);
    if (file == -1) {
        perror("[ERRO]: abrir arquivo");
        return;
    }

    FileInfo fInfo;
    while (read(file, &fInfo, sizeof(FileInfo)) > 0) {
        char dirpath[FILENAME_LENGTH];
        getDirectoryPath(fInfo.fileName, dirpath);

        if (strcmp(dirname, "/") == 0) {
            char* remainingPath = fInfo.fileName + 1;
            if (strchr(remainingPath, '/') == NULL)
                print_data(fInfo);
        } 
        else{
            if (strcmp(dirpath, dirname) == 0)
                print_data(fInfo);
        }
    }
    close(file);
}

/* imprimir diretorios e o que esta dentro deles */
void imprime_diretorios(){
    int i, j;
    for(i = 0; i < maxDir; i++){
        if(strcmp(dirTree[i][0], "") == 0)
            break;
        if(strcmp(dirTree[i][0], "/"))
            printf("%s/:\n", dirTree[i][0]);
        else
            printf("%s:\n", dirTree[i][0]);
        for(j = 1; j < maxFiles; j++){
            if(strcmp(dirTree[i][j], "") == 0)
                break;
            printf("\t%s\n", dirTree[i][j]);
        }
    }
}

/* define 0 ou 1 em uma posicao do bitmap */
void set_bitmap(int block, int value){
    int byteIndex = block / 8;
    int bitIndex = block % 8;
    if(value)
        bitmap[byteIndex] |= (1 << bitIndex); // marca como ocupado
    else
        bitmap[byteIndex] &= ~(1 << bitIndex); // marca como livre
}

/* remover os blocos reservados a um arquivo na FAT */
void free_fat_list(int i){
    int current_block = i;

    while (fat[current_block] != -1){
        int next_block = fat[current_block];

        set_bitmap(current_block, 0);
        fat[current_block] = 0;

        current_block = next_block;
    }

    if(fat[current_block] == -1)
        fat[current_block] = 0;
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
    int file = open(mount_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    ssize_t bytes_written;
    if(file == -1){
        perror("[ERRO]: abrir arquivo");
        return;
    }

    isSystemMounted = 0;
    mount_file = NULL;

    /* copiar FAT e bitmap para o mount_file */
    bytes_written = write(file, fat, TOTAL_BLOCKS*sizeof(int));
    if (bytes_written != TOTAL_BLOCKS*sizeof(int)){
        perror("[ERRO]: escrever tabela FAT");
        close(file);
        return;
    }

    bytes_written = write(file, bitmap, (TOTAL_BLOCKS/8)*sizeof(uint8_t));
    if (bytes_written != (TOTAL_BLOCKS/8)*sizeof(uint8_t)){
        perror("[ERRO]: escrever bitmap");
        close(file);
        return;
    }

    /* zerar FAT e bitmap */
    for(i = 0; i < TOTAL_BLOCKS; i++)
        fat[i] = 0;
    
    for(i = 0; i < TOTAL_BLOCKS/8; i++)
        bitmap[i] = 0;

    close(file);
}

/* salvar informacoes do arquivo lido */
void save_file_info(FileInfo* fileInfo){
    int file = open(mount_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1) {
        perror("[ERRO]: abrir arquivo");
        return;
    }
    FileInfo f;
    strcpy(f.fileName, fileInfo->fileName);
    strcpy(f.accessTime, fileInfo->accessTime);
    strcpy(f.creationTime, fileInfo->creationTime);
    strcpy(f.modificationTime, fileInfo->modificationTime);
    f.fat_block = fileInfo->fat_block;
    f.bytesSize = fileInfo->bytesSize;
    f.is_directory = fileInfo->is_directory;

    write(file, &f, sizeof(FileInfo));

    close(file);
}

/* modificar no arquivo binário o access time */
void update_access_time(char *filename){
    int file = open(mount_file, O_RDWR);
    if (file == -1){
        perror("[ERRO]: abrir arquivo");
        return;
    }

    FileInfo fInfo;

    while (read(file, &fInfo, sizeof(FileInfo)) > 0){
        if (strcmp(fInfo.fileName, filename) == 0){
            char acTime[20];
            get_current_date_time(acTime, sizeof(acTime));
            strncpy(fInfo.accessTime, acTime, sizeof(acTime));
            lseek(file, -sizeof(FileInfo), SEEK_CUR);
            write(file, &fInfo, sizeof(FileInfo));
            close(file);
            return;
        }
    }

    close(file);
}

/* criar arvore de diretorios/arquivos */
void update_db(){
    int i, j;
    int line = 0;
    int file = open(mount_file, O_RDONLY);
    char path[255];

    if(file == -1){
        perror("[ERRO]: abrir arquivo");
        return;
    }

    for(i = 0; i < maxDir; i++)
        for(j = 0; j < maxFiles; j++)
            strcpy(dirTree[i][j], "");

    FileInfo fInfo;
    while(read(file, &fInfo, sizeof(FileInfo)) > 0){
        if(fInfo.is_directory){
            strcpy(dirTree[line][0], fInfo.fileName);   /* filename ja tem o diretorio junto */
            line++;
        }
        else{
            getDirectoryPath(fInfo.fileName, path);
            j = 0;
            for(i = 0; i < line; i++){
                if(strcmp(dirTree[i][0], path) == 0){
                    while(strcmp(dirTree[i][j], ""))
                        j++;
                    strcpy(dirTree[i][j], fInfo.fileName);
                    break;
                }
            }
        }
    }
}

/* buscar uma string s no bd */
void search_string(char* s){
    int i, j;
    char* substring;

    for(i = 0; i < maxDir; i++){
        if(strcmp(dirTree[i][0], "") == 0)
            break;
        for(j = 0; j < maxFiles; j++){
            if(strcmp(dirTree[i][j], "") == 0)
                break;
            substring = strstr(dirTree[i][j], s);
            if (substring != NULL) {
                printf("\t-> %s\n", dirTree[i][j]);
            }
        }
    }
}

/* imprimir status do sistema de arquivos */
void print_status(){
    int bsize;
    int total_dirs = 0;
    int total_files = 0;
    double free_space = 0;
    double unused_space = 0;
    int file = open(mount_file, O_RDONLY);
    if(file == -1){
        perror("[ERRO]: abrir arquivo");
        return;
    }

    FileInfo fInfo;

    while(read(file, &fInfo, sizeof(FileInfo)) > 0){
        if(fInfo.is_directory) total_dirs += 1;
        else total_files += 1;
        bsize = fInfo.bytesSize;
        free_space += bsize;
        unused_space += (maxBytesSize - bsize); 
    }

    free_space = totalAvailableKB - (free_space/KB);
    unused_space /= KB;

    printf("\t[x] Arquivos: %d\n", total_files);
    printf("\t[x] Diretórios: %d\n", total_dirs);
    printf("\t[x] Espaço livre(KB): %.2f\n", free_space);
    printf("\t[x] Espaço desperdiçado(KB): %.2f\n", unused_space);
}

/* inicializar os valores do arquivo criado */
FileInfo set_file_config(char* filename, int isDir, int fi){
    FileInfo new_file;

    /* preencher as informações do arquivo */
    get_current_date_time(new_file.creationTime, sizeof(new_file.creationTime));
    strcpy(new_file.fileName, filename);
    strcpy(new_file.accessTime, new_file.creationTime);
    strcpy(new_file.modificationTime, new_file.creationTime);
    new_file.fat_block = fi;

    if(!isDir)
        new_file.bytesSize = 327;
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

/* apagar um arquivo regular */
int erase_file(char* filename){
    int readFile = open(mount_file, O_RDWR);
    int auxFile = open("aux", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(auxFile == -1 || readFile == -1){
        perror("[ERRO]: abrir arquivo");
        return 0;
    }

    FileInfo fInfo;
    
    // copiar todos arquivos desejados para o arquivo auxiliar
    while(read(readFile, &fInfo, sizeof(FileInfo)) > 0){
        if(strcmp(fInfo.fileName, filename) != 0)
            write(auxFile, &fInfo, sizeof(FileInfo)); 
        else
            free_fat_list(fInfo.fat_block); // liberar tanto FAT como bitmap
    }

    if (rename("aux", mount_file) == -1) {
        perror("[ERRO]: salvar arquivo novo!");
        return 0; 
    }

    close(readFile);
    close(auxFile);

    return 1;
}

/* apagar um diretório e todos arquivos embaixo */
int erase_dir(char* dirname){
    int dirname_len = strlen(dirname);
    int file = open(mount_file, O_RDWR);
    int auxFile = open("aux", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(file == -1 || auxFile == -1){
        perror("[ERRO]: abrir arquivo");
        return 0;
    }

    FileInfo fInfo;

    while(read(file, &fInfo, sizeof(FileInfo)) > 0){
        if(strncmp(dirname, fInfo.fileName, dirname_len) != 0)
            write(auxFile, &fInfo, sizeof(FileInfo));
        else{
            free_fat_list(fInfo.fat_block);
            printf("[-] %s\n", fInfo.fileName);
        }
    }

    if(rename("aux", mount_file) == -1){
        perror("[ERRO]: salvar arquivo novo!");
        return 0;
    }
    close(file);
    close(auxFile);
    
    return 1;
}

/* criar um novo arquivo */
int create_file(char* filename, int isDir){
    FileInfo new_file;
    int file_index = find_free_FAT_position();
    int bitmap_index = find_free_bitmap_position();
    char dir_path[FILENAME_LENGTH];

    getDirectoryPath(filename, dir_path);
    if(strcmp(dir_path, "") == 0)
        strcpy(dir_path, "/");

    /* é possível criar o arquivo? */
    if(file_index == -1 || bitmap_index == -1){
        printf("[ERRO]: espaço insuficiente.\n");
        return 0;
    }

    new_file = set_file_config(filename, isDir, file_index);

    save_file_info(&new_file);

    /* preencher a tabela fat e o bitmap */
    fat[file_index] = -1;
    bitmap[bitmap_index] = 1;

    return 1;
}

/* verifica se um arquivo existe */
int fileExists(char *filename){
    int file = open(mount_file, O_RDONLY);
    FileInfo fInfo;

    if (file == -1) {
        perror("[ERRO]: abrir arquivo");
        return -1;
    }

    while (read(file, &fInfo, sizeof(FileInfo)) > 0) {
        if (strcmp(fInfo.fileName, filename) == 0) {
            close(file);
            return 1;
        }
    }
    close(file);
    return 0;
}