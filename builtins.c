/*
    Comandos integrados o "built-in" de DISH
*/

#include <sys/wait.h>
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
    "salir",
    "sys"
};

int (*builtin_func[]) (char**) =
{
    &dish_cd,
    &dish_help,
    &dish_exit,
    &dish_sys
};

int dish_print_help(char *command_name)
{
    char *filename;
    int c;
    FILE *file;
    
    filename = malloc(FILENAME_LENGTH * sizeof(char));
    if (!filename)  // error de malloc
    {
        fprintf(stderr, "dish: allocation error\n");
        exit(FAILURE);
    }

    filename[0] = '\0';
    filename = strcat(filename, "/root/shell/dish/help/");
    filename = strcat(filename, command_name);
    filename = strcat(filename, ".txt");
    file = fopen(filename, "r");

    if (!file)  // error de fopen
    {
        fprintf(stderr, "dish: no se pudo abrir el archivo\n");
        return 1;
    }

    c = fgetc(file);
    while (c != EOF)
    {
        putc(c, stdout);
        c = fgetc(file);
    }

    fclose(file);
}

// Cambia el directorio actual
int dish_cd(char **args)
{
    char *options[] = 
    {
        "--ayuda"
    };

    if (args[1] == NULL)
    {
        fprintf(stderr, "dish: se espera un argumento para \"ir\"\n");
    } else if (chdir(args[1]) != 0)
    {
        // Opciones
        if (strcmp(args[1], options[0]) == 0)
        {
            // --ayuda
            dish_print_help(builtin_str[0]);
        } else
        {
            perror("dish");
        }
    }

    return 1;
}

// Imprime una ayuda al usuario
int dish_help(char **args)
{
    printf("DISH: Giovanni Dueck's Interactive Shell\n");
    printf("      Ingresa un comando y sus argumentos y presiona enter.\n");
    printf("      Los comandos disponibles son:\n");

    for (int i = 0; i < NUM_BUILTINS; i++)
    {
        printf("        %s\n", builtin_str[i]);
    }
    printf("\n      Ingresa sys [comando] para ejecutar un comando del sistema.");
    
    printf ("\n");
    return 1;
}

// Cierra y sale del shell
int dish_exit(char **args)
{
    return 0;
}

// Inicia un nuevo proceso para ejecutar un comando del sistema;
// permite al usuario ejecutar comandos del sistema no propios del shell
int dish_sys(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        // Se modifica el proceso para que ejecute el
        //   comando dado
        if (execvp(args[1], &args[1]) == -1)
        {
            perror("dish");
        }
        exit(FAILURE);
    } else if (pid < 0)
    {
        // fork error
        perror("dish");
    } else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}
