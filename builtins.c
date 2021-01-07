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
#include <termios.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <crypt.h>
#include <time.h>

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
    "listar",
    "permisos",
    "propietario",
    "uinfo"
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
    &dish_ls,
    &dish_chmod,
    &dish_chown,
    &dish_userinfo
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
        sprintf(msg, "no se pudo abrir el archivo %s\n", filename);
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
        sprintf(msg, "no se pudo abrir el historial.\n");
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

        // Privilegios
        if (strcmp(username, "root") != 0)
        {
            printf("usuario: acceso denegado.\n\tEste comando esta reservado para el usuario root.\n");
            char msg[] = "acceso denegado.";
            err_log_add_msg(msg);
            return 2;
        }

        // se revisa si el nombre es valido
        for (int c = 0; args[i][c] != '\0'; c++)
        {
            if (args[i][c] == '/')
            {
                char msg[] = "Caracter invalido en el nombre de usuario.";
                err_log_add_msg(msg);
                return 2;
            }
        }

        // se revisa si el grupo es valido
        if (group_flag)
        {
            for (int c = 0; args[i + 1][c] != '\0'; c++)
            {
                if (args[i + 1][c] == '/')
                {
                    char msg[] = "Caracter invalido en el nombre del grupo.";
                    err_log_add_msg(msg);
                    return 2;
                }
            }
        }

        FILE *group, *passwd_f;

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
        struct passwd *passwd_s = getpwnam(nombre);
        if (passwd_s != NULL)
        {
            printf("usuario: nombre de usuario ya existe.\n");
            char msg[] = "nombre de usuario ya existe.";
            err_log_add_msg(msg);
            return 2;
        }

            // se crea el uid
        passwd_f = fopen("/etc/passwd", "r");
        while (getline(&line, &n, passwd_f) != -1)
        tok = strtok(line, ":");
        passwd_f = fopen("/etc/passwd", "r");
        strtok(NULL, ":");
        tok = strtok(NULL, ":");
        uid = atoi(tok) + 1;
        fclose(passwd_f);

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
        passwd_f = fopen("/etc/passwd", "a");
        sprintf(home, "/home/%s", nombre);
        fprintf(passwd_f, "%s:x:%d:%d::%s:/bin/bash\n", nombre, uid, gid, home);
        fclose(passwd_f);
        mkdir(home, 0777);
        printf("Usuario %s creado. Completa informacion adicional con uinfo.\n", nombre);
        
            // shadow
        FILE *fp = fopen("/etc/shadow", "a");
        fprintf(fp, "%s:!!:%d:%d:%d:%d:::\n", nombre, (int)time(NULL) / 86400, 0, 99999, 7);
        fclose(fp);
    }
    return 1;
}

