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
// Retorna una cadena con valor asignado por malloc, debe ser liberada.
// Retorna NULL en caso de fallo
char *get_err_msg()
{
    char *str = malloc(sizeof(char) * MSG_LENGTH);

    switch (errno)
    {
    case EACCES:
        sprintf(str, "Acceso denegado.\n");
        return str;
    case ENOENT:
        sprintf(str, "Directorio no existe.\n");
        return str;
    case ENOTDIR:
        sprintf(str, "No es directorio.\n");
        return str;

    default:
        free(str);
        return NULL;
    }
}

void err_log_add_errno(char **args)
{

}

void err_log_add_msg(char **args, char *msg)
{

}

void err_print(char *msg)
{
    char *err = get_err_msg();

    if (msg != NULL && msg[0] != '\0')
    {
        printf("%s: ", msg);
    }

    prtinf("%s\n", err);

    free(err);
}