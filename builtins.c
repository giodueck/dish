/*
    Comandos integrados o "built-in" de DISH
*/

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "builtins.h"
#include "defines.h"
#include "dish.h"
#include "err.h"

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
    "removerdir",
    "listar"
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
    &dish_rmdir,
    &dish_ls
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
        char *msg = malloc(sizeof(char) * MSG_LENGTH);
        sprintf(msg, "ayuda: no se pudo abrir el archivo %s\n", filename);
        fprintf(stderr, msg);
        err_log_add_msg(msg);
        free(msg);
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
        err_print("ir");
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
                err_print("execvp");
            }
            exit(FAILURE);
        } else if (pid < 0)
        {
            // fork error
            err_print("fork");
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
        "-v",
        "-r"
    };
    char help_flag = FALSE;
    char verbose_flag = FALSE;
    char reset_flag = FALSE;
    extern char *log_filename;

    FILE *log = fopen(log_filename, "r");

    if (!log)  // error de fopen
    {
        char *msg = malloc(sizeof(char) * MSG_LENGTH);
        sprintf(msg, "historial: no se pudo abrir el historial.\n");
        fprintf(stderr, msg);
        err_log_add_msg(msg);
        free(msg);
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
            if (verbose_flag || reset_flag)
            {
                printf("dish: Opcion invalida.\n      La opcion ayuda no toma parametros adicionales.\n");
                return 1;
            }

            help_flag = TRUE;
            break;
        } else if (strcmp(args[i], options[3]) == 0)
        {
            if (verbose_flag || help_flag)
            {
                printf("dish: Opcion invalida.\n      La opcion -r no toma parametros adicionales.\n");
                return 1;
            }

            reset_flag = TRUE;
            break;
        } else if (strcmp(args[i], options[2]) == 0)
        {
            if (help_flag || reset_flag)
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
    } else if (reset_flag)
    {
        return 'r';
    } else
    {
        int c = 0, i = 0;

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
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-g"
    };
    char help_flag = FALSE;
    char group_flag = FALSE;
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
            group_flag = TRUE;
            continue;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"usuario --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[5]);
    } else
    {
        // usuario NOMBRE
        // usuario -g NOMBRE GRUPO

        // se revisa si el nombre es valido
        for (int c = 0; args[i][c] != '\0'; c++)
        {
            if (args[i][c] == '/')
            {
                char *msg = "Caracter invalido.";
                err_print(msg);
                return 2;
            }
        }

        FILE *group, *passwd;

        char *nombre = args[i];
        char *grupo;
        char *tok;
        int gid, uid;
        char *home = malloc(sizeof(char) * FILENAME_LENGTH);
        long unsigned int n = 64;
        char *line = malloc(sizeof(char) * n);
        char group_exists = FALSE;
        i++;

        // Setup del usuario
        passwd = fopen("/etc/passwd", "r");

            // uid y check de nombre
        while (getline(&line, &n, passwd) != -1)
        {
            tok = strtok(line, ":");
            if (strcmp(tok, nombre) == 0)
            {
                printf("usuario: nombre de usuario ya existe.\n");
                err_log_add_msg("nombre de usuario ya existe.");
                fclose(passwd);
                return 2;
            }
        }

            // se crea el uid
        strtok(NULL, ":");
        tok = strtok(NULL, ":");
        uid = atoi(tok) + 1;
        fclose(passwd);

        // Grupo
            // nombre
        if (group_flag)
        {
            grupo = args[i];
        } else
        {
            grupo = nombre;
        }

        group = fopen("/etc/group", "r+");
        
            // Se revisa si el grupo existe
        while (getline(&line, &n, group) != -1)
        {
            tok = strtok(line, ":");
            if (strcmp(tok, grupo) == 0)
            {
                strtok(NULL, ":");
                tok = strtok(NULL, ":");
                gid = atoi(tok);
                group_exists = TRUE;
                break;
            }
        }

            // si no existe se escribe
        if (!group_exists)
        {
            strtok(NULL, ":");
            tok = strtok(NULL, ":");
            gid = atoi(tok) + 1;
            fclose(group);
            group = fopen("/etc/group", "a");
            fprintf(group, "%s:x:%d:\n", grupo, gid);
            fclose(group);
            group = fopen("/etc/gshadow", "a");
            fprintf(group, "%s:!::\n", grupo);
        }

        fclose(group);

        // Creacion del usuario
        passwd = fopen("/etc/passwd", "a");
        sprintf(home, "/home/%s", nombre);
        fprintf(passwd, "%s:x:%d:%d::%s:/bin/bash\n", nombre, uid, gid, home);
        fclose(passwd);
        printf("Usuario %s creado.\n", nombre);
        // /etc/subgid
        // /etc/subuid
        // /etc/login.defs
    }
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
            char *msg = malloc(sizeof(char) * MSG_LENGTH);
            sprintf(msg, "copiar: archivo de origen no especificado.\n");
            fprintf(stderr, msg);
            err_log_add_msg(msg);
            free(msg);
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
                char *msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "copiar: archivo de origen no existe o no se pudo abrir.\n");
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);
            } else
            {
                // si existe
                int c;

                // Cambia el directorio al directorio dado
                curr_dir = getcwd(curr_dir, DIR_BUFSIZE);
                if (chdir(dir) != 0)
                {
                    err_print("dish");
                } else
                {

                    // Se verifica si el archivo de destino ya existe
                    if (new_filename_given)
                    {
                        // Si se dio el nombre nuevo, se alerta al usuario y se interrumpe la copia
                        if ((new_file = fopen(new_filename, "r")))
                        {
                            char *msg = malloc(sizeof(char) * MSG_LENGTH);
                            sprintf(msg, "copiar: archivo de destino ya existe.\n");
                            fprintf(stderr, msg);
                            err_log_add_msg(msg);
                            free(msg);
                            fclose(new_file);
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
                            if (i) sprintf(new_filename, "%s-Copia%d", filename, i);
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
                        char *msg = malloc(sizeof(char) * MSG_LENGTH);
                        sprintf(msg, "copiar: archivo de destino ya existe.\n");
                        fprintf(stderr, msg);
                        err_log_add_msg(msg);
                        free(msg);
                        fclose(new_file);
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
                        sprintf(new_filename, "%s-Copia%d", args[1], i);
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
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-r",
        "--renombrar"
    };
    char help_flag = FALSE;
    char rn_flag = FALSE;
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
        } else if (strcmp(args[i], options[2]) == 0 || strcmp(args[i], options[3]) == 0)
        {
            rn_flag = TRUE;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"mover --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[8]);
    } else
    {
        // Se usa copiar para copiar el archivo
        // Se usa remover para eliminar el archivo de origen
        // args[i] = archivo a mover
        // args[i+1] = directorio de destino
        //   o
        // args[i] = archivo a mover
        // args[i+1] = nombre nuevo
        // args[i+2] = directorio de destino

        // Renombramiento
        char rn_arg0[] = "renombrar";
        char rn_arg1[] = "-s";
        char *rn_arg2 = args[i];
        char *rn_arg3 = args[i + 1];
        char *rn_args[5] = { rn_arg0, rn_arg1, rn_arg2, rn_arg3, NULL };
        dish_rn(rn_args);

        // Copia
        char cp_arg0[] = "copiar";
        char cp_arg1[] = "-d";
        char *cp_arg2 = (rn_flag) ? args[i + 1] : args[i];
        char *cp_arg3 = (rn_flag) ? args[i + 2] : args[i + 1];
        char *cp_args[5] = { cp_arg0, cp_arg1, cp_arg2, cp_arg3, NULL };
        dish_cp(cp_args);

        // Eliminacion
        char rm_arg0[] = "remover";
        char rm_arg1[] = "-f";
        char rm_arg2[] = "-s";
        char *rm_arg3 = (rn_flag) ? args[i + 1] : args[i];
        char *rm_args[5] = { rm_arg0, rm_arg1, rm_arg2, rm_arg3, NULL };
        dish_rm(rm_args);
    }
    return 1;
}

// Renombra un archivo
int dish_rn(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-s",
        "--silencio"
    };
    char help_flag = FALSE;
    char quiet_flag = FALSE;
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
        } else if (strcmp(args[i], options[2]) == 0 || strcmp(args[i], options[3]) == 0)
        {
            quiet_flag = TRUE;
            continue;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"renombrar --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[9]);
    } else
    {
        // args[i] = archivo
        // args[i+1] = nombre nuevo
        
        int err = rename(args[i], args[i + 1]);
        if (err)
        {
            err_print("renombrar");
        }
    }
    return 1;
}

