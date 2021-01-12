# DISH: Giovanni Dueck's Interactive Shell
Desarrollo de shell - Sistemas Operativos 1

# Version 0.9
Changelog:\
    - comando "uinfo":\
        Informacion sobre otros usuarios puede agregarse usando\
        "uinfo [usuario]". Limitado al usuario root.\
        Usando la opcion -s se puede mostrar la informacion actual\
        sin modificarla.\
    - errores de sistema ahora se registran en /var/log/shell/sistema_error.log\
    - inicios y cierres de sesion ahora se registran en /var/log/shell/{usuario}_horarios_log
    