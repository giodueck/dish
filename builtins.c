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

int num_builtins()
{
    return sizeof(builtin_str) / sizeof(char*);
}

int dish_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "dish: se espera un argumento para \"cd\"\n");
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

    for (int i = 0; i < num_builtins(); i++)
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