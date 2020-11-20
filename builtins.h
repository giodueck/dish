/*
    Comandos integrados o "built-in" de DISH
*/

#ifndef BUILTIN_H
#define BUILTIN_H

// Prototipos de funciones
int dish_cd(char **args);
int dish_help(char **args);
int dish_exit(char **args);
int dish_sys(char **args);

// Arrays de nombres de builtins con sus respectivas funciones
#define NUM_BUILTINS 4

char *builtin_str[NUM_BUILTINS];

int (*builtin_func[NUM_BUILTINS]) (char**);

#endif // BUILTIN_H