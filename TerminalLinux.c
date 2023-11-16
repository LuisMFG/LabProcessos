#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void add_history(char **fila_comandos, char comando[]){
    if(fila_comandos[0] == NULL){
        fila_comandos[0] = malloc(strlen(comando) * sizeof(char));
        strcpy(*(fila_comandos + 0), comando);
        return;
    }

    int pos_null = 9;
    for(int i = 0; i < 10; i++){
        if(fila_comandos[i] == NULL){
            pos_null = i;
            break;
        }
    }

    for(int i = pos_null; i > 0; i--){
        if(fila_comandos[i] == NULL){
            fila_comandos[i] = malloc(strlen(fila_comandos[i-1]) * sizeof(char));
            strcpy(fila_comandos[i], fila_comandos[i-1]);
        }
        else{
            fila_comandos[i] = realloc(fila_comandos[i], strlen(fila_comandos[i-1]) * sizeof(char));
            strcpy(fila_comandos[i], fila_comandos[i-1]);
        }
    }
    fila_comandos[0] = (char *) realloc(fila_comandos[0], strlen(comando) * sizeof(char));
    strcpy(*(fila_comandos + 0), comando);
}

void history(char *fila_comandos[], char *comando, int *indice){
    int indice_final = *indice;
    int indice_inicial = *indice - 9;
    if(indice_inicial < 1){
        indice_inicial = 1;
    }

    int pos = 0;
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    while(pos < 10){
        if(*(fila_comandos + pos) == NULL){
            if(fila_comandos[0] == NULL){
                printf("Nenhum comando no histórico...");
            }
            break;
        }
        printf("%d %s", indice_final, fila_comandos[pos]);
        indice_final--;
        pos++;
    }
    indice_final = *indice;

    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    char opcao[10];
    for(;;){
        printf("- ");
        fgets(opcao, 10, stdin);
        if(opcao[0] == '!' && opcao[1] == '!'){
            strcpy(comando, fila_comandos[0]);
            break;
        }
        else if(opcao[0] == '!' &&  opcao[1] != '!'){
            char pos_comandos[strlen(opcao)-2];
            int pos_opcao = 1;
            for(int i = 0; i < (int) strlen(opcao)-2; i++){
                pos_comandos[i] = opcao[pos_opcao];
                pos_opcao++;
            }
            int indice_historico = indice_final - strtol(pos_comandos, NULL, 10);
            strcpy(comando, fila_comandos[indice_historico]);
            break;
        }
        else{
            printf("Opção inválida...\n");
        }
    }

}

int main(){
    
    pid_t identificador;
    int status;

    char *fila_comandos[10];
    for(int i = 0; i < 10; i++){
        fila_comandos[i] = NULL;
    }

    int should_run = 1;
    int indice = 0;
    while(should_run == 1){
        //lê o comando
        char comandos[100];
        printf("osh> ");
        fgets(comandos, 100, stdin);

        if(strncmp(comandos, "exit", strlen(comandos)-1) == 0){
            should_run = 0;
            continue;
        }

        //verifica se o comando passado é history
        if(strncmp(comandos, "history", strlen(comandos)-1) == 0){
            history(fila_comandos, &comandos[0], &indice);
        }
        
        int qta_divs = 0;
        int pos = 0;
        //determina a quantidade de espaços do comando para dimensionar o vetor
        while(comandos[pos] != '\0'){
            if(comandos[pos] == ' '){
                qta_divs++;
            }
            pos++;
        }

        int posDivs[qta_divs + 1];
        int cont_divs = 0;
        pos = 0;
        //atribui ao vetor a posição de cada espaço do comando
        while(comandos[pos] != '\0'){
            if(comandos[pos] == ' '){
                posDivs[cont_divs] = pos;
                cont_divs++;
            }
            pos++;
        }
        //considera-se o último caracter do comando como um espaço
        posDivs[cont_divs] = strlen(comandos) - 1;

        //define o tamanho de args considerando que o último ponteiro deve apontar para um endereço nulo
        int qta_args = qta_divs + 2;
        char *args[qta_args];
        int divIni = 0;
        int divFin;
        int tamArg;
        for(int i = 0; i < qta_args; i++){
            divFin = posDivs[i];
            tamArg = divFin - divIni;
            if(i == qta_args - 1){
                *(args + i) = NULL;
                break;
            }
            //aloca dinamicamente memória para cada ponteiro de args correpondente ao tamanho de cada token
            args[i] = (char *) malloc(tamArg * sizeof(char) + 1); 
            strncpy(args[i], &comandos[divIni], tamArg);
            args[i][tamArg] = '\0';
            divIni = divFin + 1;
        }

        identificador = fork();
        if(identificador < 0){
            printf("Error: Não é possível criar outro processo...");
            exit(EXIT_FAILURE);
        }
        else{
            if(identificador == 0){
                execvp(args[0], args);
                exit(EXIT_FAILURE);
            }
            else{
                wait(&status);
            }
        }

        for(int i = 0; i < qta_args; i++){
            free(args[i]);
        }

        if(WEXITSTATUS(status) == EXIT_FAILURE){
            printf("Comando não reconhecido...\n");
        }
        else{
            if(strncmp(comandos, "history", strlen(comandos)-1) != 0){
                add_history(fila_comandos, comandos);
                indice = indice + 1;
            }
        }
    }

    for(int i = 0; i < 10; i++){
        free(fila_comandos[i]);
    }

    exit(EXIT_SUCCESS);

    return 0;

}