// Crea un nuevo directorio
int dish_mkdir(char **args)
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
            printf("dish: Opcion invalida.\n      Ingresa \"creardir --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[10]);
    } else
    {
        // args[1] = nombre del directorio
        if (args[1] == NULL)
        {
            char *msg = malloc(sizeof(char) * MSG_LENGTH);
            sprintf(msg, "creardir: nombre no especificado.\n");
            fprintf(stderr, msg);
            err_log_add_msg(msg);
            free(msg);
        } else
        {
            int err = mkdir(args[1], 0777);
            if (err)
            {
                err_print("creardir");
            }
        }
    }
    return 1;
}

// Elimina un archivo
int dish_rm(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-f",
        "-s",
        "--silencio"
    };
    char help_flag = FALSE;
    char force_flag = FALSE;
    char recursive_flag = FALSE;
    char quiet_flag = FALSE;
    char rm = FALSE;
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
            force_flag = TRUE;
            continue;
        } else if (strcmp(args[i], options[3]) == 0 || strcmp(args[i], options[4]) == 0)
        {
            quiet_flag = TRUE;
            continue;
        } else 
        {
            printf("dish: Opcion invalida.\n      Ingresa \"remover --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[11]);
    } else
    {
        FILE *file = NULL;
        char *curr_dir = malloc(sizeof(char) * DIR_BUFSIZE);
        
        // Check para ver si args[i] es directorio
        if (!recursive_flag)
        {
            curr_dir = getcwd(curr_dir, DIR_BUFSIZE);
            if (chdir(args[i]) == 0)
            {
                char *msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "remover: no se pudo remover, %s es directorio.\n", args[i]);
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);

                chdir(curr_dir);
                free(curr_dir);
                return 1;
            }
        }
        free(curr_dir);

        // Check para ver si existe el archivo
        file = fopen(args[i], "r");
        if (file)
        {
            fclose(file);
        } else
        {
            char *msg = malloc(sizeof(char) * MSG_LENGTH);
            sprintf(msg, "remover: el archivo no existe.\n");
            fprintf(stderr, msg);
            err_log_add_msg(msg);
            free(msg);
            return 1;
        }

        // Confirmacion
        if (!force_flag)
        {
            int bufsize = 16;
            char buffer[bufsize];
            char **rm_args;
            int c, j = 0;

            printf("remover: Desea eliminar el archivo %s? (s/n) ", args[i]);
            while (1)
            {
                c = getchar();

                if (c == EOF || c == '\n')
                {
                    buffer[j] = '\0';
                    break;
                } else if (j >= bufsize)
                {
                    buffer[j] = '\0';
                    // flush buffer
                    while ((c = getchar()) != '\n' && c != EOF);
                    break;
                } else
                {
                    buffer[j] = c;
                    j++;
                }
            }

            rm_args = dish_split_line(buffer);
            if (strcmp(rm_args[0], "s") == 0 || strcmp(rm_args[0], "S") == 0)
            {
                rm = TRUE;
            } else
            {
                rm = FALSE;
            }    

            free(rm_args);
        } else
        {
            rm = TRUE;
        }
        
        // Eliminacion
        if (rm)
        {
            int res = remove(args[i]);
            if (res != 0)
            {
                err_print("remover");
            }  
        }
    }
    return 1;
}

