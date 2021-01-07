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
char err_filename[FILENAME_LENGTH];
char user_horarios_log_filename[FILENAME_LENGTH];
char *home;

// para que otros archivos tengan acceso a traves de extern
char *line;
char **args;

// Setea username y hostname
void check_user()
{
    username = getlogin();
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
    sprintf(log_filename, "%s/.dish_log", home);

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
        fprintf(file, "LOG creado %d-%02d-%02d %02d:%02d:%02d\n\n", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
        fclose(file);
    }

    // Errores
    sprintf(err_filename, "%s/.dish_err", home);

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
    sprintf(user_horarios_log_filename, "%s/.dish_%s_horarios_log", home, username);
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

int main (int argc, char **argv)
{
    // Inicializacion
    check_user();
    check_logs();

    // Main command loop
    dish_loop();

    // Shutdown

    return SUCCESS;
}
