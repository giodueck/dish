/*
    Funciones basicas de DISH
*/

#ifndef DISH_H
#define DISH_H

// Lee una linea de stdin hasta encontrar EOF o '\n'
char *dish_read_line();

// Divide la cadena dada en tokens
char **dish_split_line(char *line);

// Ejecuta el comando dado por args
int dish_execute(char **args);

// Imprime un mensaje de error en caso de que el comando dado no existe
int dish_command_not_found(char **args);
#endif // DISH_H