/*
    Funciones relacionadas a errores
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "err.h"

extern char *err_filename;
extern char **args;

// Escribe en una cadena un mensaje para el valor de errno
// Retorna la cadena con el mensaje de error
// Retorna NULL en caso de fallo
char *get_err_msg(char *str)
{
    switch (errno)
    {
    case EACCES:
        sprintf(str, "Acceso denegado.");
        return str;
    case ENOENT:
        sprintf(str, "Archivo o directorio no existe.");
        return str;
    case ENOTDIR:
        sprintf(str, "No es directorio.");
        return str;
    case EAGAIN:
        sprintf(str, "Error de sistema.");
        return str;
    case ENOMEM: case ENOSPC:
        sprintf(str, "Memoria insuficiente.");
        return str;
    case ENOSYS:
        sprintf(str, "Funcion no soportada por el sistema.");
        return str;
    case EEXIST:
        sprintf(str, "Archivo o directorio ya existe.");
        return str;
    case EROFS:
        sprintf(str, "Directorio de solo-lectura.");
        return str;
    case ENOTEMPTY:
        sprintf(str, "Directorio no vacio.");
        return str;
    case EBUSY:
        sprintf(str, "Archivo o directorio en uso.");
        return str;

    default:
        return NULL;
    }
}

// Escribe la fecha y hora en el log de error
void err_put_date(FILE *file)
{
    time_t t = time(NULL);
    struct tm tms = *localtime(&t);
    
    fprintf(file, "[%d-%02d-%02d %02d:%02d:%02d] ", tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday, tms.tm_hour, tms.tm_min, tms.tm_sec);
}

// Ya que line fue tokenizado, esta funcion recompone la linea para escribir en el log de error
char *compose_line(char *dest)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        strcat(dest, args[i]);
        strcat(dest, " ");
    }

    return dest;
}

void err_log_add_msg(char *msg)
{
    char line[LINE_BUFSIZE] = "";

    msg[0] = toupper(msg[0]);

    char *filename = "/var/log/shell/sistema_error.log";
    FILE *err_file = fopen(filename, "a");

    if (!err_file)
    {
        printf("Error opening error file\n");
        return;
    }

    err_put_date(err_file);
    fprintf(err_file, "%s: %s\n", compose_line(line), msg);
    
    fclose(err_file);
}

void err_print(char *msg)
{
    // Lo primero que se hace es revisar errno, ya que cualquier funcion de libreria
    // puede modificar errno y cambiar su valor actual
    char err[MSG_LENGTH];
    char line[LINE_BUFSIZE] = "";

    get_err_msg(err);
    if (msg != NULL && msg[0] != '\0')
    {
        printf("%s: ", msg);
    }
    printf("%s\n", err);

    FILE *err_file = fopen(err_filename, "a");

    if (!err_file)
    {
        printf("Error opening error file\n");
        return;
    }
    
    err_put_date(err_file);
    fprintf(err_file, "%s: %s\n", compose_line(line), err);
    
    fclose(err_file);
}