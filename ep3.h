#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

char* displayPrompt();
int process_command(char* args[], int qtdParametros);
int nomeValido(char* nomeArquivo);