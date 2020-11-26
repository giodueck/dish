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
    "contrasena",
    "copiar",
    "mover",
    "renombrar",
    "creardir",
    "remover",
    "removerdir"
};

int (*builtin_func[]) (char**) =
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
    &dish_rmdir
};

extern char *username;
extern char *home;

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
        args[1] = home;
        args[2] = NULL;
    } else
    {
        // Opciones
        for (int i = 1; args[i] != NULL; i++)
        {
            // No se comparan argumentos que no sean opciones
            if (args[i][0] != '-')
            {
                // Opciones siempre van antes del resto de los argumentos para ser validas
                break;
            }

            // -h || --ayuda
            if (strcmp(args[1], options[0]) == 0 || strcmp(args[1], options[1]) == 0)
            {
                help_flag = TRUE;
                break;
            } else
            {
                printf("dish: Opcion invalida.\n      Ingresa \"ir --ayuda\" para ver las opciones disponibles.\n");
                // en caso de opcion invalida se termina la ejecucion del comando
                return 1;
            }
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
            // Opciones siempre van antes del resto de los argumentos para ser validas
            break;
        }

        if (strcmp(args[i], options[0]) == 0 || strcmp(args[i], options[1]) == 0)
        {
            help_flag = TRUE;
            break;
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
            // Opciones siempre van antes del resto de los argumentos para ser validas
            break;
        }

        if (strcmp(args[i], options[0]) == 0 || strcmp(args[i], options[1]) == 0)
        {
            help_flag = TRUE;
            break;
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
    extern char *log_filename;

    FILE *log = fopen(log_filename, "r");

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
            // Opciones siempre van antes del resto de los argumentos para ser validas
            break;
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
            break;
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

// Copia un archivo
int dish_cp(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-d"
    };
    char help_flag = FALSE;
    char dir_flag = FALSE;
    int i;

    // Opciones
    for (i = 1; args[i] != NULL; i++)
    {
        if (args[i][0] != '-')
        {
            // Opciones siempre van antes del resto de los argumentos para ser validas
            break;
        }

        if (strcmp(args[i], options[0]) == 0 || strcmp(args[i], options[1]) == 0)
        {
            help_flag = TRUE;
            break;
        } else if (strcmp(args[i], options[2]) == 0)
        {
            dir_flag = TRUE;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"copiar --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[7]);
    } else
    {
        if (args[1] == NULL)
        {
            fprintf(stderr, "dish: archivo de origen no especificado\n");
        } else if (dir_flag)
        {
            // Se especifica un directorio
            // args[n] = nombre del archivo
            // args[n+1] = nombre del nuevo archivo o nombre del directorio
            // args[n+2] = si args[2] es el nombre del nuevo archivo, nombre del directorio; si no, NULL
            // el argumento que esta antes del args[n] = NULL es el directorio

            // args[i] debe ser el nombre del archivo ya que es el primer argumento que no es opcion
            char *filename = args[i];
            char *new_filename;
            char new_filename_given;
            char *dir;
            char *curr_dir = malloc(sizeof(char) * DIR_BUFSIZE);

            // Encuentra el indice del argumento NULL
            while (args[i] != NULL) i++;

            // El elemento antes del argumento NULL debe ser el directorio, ya que se especifica la opcion -d
            dir = args[i - 1];

            // Revisa si el nombre del nuevo archivo se da
            if (args[i - 2] == filename)
            {
                // No se da
                new_filename = malloc(sizeof(char) * FILENAME_LENGTH);
                sprintf(new_filename, "%s", filename);
                new_filename_given = FALSE;
            } else
            {
                // Si se da
                new_filename = args[i - 2];
                new_filename_given = TRUE;
            }
            
            // Copia
            FILE *file;
            FILE *new_file;

            // Se abre al archivo origen
            if (!(file = fopen(filename, "r")))
            {
                // si no existe
                fprintf(stderr, "dish: archivo de origen no existe o no se pudo abrir\n");
            } else
            {
                // si existe
                int c;

                // Cambia el directorio al directorio dado
                curr_dir = getcwd(curr_dir, DIR_BUFSIZE);
                if (chdir(dir) != 0)
                {
                    perror("dish");
                } else
                {

                    // Se verifica si el archivo de destino ya existe
                    if (new_filename_given)
                    {
                        // Si se dio el nombre nuevo, se alerta al usuario y se interrumpe la copia
                        if ((new_file = fopen(new_filename, "r")))
                        {
                            fclose(new_file);
                            fprintf(stderr, "dish: archivo de destino ya existe\n");
                            return 1;
                        } else
                        {
                            new_file = fopen(new_filename, "w");
                        }
                    } else
                    {
                        // Si el nombre es autogenerado, se modifica el nombre
                        i = 0;
                        while ((new_file = fopen(new_filename, "r")))
                        {
                            fclose(new_file);
                            if (i) sprintf(new_filename, "%s-Copia %d", filename, i);
                            else sprintf(new_filename, "%s-Copia", filename);
                            i++;
                        }
                        new_file = fopen(new_filename, "w");
                    }

                    // La copia en si
                    c = fgetc(file);
                    while (c != EOF)
                    {
                        fputc(c, new_file);
                        c = fgetc(file);
                    }
                    
                    fclose(file);
                    fclose(new_file);
                    // Vuelve al directorio original
                    chdir(curr_dir);
                }
            }

            free(curr_dir);
            if (!new_filename_given) free(new_filename);

        } else
        {
            // No se especifica un directorio
            // args[1] = nombre del archivo
            // args[2] = nombre del nuevo archivo (opcional)

            // Nombres de archivos
            char *filename = args[1];
            char *new_filename = args[2];
            char new_filename_given = (new_filename) ? TRUE : FALSE;
            if (!new_filename_given)
            {
                new_filename = malloc(sizeof(char) * FILENAME_LENGTH);
                sprintf(new_filename, "%s-Copia", args[1]);
            }

            // Copia
            FILE *file;
            FILE *new_file;

            // Se abre al archivo origen
            if (!(file = fopen(filename, "r")))
            {
                // si no existe
                fprintf(stderr, "dish: archivo de origen no existe o no se pudo abrir\n");
            } else
            {
                // si existe
                int c;

                // Se verifica si el archivo de destino ya existe
                if (new_filename_given)
                {
                    // Si se dio el nombre nuevo, se alerta al usuario y se interrumpe la copia
                    if ((new_file = fopen(new_filename, "r")))
                    {
                        fclose(new_file);
                        fprintf(stderr, "dish: archivo de destino ya existe\n");
                        return 1;
                    } else
                    {
                        new_file = fopen(new_filename, "w");
                    }
                } else
                {
                    // Si el nombre es autogenerado, se modifica el nombre
                    i = 2;
                    while ((new_file = fopen(new_filename, "r")))
                    {
                        fclose(new_file);
                        sprintf(new_filename, "%s-Copia %d", args[1], i);
                        i++;
                    }
                    new_file = fopen(new_filename, "w");
                }

                // La copia en si
                c = fgetc(file);
                while (c != EOF)
                {
                    fputc(c, new_file);
                    c = fgetc(file);
                }
                
                fclose(file);
                fclose(new_file);
            }

            if (!new_filename_given) free(new_filename);
        }
    }

    return 1;
}

// Mueve un archivo a otro directorio
int dish_mv(char **args)
{
    return 1;
}

// Renombra un archivo
int dish_rn(char **args)
{
    return 1;
}

// Crea un nuevo directorio
int dish_mkdir(char **args)
{
    return 1;
}

// Elimina un archivo
int dish_rm(char **args)
{
    return 1;
}

// Elimina un directorio
int dish_rmdir(char **args)
{
    return 1;
}