// Elimina un directorio
int dish_rmdir(char **args)
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
            printf("dish: Opcion invalida.\n      Ingresa \"removerdir --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[12]);
    } else
    {
        int err = rmdir(args[1]);
        if (err)
        {
            err_print("removerdir");
        }
    }
    return 1;
}

// Lista archivos y directorios
int dish_ls(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda",
        "-a"
    };
    char help_flag = FALSE;
    char all_flag = FALSE;
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
            all_flag = TRUE;
            continue;
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
        dish_print_help(builtin_str[13]);
    } else
    {
        DIR *d;
        struct dirent *dir;
        int i = 0;

        d = opendir(".");
        if (d)
        {
            int n = 0;
            int size = 10;
            char **dirs = malloc(sizeof(char*) * size);
            while ((dir = readdir(d)) != NULL)
            {
                // evita listar archivos ocultos sin la opcion -a
                if (dir->d_name[0] == '.' && !all_flag)
                    continue;
                else
                {
                    dirs[n] = dir->d_name;
                    n++;
                    if (size <= n)
                    {
                        size += 10;
                        dirs = realloc(dirs, sizeof(char*) * size);
                    }
                }
            }

            // ordena la lista de directorios
            dish_sort(dirs, n);

            // imprime lista ordenada
            for (i = 0; i < n; i++)
                printf("%s\n", dirs[i]);

            free(dirs);
            closedir(d);
        }
    }
    return 1;
}