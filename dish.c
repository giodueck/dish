/*
    Funciones basicas de DISH
*/

// #include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dish.h"
#include "defines.h"

// Lee una linea de stdin hasta encontrar EOF o '\n'
char *dish_read_line()
{
    int bufsize = LINE_BUFSIZE;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c, i = 0;

    if (!buffer) // error de malloc
    {
        fprintf(stderr, "dish: allocation error\n");
        exit(FAILURE);
    }

    while (1)
    {
        c = getchar();

        if (c == EOF || c == '\n')
        {
            buffer[i] = '\0';
            return buffer;
        } else if (i >= bufsize)
        {
            bufsize += LINE_BUFSIZE;
            buffer = realloc(buffer, sizeof(char) * bufsize);

            if (!buffer) // error de realloc
            {
                fprintf(stderr, "dish: allocation error\n");
                exit(FAILURE);
            }
        } else
        {
            buffer[i] = c;
            i++;
        }
    }
}

// Divide la cadena dada en tokens
char **dish_split_line(char *line)
{
    int bufsize = ARGS_BUFSIZE;
    char **args = malloc(sizeof(char*) * bufsize);
    int i = 0;

    if (!args) // error de malloc
    {
        fprintf(stderr, "dish: allocation error\n");
        exit(FAILURE);
    }

    args[i] = strtok(line, TOK_DELIM);

    while (args[i] != NULL)
    {
        if (i >= bufsize)
        {
            bufsize += ARGS_BUFSIZE;
            args = realloc(args, sizeof(char*) * bufsize);

            if (!bufsize) // error de realloc
            {
                fprintf(stderr, "dish: allocation error\n");
                exit(FAILURE);
            }
        }

        i++;
        args[i] = strtok(NULL, TOK_DELIM);
    }

    return args;
}

// Ejecuta el comando dado por args
int dish_execute(char **args);