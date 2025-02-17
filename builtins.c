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

extern char *username;
extern char *home;

// Imprime la ayuda de un comando guardada en help/command_name.txt
int dish_print_help(char *command_name)
{
    char *filename;
    char *install_dir = "/programs";
    int c;
    FILE *file;
    
    filename = malloc(FILENAME_MAX * sizeof(char));
    if (!filename)  // error de malloc
    {
        fprintf(stderr, "dish: allocation error\n");
        exit(FAILURE);
    }

    // Se construye el camino absoluto del archivo
    filename[0] = '\0';
    filename = strcat(filename, install_dir);
    filename = strcat(filename, "/dish-help/");
    filename = strcat(filename, command_name);
    filename = strcat(filename, ".txt");
    file = fopen(filename, "r");

    if (!file)  // error de fopen
    {
        char *msg = malloc(sizeof(char) * MSG_LENGTH);
        sprintf(msg, "no se pudo abrir el archivo %s o no existe ayuda para el comando.\n", filename);
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
    if (file) fclose(file);
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
                printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
    char log_filename[FILENAME_MAX];

    sprintf(log_filename, "%s/.dish_history", getpwnam(getenv("USER"))->pw_dir);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
        char *home = malloc(sizeof(char) * FILENAME_MAX);
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
        fprintf(passwd_f, "%s:x:%d:%d::%s:/programs/bin/dish\n", nombre, uid, gid, home);
        fclose(passwd_f);

        // se crea el directorio home para el usuario nuevo
        char command[100];
        char **args_;

        mkdir(home, 0777);
        sprintf(command, "sys chown -hR %s:%s %s", nombre, grupo, home);
        args_ = dish_split_line(command);
        dish_sys(args_);    // se usa sys para que el comando se ejecute en un subproceso
        
        printf("Usuario %s creado. Completa informacion adicional con uinfo. Crea una contrasena con contrasena.\n", nombre);
        
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
    // Para que el programa en si funcione hice que use el comando sys para llamar al comando
    // passwd

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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
        char command[100], **args_;
        sprintf(command, "sys passwd");
        if (args[i])
        {
            strcat(command, " ");
            strcat(command, args[i]);
        }
        args_ = dish_split_line(command);
        dish_sys(args_);
        
        /*
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
        */
    }
    return 1;
}