// Modifica la contrasenha del usuario dado
int dish_passwd(char **args)
{

    // Esta funcion no funciona de la forma que quiero, por lo que se queda sin terminar
    // por ahora. Si esto llega a la version final que pena.
    return 1;

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
            printf("dish: Opcion invalida.\n      Ingresa \"contrasena --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[6]);
    } else
    {
        // contrasena nombre
        char *nombre = args[1];
        if (nombre == NULL)
            nombre = username;

        // Usuario existe?
        struct passwd *passwd_s = getpwnam(nombre);
        if (passwd_s == NULL)
        {
            printf("contrasena: usuario %s no existe.\n", nombre);
            char msg[] = "nombre de usuario no existe.";
            err_log_add_msg(msg);
            return 2;
        }

        // Privilegios
        if (strcmp(username, "root") != 0)
        {
            printf("contrasena: acceso denegado.\n\tEste comando esta reservado para el usuario root.\n");
            char msg[] = "acceso denegado.";
            err_log_add_msg(msg);
            return 2;
        }

        // se recupera la entrada actual de /etc/shadow si existe
        setspent();
        lckpwdf();

        struct spwd *entry;
        struct spwd *rpwd;

        rpwd = getspnam(username);

        // si no existe una entrada para el usuario actual, entry = NULL
        while ((entry = getspent()) != NULL)
        {
            if (strcmp(entry->sp_namp, nombre) == 0)
            {
                break;  // se encontro la entrada correcta
            }
        }

        ulckpwdf();
        endspent();

        // se deshabilita el ECHO de stdin
        struct termios term;
        tcgetattr(fileno(stdin), &term);

        term.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), 0, &term);

        // entrada del pass
        char pass[100];
        char c;
        int i;
        printf("Password:");
        fgets(pass, sizeof(pass), stdin);
        for (i = 0; pass[i] != '\n'; i++);
        if (pass[i] == '\n') pass[i] = '\0';     // se elimina el '\n' final

        char *auxstr = NULL;

        // verificaciones
        if (strlen(pass) == 0)
        {
            char hpass[] = "!!";
            entry->sp_pwdp = hpass;
        } else
        {
            // hashing
            // crypt toma la frase y una frase encriptada valida como "setting" y retorna la frase encriptada
            char *hpass = crypt(pass, rpwd->sp_pwdp);
            char *auxstr = malloc(sizeof(char) * strlen(hpass));
            strcpy(auxstr, hpass);
            entry->sp_pwdp = auxstr;
        }

        // cambio
            // ultimo cambio en dias desde el 1/1/1970
        entry->sp_lstchg = (long int)time(NULL) / 86400;

        // testing to see if it even works
        FILE *newshadow = fopen("newshadow", "w");

        setspent();
        lckpwdf();

        struct spwd *aux;

        // si no existe una entrada para el usuario actual, aux = NULL
        while ((aux = getspent()) != NULL)
        {
            if (strcmp(aux->sp_namp, nombre) == 0)
            {
                printf("\n%d\n%s:%s:%ld:0:99999:7:::\n", strlen(pass), entry->sp_namp, entry->sp_pwdp, entry->sp_lstchg);
                putspent(entry, newshadow);
            } else
            {
                putspent(aux, newshadow);
            }
        }

        ulckpwdf();
        endspent();

        fclose(newshadow);
        // remove("/etc/shadow");
        // rename("/etc/newshadow", "/etc/shadow");

        if (auxstr) free(auxstr);

        // se rehabilita ECHO de stdin
        term.c_lflag |= ECHO;
        tcsetattr(fileno(stdin), 0, &term);
        printf("\n");
    }
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
            sprintf(msg, "archivo de origen no especificado.\n");
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
                sprintf(msg, "archivo de origen no existe o no se pudo abrir.\n");
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
                            sprintf(msg, "archivo de destino ya existe.\n");
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
                        sprintf(msg, "archivo de destino ya existe.\n");
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
            sprintf(msg, "nombre no especificado.\n");
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
                sprintf(msg, "no se pudo remover, %s es directorio.\n", args[i]);
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
            sprintf(msg, "el archivo no existe.\n");
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

// Cambia los permisos del archivo o directorio dado
int dish_chmod(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda"
    };
    char help_flag = FALSE;
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
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"permisos --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[14]);
    } else
    {
        // args[0] = permisos
        // args[i] = filename
        // args[i+1] = mode (777)

        if (args[i] == NULL)
        {
            char *msg = malloc(sizeof(char) * MSG_LENGTH);
            sprintf(msg, "modo no dado.\n");
            fprintf(stderr, msg);
            err_log_add_msg(msg);
            free(msg);
            return 1;
        }

        struct stat _stat;
        stat(args[i], &_stat);

        int mode = 0;
        // se convierte el modo dado a un numero en octal igual
        for (int j = 0; j < 3; j++)
        {
            if (args[i + 1][j] >= '0' && args[i + 1][j] <= '7')
            {
                mode += (args[i + 1][j] - '0') * dish_pow((double)8, (double)(2 - j));
            } else
            {
                char *msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "modo invalido.\n");
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);
                break;
            }
        }

        printf("mode = %d\n", mode);
        printf("filetype = %d\n", _stat.st_mode & S_IFMT);

        printf("chmod return = %d\n", chmod(args[i], _stat.st_mode & S_IFMT + mode));
    }
    return 1;
}

