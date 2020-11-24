/*
    Giovanni Dueck's Interactive Shell - DISH

    Sistemas Operativos 1
    Desarrollo del Shell

    Giovanni Dueck
*/

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
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

// Revisa si los archivos log necesarios existen, y si no, los crea
void check_logs()
{
    DIR *dir = opendir("/var/log/dish");

    // Se revisa si existe el directorio
    if (dir)
    {
        // Existe el directorio
        closedir(dir);
    } else if (ENOENT == errno)
    {
        // No existe el directorio y se crea
        mkdir("/var/log/dish");
    } else
    {
        // Error de opendir()
        fprintf(stderr, "dish: opendir() error\n");
        exit(FAILURE);
    }
    
    // Se revisa si existen los archivos
    FILE *log;

    if ((log = fopen("/var/log/dish/dish.log", "r");))
    {
        // Existe el archivo
        fclose(log);
    } else
    {
        // No existe el archivo y se crea
        time_t t = time(NULL);
        struct tm tms = *localtime(&t);

        log = fopen("/var/log/dish/dish.log", "w");
        fprintf(log, "LOG creado %d-%02d-%02d %02d:%02d:%02d\n\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
        fclose(log);
    }
}

int main (int argc, char **argv)
{
    // Inicializacion
    check_logs();

    // Main command loop
    dish_loop();

    // Shutdown

    return SUCCESS;
}
