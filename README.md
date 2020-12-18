# DISH: Giovanni Dueck's Interactive Shell
Desarrollo de shell - Sistemas Operativos 1

# Version 0.6
Changelog:\
    - comando "creardir" agregado:\
        Crea un directorio con el nombre dado.\
    - comando "removerdir" agregado"\
        Elimina un directorio, pero solo si es un directorio vacio.\
    - comando "remover":\
        Opcion -r removida, complejidad innecesaria.\
    - errores ahora son mayormente controlados por funciones dedicadas,\
      asi tambien la impresion de errores en pantalla.\
    - registro de errores ocurre en el archivo .dish_err ubicado en el\
      directorio home.\
    - registro de historial ahora ocurre en el archivo .dish_log ubicado\
      en el directorio home.\
    - comando "historial":\
        Opcion -r recrea el archivo historial.\
    - comando "listar":\
        Opcion -l removida, listar ahora siempre imprime en forma de lista.\
        Ademas, la impresion se hace en orden segun nombres de archivo.\
    - comando "usuario" agregado:\
        Crea un nuevo usuario con la opcion de crear un grupo de nombre\
        diferente al del usuario.
