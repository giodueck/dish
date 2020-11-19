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

char *builtin_str[] = 
{
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char**) =
{
    &dish_cd,
    &dish_help,
    &dish_exit
};

int num_builtins()
{
    return sizeof(builtin_str) / sizeof(char*);
}

