/*
    Funciones relacionadas a errores
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "defines.h"
#include "err.h"

extern char *err_filename;

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
        sprintf(str, "Directorio no existe.");
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
        sprintf(str, "Funcion no soprtada por el sistema.");
        return str;
    case EEXIST:
        sprintf(str, "Archivo ya existe.");
        return str;
    case EROFS:
        sprintf(str, "Directorio de solo-lectura.");
        return str;
    case ENOTEMPTY:
        sprintf(str, "Directorio no vacio.");
        return str;

    default:
        return NULL;
    }
}

void err_log_add_errno(char **args)
{
    return;
}

void err_log_add_with_msg(char **args, char *msg)
{
    return;
}

void err_log_add_msg(char *msg)
{
    return;
}

void err_print(char *msg)
{
    // Lo primero que se hace es revisar errno, ya que cualquier funcion de libreria
    // puede modificar errno
    char err[MSG_LENGTH];
    get_err_msg(err);

    if (msg != NULL && msg[0] != '\0')
    {
        printf("%s: ", msg);
    }

    printf("%s\n", err);
}