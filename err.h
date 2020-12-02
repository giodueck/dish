/*
    Funciones relacionadas a errores
*/

#ifndef ERR_H
#define ERR_H

// Agrega un mensaje al registro de errores
void err_log_add_msg(char *msg);

// Imprime el error apropiado dependiendo de errno y agrega al registro
void err_print(char *msg);

#endif // ERR_H