/*
    Giovanni Dueck's Interactive Shell - DISH

    Sistemas Operativos 1
    Desarrollo del Shell

    Giovanni Dueck
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "dish.h"

// Loop central del shell
void dish_loop()
{
    char *line;
    char **args;
    char *current_dir = malloc (sizeof(char) * CURRDIR_BUFSIZE);
    int status = 0;

    if (!current_dir) // error de malloc
    {
        fprintf(stderr, "dish: allocation error\n");
        exit(FAILURE);
    }

    do
    {
        // Se imprime el directotio actual y se lee el comando a ejecutarse
        current_dir = getcwd(current_dir, CURRDIR_BUFSIZE);
        printf("[%s] > ", current_dir);
        line = dish_read_line();
        args = dish_split_line(line);
        status = dish_execute(args);

        // Como a line y args se asignan bloques de memoria dinamica hay que liberarlos 
        free(line);
        free(args);

        // Mientras que status != 0, el shell sigue leyendo comandos
    } while (status);
    
    free(current_dir);
}

int main (int argc, char **argv)
{
    // Inicializacion

    // Main command loop
    dish_loop();

    // Shutdown

    return SUCCESS;
}
