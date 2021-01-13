# DISH: Giovanni Dueck's Interactive Shell
Desarrollo de shell - Sistemas Operativos 1

# Version 1.0
Changelog:\
    - comando "contrasena":\
        Llama a passwd a traves de sys para dar una contrasena al usuario\
        en cuestion. No implemento una funcion propia porque ya no me da\
        el cuero; de todas formas el codigo tentativo de una funcion propia\
        se encuentra comentado.\
    - comando "uinfo":\
        El archivo en el que se guarda uinfo ahora se cambia para que el\
        usuario cuya informacion guarda sea el propietario. Esto posibilita\
        que el usuario modifique su propia uinfo.\
    - comando "usuario":\
        El directorio home cambia de propietario despues de crearse para que\
        el nuevo propietario sea el usuario en cuestion.\
    - varios cambios para que dish funcione en un sistema LFS, para cualquier\
        usuario.

# Instalacion
1. Ejecuta el script check.sh para revisar que esta lo que se necesita
2. Ejecuta el script install.sh para compilar e instalar dish
3. El path de instalacion debe ser agregado al PATH, este es /programs/bin

# Manual de uso
El comando "ayuda" lista todos los comandos disponibles. Para leer el manual\
de uso de cada comando se puede ingresar "[comando] -h" o "[comando] --ayuda"\
para imprimir una ayuda. Para ejecutar comandos de bash, se ingresa "sys\
[comando, opciones, etc.]".