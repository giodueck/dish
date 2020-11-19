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

