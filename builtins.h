/*
    Comandos integrados o "built-in" de DISH
*/

#ifndef BUILTIN_H
#define BUILTIN_H

// Arrays de nombres de builtins con sus respectivas funciones
char *builtin_str[];
int (*builtin_func[]) (char**);

// Retorna el numero de funciones builtin
int num_builtins();

// Prototipos de funciones
int dish_cd(char **args);
int dish_help(char **args);
int dish_exit(char **args);

#endif // BUILTIN_H