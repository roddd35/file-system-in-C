#include "ep3.h"

int main(){
    int i;
    int exit = 0;
    char* command;
    char* args[10];

    while(1){
        /* read command */
        command = readline(displayPrompt());
        if(command == NULL)
            continue;

        /* separate command into args */
        i = 0;
        args[i] = strtok(command, " ");
        while (args[i] != NULL && i < 10){
            i++;
            args[i] = strtok(NULL, " ");
        }

        exit = process_command(args);
        if(exit)
            return 0;
    }

    return 0;
}

char* displayPrompt(){
    return "{ep3}: ";
}

int process_command(char* args[]){
    /* salvar estado e sair do simulador */
    if(strcmp(args[0], "sai") == 0){
        return 1;
    }
    return 0;
}