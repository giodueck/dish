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
int dish_history(char **args);
int dish_useradd(char **args);  // missing help
int dish_passwd(char **args);   // missing help
int dish_cp(char **args);
int dish_mv(char **args);       // missing help
int dish_rn(char **args);       // missing help
int dish_mkdir(char **args);    // missing help
int dish_rm(char **args);       // missing help
int dish_rmdir(char **args);    // missing help

// Arrays de nombres de builtins con sus respectivas funciones
#define NUM_BUILTINS 13

char *builtin_str[NUM_BUILTINS];

int (*builtin_func[NUM_BUILTINS]) (char**);

#endif // BUILTIN_H