// Cambia el propietario del archivo o directorio dado
int dish_chown(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda"
    };
    char help_flag = FALSE;

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
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"propietario --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[15]);
    } else
    {
        // args[i] = path/name
        // args[i+1] = usuario
        // args[i+2] = grupo

        uid_t uid;
        gid_t gid;
        struct passwd *pwd;
        struct group *grp;
        char *msg;
        char *grupo;

        // se busca el uid
        pwd = getpwnam(args[i + 1]);
        if (pwd == NULL)
        {
            msg = malloc(sizeof(char) * MSG_LENGTH);
            sprintf(msg, "no se pudo encontrar el usuario.\n");
            fprintf(stderr, msg);
            err_log_add_msg(msg);
            free(msg);
            return 1;
        }
        uid = pwd->pw_uid;

        // se busca el gid
        if (args[i + 2] == NULL)    // si no se da grupo el comando asume que el grupo se llama como el usuario
        {
            grupo = args[i + 1];
        } else grupo = args[i + 2];

        grp = getgrnam(grupo);
        if (grp == NULL)
        {
            msg = malloc(sizeof(char) * MSG_LENGTH);
            sprintf(msg, "no se pudo encontrar el grupo.\n");
            fprintf(stderr, msg);
            err_log_add_msg(msg);
            free(msg);
            return 1;
        }
        gid = grp->gr_gid;

        // se ejecuta chown
        if (chown(args[i], uid, gid) == -1)
        {
            err_print("propietario");
        }
    }
    return 1;
}

// Agrega o modifica informacion sobre el usuario
int dish_userinfo(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda"
    };
    char help_flag = FALSE;

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
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"uinfo --ayuda\" para ver las opciones disponibles.\n");
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[16]);
    } else
    {
        // args[i] = usuario

        char uinfo_filename[FILENAME_LENGTH];
        FILE *log;

        // Archivo de texto que almacena la informacion de usuario
        sprintf(uinfo_filename, "%s/.dish_%s", home, username);
        if ((log = fopen(uinfo_filename, "r")) != NULL)
        {
            // No existe el archivo y se crea
            time_t t = time(NULL);
            struct tm tms = *localtime(&t);

            fclose(log);
            log = fopen(uinfo_filename, "w+");

            fprintf(log, "UINFO creado %d-%02d-%02d %02d:%02d:%02d\n\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
        } else
        {
            // Existe el archivo y se escribe al final
            fclose(log);
            log = fopen(uinfo_filename, "w+");
        }

        // Se pregunta por las informaciones adicionales

        int hh_i = -1, mm_i = -1;
        int hh_f = -1, mm_f = -1;
        char *lugares[HOST_NAME_MAX];
        char buf[BUFSIZ];
        char done;

            // Horarios
        printf("# Horario de trabajo\n");
        // hora inicial
        do {
            done = TRUE;
            printf(" Hora de inicio (por defecto 00:00): ");
            fgets(buf, BUFSIZ, stdin);
            if (buf[0] == '\n')
            {
                hh_i = 0;
                mm_i = 0;
            } else
            {
                hh_i = atoi(strtok(buf, ":"));
                mm_i = atoi(strtok(NULL, ""));
                if (hh_i < 0 || hh_i > 23 || mm_i < 0 || mm_i > 59)
                {
                    printf("   Hora invalida, ingresa un numero entre 0 y 23 seguido de ':' y otro numero entre 0 y 59.\n");
                    done = FALSE;
                    continue;
                }
            }
        } while (!done);

        // hora final
        do {
            done = TRUE;
            printf(" Hora de cierre (por defecto 23:59): ");
            fgets(buf, BUFSIZ, stdin);
            if (buf[0] == '\n')
            {
                hh_i = 23;
                mm_i = 59;
            } else
            {
                hh_i = atoi(strtok(buf, ":"));
                mm_i = atoi(strtok(NULL, ""));
                if (hh_i < 0 || hh_i > 23 || mm_i < 0 || mm_i > 59)
                {
                    printf("   Hora invalida, ingresa un numero entre 0 y 23 seguido de ':' y otro numero entre 0 y 59.\n");
                    done = FALSE;
                    continue;
                }
            }
        } while (!done);

            // Lugares de trabajo
        

        fclose(log);
    }
    return 1;
}