// Copia un archivo
int dish_cp(char **args)
{
    char *options[] = 
    {
        "-h",
        "--ayuda"
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
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
        } else
        {
            // args[1] = nombre del archivo
            // args[2] = nombre del nuevo archivo (opcional)

            // Nombres de archivos
            char *filename = args[1];
            char *new_filename = args[2];
            char new_filename_given = (new_filename) ? TRUE : FALSE;
            if (!new_filename_given)
            {
                new_filename = malloc(sizeof(char) * FILENAME_MAX);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
        if (rn_flag)
        {
            char rn_arg0[] = "renombrar";
            char *rn_arg1 = args[i];
            char *rn_arg2 = args[i + 1];
            char *rn_args[4] = { rn_arg0, rn_arg1, rn_arg2, NULL };
            dish_rn(rn_args);
        }

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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
        "-f"
    };
    char help_flag = FALSE;
    char force_flag = FALSE;
    char recursive_flag = FALSE;
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
        } else 
        {
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
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
        "--ayuda",
        "-s"
    };
    char help_flag = FALSE;
    char show_flag = FALSE;

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
            show_flag = TRUE;
            continue;
        } else
        {
            printf("dish: Opcion invalida.\n      Ingresa \"%s --ayuda\" para ver las opciones disponibles.\n", args[0]);
            // en caso de opcion invalida se termina la ejecucion del comando
            return 1;
        }
    }

    // Ejecucion
    if (help_flag)
    {
        dish_print_help(builtin_str[16]);
    } else if (show_flag)
    {
        // args[i] = usuario
        struct passwd *pwd;
        struct uinfo info;
        char *name, *homedir;
        char *msg;
        char uinfo_filename[FILENAME_MAX];
        char aux[HOST_NAME_MAX];
        FILE *log;

        if (args[i] != NULL)
        {
            // solo root puede modificar uinfo de otros usuarios
            if (strcmp(username, "root") != 0)
            {
                msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "acceso denegado.\n");
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);
                return 1;
            }
            // se busca al usuario dado
            if ((pwd = getpwnam(args[i])) == 0)
            {
                msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "no se pudo encontrar el usuario.\n");
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);
                return 1;
            } else
            {
                homedir = pwd->pw_dir;
                name = pwd->pw_name;
            }
        } else
        {
            // se busca el passwd del usuario actual
            pwd = getpwnam(username);
            homedir = pwd->pw_dir;
            name = pwd->pw_name;
        }

        // Archivo binario que almacena la informacion de usuario
        sprintf(uinfo_filename, "%s/.dish_%s", homedir, name);
        if ((log = fopen(uinfo_filename, "rb")) == NULL)
        {
            printf("No existe informacion adicional sobre el usuario %s.\n", name);
            return 1;
        }

        // se lee el struct uinfo y los lugares de conexion y se imprimen los datos
        fread(&info, sizeof(struct uinfo), 1, log);

        printf("Hora de inicio: %02d:%02d\n", info.hh_i, info.mm_i);
        printf("Hora de cierre: %02d:%02d\n", info.hh_f, info.mm_f);
        printf("Lugares de conexion: %d\n", info.num_lugares);
        for (int i = 0; i < info.num_lugares; i++)
        {
            fread(aux, sizeof(char) * HOST_NAME_MAX, 1, log);
            printf("  %d: %s\n", i + 1, aux);
        }
        fclose(log);
    } else
    {

        // args[i] = usuario
        struct passwd *pwd;
        struct uinfo info;
        char *name, *homedir;
        char *msg;
        char uinfo_filename[FILENAME_MAX];
        FILE *log;

        if (args[i] != NULL)
        {
            // solo root puede modificar uinfo de otros usuarios
            if (strcmp(username, "root") != 0)
            {
                msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "acceso denegado.\n");
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);
                return 1;
            }
            // se busca al usuario dado
            if ((pwd = getpwnam(args[i])) == 0)
            {
                msg = malloc(sizeof(char) * MSG_LENGTH);
                sprintf(msg, "no se pudo encontrar el usuario.\n");
                fprintf(stderr, msg);
                err_log_add_msg(msg);
                free(msg);
                return 1;
            } else
            {
                homedir = pwd->pw_dir;
                name = pwd->pw_name;
            }
        } else
        {
            // se busca el passwd del usuario actual
            pwd = getpwnam(username);
            homedir = pwd->pw_dir;
            name = pwd->pw_name;
        }

        // Archivo binario que almacena la informacion de usuario
        sprintf(uinfo_filename, "%s/.dish_%s", homedir, name);
        log = fopen(uinfo_filename, "wb");
        
        // Se pregunta por las informaciones adicionales
        
        info.hh_i = -1;
        info.mm_i = -1;
        info.hh_f = -1;
        info.mm_f = -1;
        info.num_lugares = -1;

        char **lugares;
        char buf[BUFSIZ];
        char done;
        char *tok1, *tok2;

            // Horarios
        printf("# Horario de trabajo\n");
        // hora inicial
        do {
            done = TRUE;
            printf(" Hora de inicio (por defecto 00:00): ");
            fgets(buf, BUFSIZ, stdin);
            if (buf[0] == '\n')
            {
                info.hh_i = 0;
                info.mm_i = 0;
            } else
            {
                tok1 = strtok(buf, ":");
                if (strlen(tok1) == 0) info.hh_i = -1;
                else info.hh_i = atoi(tok1);

                tok2 = strtok(NULL, "");
                if (tok2 == NULL || strlen(tok2) == 0) info.mm_i = -1;
                else info.mm_i = atoi(tok2);

                if (info.hh_i < 0 || info.hh_i > 23 || info.mm_i < 0 || info.mm_i > 59)
                {
                    printf("   Hora invalida, formato debe ser hh:mm, h:mm o h:m\n");
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
                info.hh_f = 23;
                info.mm_f = 59;
            } else
            {
                tok1 = strtok(buf, ":");
                if (strlen(tok1) == 0) info.hh_f = -1;
                else info.hh_f = atoi(tok1);

                tok2 = strtok(NULL, "");
                if (tok2 == NULL || strlen(tok2) == 0) info.mm_f = -1;
                else info.mm_f = atoi(tok2);

                if (info.hh_f < 0 || info.hh_f > 23 || info.mm_f < 0 || info.mm_f > 59)
                {
                    printf("   Hora invalida, formato debe ser hh:mm, h:mm o h:m\n");
                    done = FALSE;
                    continue;
                }
            }
        } while (!done);

            // Lugares de trabajo
        printf("# Lugares de conexion\n");
        
        // cantidad de lugares
        do
        {
            done = TRUE;
            printf(" Cantidad de lugares (por defecto 1): ");
            fgets(buf, BUFSIZ, stdin);
            if (buf[0] == '\n')
            {
                info.num_lugares = 1;
            } else
            {
                info.num_lugares = atoi(buf);
                if (info.num_lugares < 1)
                {
                    printf("   Numero invalido, debe ser >= 1\n");
                    done = FALSE;
                    continue;
                }
            }
        } while (!done);

        lugares = malloc(sizeof(char*) * info.num_lugares);

        // nomobres de lugares
        printf(" Ingrese el nombre o la direccion IP de cada lugar.\n Dejar en blanco una entrada para terminar antes.\n");
        for (int j = 0; j < info.num_lugares; j++)
        {
                done = TRUE;
                printf(" Lugar %d: ", j + 1);
                lugares[j] = malloc(sizeof(char) * HOST_NAME_MAX);
                fgets(lugares[j], HOST_NAME_MAX, stdin);
                for (int x = 0; x < HOST_NAME_MAX; x++)
                    if (lugares[j][x] == '\n')
                        lugares[j][x] = '\0';
                if (lugares[j][0] == '\0')
                {
                    // terminar antes de alcanzar info.num_lugares
                    info.num_lugares = j;
                    free(lugares[j]);
                }
        }

            // Escribir al log en binario
        // el struct con horarios y numero de lugares
        fwrite(&info, sizeof(struct uinfo), 1, log);
        // seguido de los lugares
        for (int j = 0; j < info.num_lugares; j++)
        {
            fwrite(lugares[j], sizeof(char) * HOST_NAME_MAX, 1, log);
        }        

        fclose(log);
        if (args[i] && strcmp(getenv("USER"), "root") == 0)
        {
            // se cambia el propietario al usuario en cuestion para que pueda modificar y leer su propia uinfo
            char comando[100], **args_;
            sprintf(comando, "sys chown %s %s", args[i], uinfo_filename);
            args_ = dish_split_line(comando);
            dish_sys(args_);
        }
        for (int j = 0; j < info.num_lugares; j++) free(lugares[j]);
        free(lugares);
    }
    return 1;
}