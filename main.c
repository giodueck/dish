/*
    Giovanni Dueck's Interactive Shell - DISH

    Sistemas Operativos 1
    Desarrollo del Shell

    Giovanni Dueck
*/

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "dish.h"

char *username;
char hostname[HOST_NAME_MAX];
char *hostname_short;
char log_filename[FILENAME_LENGTH];
char *err_filename = "/var/log/shell/sistema_error.log";
char user_horarios_log_filename[FILENAME_LENGTH];
char *home;

// para que otros archivos tengan acceso a traves de extern
char *line;
char **args;

// Setea username y hostname
void check_user()
{
    // username = getlogin();
    username = getenv("USER");
    gethostname(hostname, sizeof(hostname));
    hostname_short = strtok(hostname, ".");
    home = malloc(sizeof(char) * DIR_BUFSIZE);
    if (strcmp(username, "root") == 0)
    {
        sprintf(home, "/root");
    } else
    {
        sprintf(home, "/home/%s", username);
    }
}

// Revisa si los archivos log necesarios existen, y si no, los crea
void check_logs()
{
    // Se revisa si existen los archivos
    FILE *file;

    // Historial
    sprintf(log_filename, "/var/log/shell/.dish_history_%s", username);

    if ((file = fopen(log_filename, "r")))
    {
        // Existe el archivo
        fclose(file);
    } else
    {
        // No existe el archivo y se crea
        time_t t = time(NULL);
        struct tm tms = *localtime(&t);

        file = fopen(log_filename, "w");
        if (!file)
        {
            printf("Error al crear el archivo de historial.\n");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "LOG creado %d-%02d-%02d %02d:%02d:%02d\n\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
        fclose(file);
    }

    // Errores
    if ((file = fopen(err_filename, "r")))
    {
        // Existe el archivo
        fclose(file);
    } else
    {
        // No existe el archivo y se crea
        time_t t = time(NULL);
        struct tm tms = *localtime(&t);

        file = fopen(err_filename, "w");
        fprintf(file, "ERROR LOG creado %d-%02d-%02d %02d:%02d:%02d\n\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
        fclose(file);
    }

    // Horarios
    sprintf(user_horarios_log_filename, "/var/log/shell/%s_horarios_log", username);

    if ((file = fopen(user_horarios_log_filename, "r")))
    {
        // Existe el archivo
        fclose(file);
    } else
    {
        // No existe el archivo y se crea
        time_t t = time(NULL);
        struct tm tms = *localtime(&t);

        file = fopen(user_horarios_log_filename, "w");
        fprintf(file, "LOG DE HORARIOS creado %d-%02d-%02d %02d:%02d:%02d\n\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
        fclose(file);
    }
}

// Agrega una entrada al log de historial
void log_add(char *line)
{
    if (line[0] == '\0')
    {
        return;
    }

    time_t t = time(NULL);
    struct tm tms = *localtime(&t);
    FILE *log = fopen(log_filename, "a");
    
    fprintf(log, "[%d-%02d-%02d %02d:%02d:%02d] %s\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec, line);
    
    fclose(log);
}

// Loop central del shell
void dish_loop()
{
    char *current_dir = malloc (sizeof(char) * DIR_BUFSIZE);
    int status = 0;

    if (!current_dir) // error de malloc
    {
        fprintf(stderr, "dish: allocation error\n");
        exit(FAILURE);
    }

    do
    {
        // se resetea el historial
        if (status == 'r')
        {
            remove(log_filename);
            check_logs();
        }
        // Se imprime el directotio actual y se lee el comando a ejecutarse
        current_dir = getcwd(current_dir, DIR_BUFSIZE);
        printf("[%s@%s %s]\n", username, hostname, current_dir);
        printf("> ");
        line = dish_read_line();
        log_add(line);  // Historial
        args = dish_split_line(line);
        status = dish_execute(args);

        // Como a line y args se asignan bloques de memoria dinamica hay que liberarlos
        free(line);
        free(args);

        // Mientras que status != 0, el shell sigue leyendo comandos
    } while (status);
    
    free(current_dir);
}

void dish_log(char mode)
{
    FILE *uinfof, *user_log;
    char uinfo_filename[FILENAME_MAX];
    struct uinfo info;
    char lugar[HOST_NAME_MAX];
    char host[HOST_NAME_MAX];
    int dif;
    char done;

    // hora actual
    time_t t = time(NULL);
    struct tm tms = *localtime(&t);

    // lectura de datos del usuario
    sprintf(uinfo_filename, "%s/.dish_%s", home, username);
    uinfof = fopen(uinfo_filename, "rb");
    user_log = fopen(user_horarios_log_filename, "a");

    if (uinfof)
        fread(&info, sizeof(struct uinfo), 1, uinfof);
    else
    {   // por si no hay un archivo con uinfo
        info.hh_i = 0;
        info.hh_f = 23;
        info.mm_i = 0;
        info.mm_f = 59;
        info.num_lugares = 0;
    }

    gethostname(host, HOST_NAME_MAX);

    // login
    if (mode == 'i')
    {
        fprintf(user_log, "LOGIN: %d-%02d-%02d %02d:%02d\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min);
        fprintf(user_log, " HORARIO: %02d:%02d; ", info.hh_i, info.mm_i);
        dif = tms.tm_hour * 60 + tms.tm_min - info.hh_i * 60 - info.mm_i;
        if (uinfof)
        {
            if (dif < 0) fprintf(user_log, "TEMPRANO: %02d:%02d\n", -dif / 60, -dif % 60);
            else if (dif > 0) fprintf(user_log, "TARDE: %02d:%02d\n", dif / 60, dif % 60);
            else fprintf(user_log, "A TIEMPO\n");
        }
        fprintf(user_log, " LUGAR DE CONEXION: %s", host);
        done = FALSE;
        for (int i = 0; i < info.num_lugares; i++)
        {
            fread(lugar, sizeof(char) * HOST_NAME_MAX, 1, uinfof);
            if (strcmp(host, lugar) == 0)
            {
                done = TRUE;
                break;
            }
        }
        if (!done && uinfof)
        {
            fprintf(user_log, "; INUSUAL");
        }
        fprintf(user_log, "\n");
    } else if (mode == 'o')  // logout
    {
        fprintf(user_log, "LOGOUT: %d-%02d-%02d %02d:%02d\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min);
        fprintf(user_log, " HORARIO: %02d:%02d; ", info.hh_f, info.mm_f);
        dif = tms.tm_hour * 60 + tms.tm_min - info.hh_f * 60 - info.mm_f;
        if (uinfof)
        {
            if (dif < 0) fprintf(user_log, "TEMPRANO: %02d:%02d\n", -dif / 60, -dif % 60);
            else if (dif > 0) fprintf(user_log, "TARDE: %02d:%02d\n", dif / 60, dif % 60);
            else fprintf(user_log, "A TIEMPO\n");
        }
        fprintf(user_log, "\n");
    }

    if (uinfof) fclose(uinfof);
    fclose(user_log);
}

int main (int argc, char **argv)
{
    // Inicializacion
    check_user();
    check_logs();
    dish_log('i');

    // Main command loop
    dish_loop();

    // Shutdown
    dish_log('o');

    return SUCCESS;
}
