#include "ep3.h"

int main(){
    int i;
    char* command;
    char* args[10];

    while(1){
        command = readline(displayPrompt());
        if(command == NULL)
            continue;

        i = 0;
        args[i] = strtok(command, " ");
        while (args[i] != NULL && i < 9){
            i++;
            args[i] = strtok(NULL, " ");
        }
    }
    return 0;
}

char* displayPrompt(){
    return "{ep3}: ";
}