/*
    Funciones relacionadas a errores
*/

#ifndef ERR_H
#define ERR_H

// Agrega el ultimo error a un registro de errores usando errno para el mensaje
void err_log_add_errno(char **args);

// Agrega el ultimo error a un registro de errores con un mensaje
void err_log_add_with_msg(char **args, char *msg);

// Agrega un mensaje al registro de errores
void err_log_add_msg(char *msg);

// Imprime el error apropiado dependiendo de errno y agrega al registro
void err_print(char *msg);

#endif // ERR_H