/*
    Funciones basicas de DISH
*/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dish.h"
#include "defines.h"
#include "builtins.h"

// Arrays de nombres de builtins con sus respectivas funciones
static char *builtin_str[] = 
{
    "ir",
    "ayuda",
    "salir",
    "sys",
    "historial",
    "usuario",
    "contrasena",
    "copiar",
    "mover",
    "renombrar",
    "creardir",
    "remover",
    "removerdir",
    "listar",
    "permisos",
    "propietario",
    "uinfo"
};

static int (*builtin_func[]) (char**) =
{
    &dish_cd,
    &dish_help,
    &dish_exit,
    &dish_sys,
    &dish_history,
    &dish_useradd,
    &dish_passwd,
    &dish_cp,
    &dish_mv,
    &dish_rn,
    &dish_mkdir,
    &dish_rm,
    &dish_rmdir,
    &dish_ls,
    &dish_chmod,
    &dish_chown,
    &dish_userinfo
};

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
//  Retorno: retorna el resultado del comando ejecutado
//      0: cuando el comando ejecutado cierra el shell
//      otro valor: cuando el shell sigue en ejecucion
int dish_execute(char **args)
{
    if (args[0] == NULL)
    {
        // comando vacio
        return 1;
    }

    // Comandos built-in
    for (int i = 0; i < NUM_BUILTINS; i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    // Comando desconocido
    return dish_command_not_found(args);
}

// Imprime un mensaje de error en caso de que el comando dado no existe
int dish_command_not_found(char **args)
{
    char *msg = "Comando no existe.";
    char *help = "Usa \"sys [comando]\" para ejecutar comandos del sistema,\n\to usa \"ayuda\" para obtener mas ayuda.";

    printf("dish: %s\n\t%s\n", msg, help);

    return 1;
}

// Selection sort para arrays de cadenas
void dish_sort(char **array, int size)
{
    char *aux;

    for (int i = 1; i < size; i++) // iterate through list
    {
        aux = array[i]; // store curr value
        for (int j = i - 1; j >= 0; j--) // iterate thru sorted sublist
        {
            if (strcmp(aux, array[j]) < 0) // smaller than item before it?
            {
                // swap
                array[j + 1] = array[j];
                array[j] = aux;
            }
        }
    }
}

// Funcion potencia recursiva basica
double dish_pow(double x, double y)
{
    if (y < 0)
        return 1 / dish_pow(x, -y);
    else if (y == 0)
        return 1;
    else if (y == 1)
        return x;
    else
        return x * dish_pow(x, y - 1);
}