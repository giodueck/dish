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

#endif // DISH_H