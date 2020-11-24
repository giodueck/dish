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
    "sys",
    "historial",
    "usuario",
    "contrasena"
};

int (*builtin_func[]) (char**) =
{
    &dish_cd,
    &dish_help,
    &dish_exit,
    &dish_sys,
    &dish_history,
    &dish_useradd,
    &dish_passwd
};

// Imprime la ayuda de un comando guardada en help/command_name.txt
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

    // Se construye el camino absoluto del archivo
    filename[0] = '\0';
    filename = strcat(filename, "/root/shell/dish/help/");
    filename = strcat(filename, command_name);
    filename = strcat(filename, ".txt");
    file = fopen(filename, "r");

    if (!file)  // error de fopen
    {
        fprintf(stderr, "dish: no se pudo abrir el archivo %s\n", filename);
        return 2;
    }

    // Se lee e imprime todo el archivo
    c = fgetc(file);
    while (c != EOF)
    {
        putc(c, stdout);
        c = fgetc(file);
    }

    // se cierra el archivo y termina la funcion
    free(filename);
    fclose(file);
    return 1;
}

// Cambia el directorio actual
int dish_cd(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda"
    };
    char help_flag = FALSE;
    
    if (args[1] == NULL)
    {
        fprintf(stderr, "dish: se espera un argumento para \"ir\"\n");
    } else
    {
        // Opciones
        for (int i = 1; args[i] != NULL; i++)
        {
            // No se comparan argumentos que no sean opciones
            if (args[i][0] != '-')
            {
                continue;
            }

            // -h || --ayuda
            if (strcmp(args[1], options[0]) == 0 || strcmp(args[1], options[1]) == 0)
            {
                help_flag = TRUE;
                continue;
            } else
            {
                printf("dish: Opcion invalida.\n      Ingresa \"ir --ayuda\" para ver las opciones disponibles.\n");
                // en caso de opcion invalida se termina la ejecucion del comando
                return 1;
            }
        }

        // Se ejecuta el comando
        if (help_flag)
        {
            dish_print_help(builtin_str[0]);
        } else if (chdir(args[1]) != 0)
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
    char *options[] = 
    {
        "-h",
        "--ayuda"
    };
    char help_flag = FALSE;

    // Opciones
    for (int i = 1; args[i] != NULL; i++)
    {
        if (args[i][0] != '-')
        {
            continue;
        }

        if (strcmp(args[i], options[0]) == 0 || strcmp(args[i], options[1]) == 0)
        {
            help_flag = TRUE;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"salir --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    if (help_flag)
    {
        dish_print_help(builtin_str[2]);
        return 1;
    } else
    {
        // retornar 0 causa que la variable status en dish_loop sea 0, por lo que termina la ejecucion.
        return 0;
    }
}

// Inicia un nuevo proceso para ejecutar un comando del sistema;
// permite al usuario ejecutar comandos del sistema no propios del shell
int dish_sys(char **args)
{
    pid_t pid, wpid;
    int status;

    char *options[] = 
    {
        "-h",
        "--ayuda"
    };
    char help_flag = FALSE;

    // Opciones
    for (int i = 1; args[i] != NULL; i++)
    {
        if (args[i][0] != '-')
        {
            continue;
        }

        if (strcmp(args[i], options[0]) == 0 || strcmp(args[i], options[1]) == 0)
        {
            help_flag = TRUE;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"sys --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[3]);
    } else
    {
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
    }
    return 1;
}

// Muestra el historial de comandos
int dish_history(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-v"
    };
    char help_flag = FALSE;
    char verbose_flag = FALSE;

    FILE *log = fopen("/var/log/dish/dish.log", "r");

    if (!log)  // error de fopen
    {
        fprintf(stderr, "dish: no se pudo abrir el historial\n");
        return 2;
    }

    // Opciones
    for (int i = 1; args[i] != NULL; i++)
    {
        // No se comparan argumentos que no sean opciones
        if (args[i][0] != '-')
        {
            continue;
        }

        // -h || --ayuda
        if (strcmp(args[i], options[0]) == 0 || strcmp(args[i], options[1]) == 0)
        {
            if (verbose_flag)
            {
                printf("dish: Opcion invalida.\n      La opcion ayuda no toma parametros adicionales.\n");
                return 1;
            }

            help_flag = TRUE;
            continue;
        } else if (strcmp(args[i], options[2]) == 0)
        {
            if (help_flag)
            {
                printf("dish: Opcion invalida.\n      La opcion -v no toma parametros adicionales.\n");
                return 1;
            }

            verbose_flag = TRUE;
            continue;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"historial --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Se ejecuta el comando
    if (help_flag)
    {
        dish_print_help(builtin_str[4]);
    } else if (verbose_flag)
    {
        int c, i = 0;

        // Recorre el archivo de historial e imprime los comandos guardados
        while (c != EOF)
        {
            c = fgetc(log);
            if (c == '[')
            {
                i++;
                printf("%d ", i);
                while (c != '\n' && c != EOF)
                {
                    putc(c, stdout);
                    c = fgetc(log);
                }
                putc('\n', stdout);
            }
        }
    } else
    {
        int c, i = 0;

        // Recorre el archivo de historial e imprime los comandos guardados
        while (c != EOF)
        {
            c = fgetc(log);
            if (c == ']')
            {
                i++;
                printf("%d", i);
                c = fgetc(log);
                while (c != '\n' && c != EOF)
                {
                    putc(c, stdout);
                    c = fgetc(log);
                }
                putc('\n', stdout);
            }
        }
    }

    fclose(log);
    return 1;
}

// Agrega un usuario
int dish_useradd(char **args)
{
    printf("placeholder\n");
    return 1;
}

// Modifica la contrasenha del usuario dado
int dish_passwd(char **args)
{
    printf("placeholder\n");
    return 1;
}