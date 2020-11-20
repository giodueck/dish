/*
    Comandos integrados o "built-in" de DISH
*/

// #include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "defines.h"

// Arrays de nombres de builtins con sus respectivas funciones
char *builtin_str[] = 
{
    "ir",
    "ayuda",
    "salir"
};

int (*builtin_func[]) (char**) =
{
    &dish_cd,
    &dish_help,
    &dish_exit
};

int dish_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "dish: se espera un argumento para \"ir\"\n");
    } else if (chdir(args[1]) != 0)
    {
        perror("dish");
    }

    return 1;
}

int dish_help(char **args)
{
    printf("DISH: Giovanni Dueck's Interactive Shell\n");
    printf("      Ingresa un comando y sus argumentos y presiona enter.\n");
    printf("      Los comandos integrados son:\n");

    for (int i = 0; i < NUM_BUILTINS; i++)
    {
        printf("        %s\n", builtin_str[i]);
    }
    
    printf ("\n");
    return 1;
}

int dish_exit(char **args)
{
    return 0;
}