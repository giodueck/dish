/*
    Comandos integrados o "built-in" de DISH
*/

#ifndef BUILTIN_H
#define BUILTIN_H

// Prototipos de funciones
int dish_cd(char **args);
int dish_help(char **args);
int dish_exit(char **args);

// Arrays de nombres de builtins con sus respectivas funciones
char *builtin_str[] = 
{
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char**) =
{
    &dish_cd,
    &dish_help,
    &dish_exit
};

// Retorna el numero de funciones builtin
int num_builtins();

#endif // BUILTIN_H