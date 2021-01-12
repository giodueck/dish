#!/bin/bash

# Compilacion
echo gcc main.c dish.c err.c builtins.c -o dish -lcrypt

# Carpetas de instalacion
if [ ! -d "/programs" ]; then
	mkdir -v /programs
fi
if [ ! -d "/programs/bin" ]; then
	mkdir -v /programs/bin
fi
if [ ! -d "/programs/dish-help" ]; then
	mkdir -v /programs/dish-help
fi

cp -v dish /programs/bin/dish
cp -v help/*.txt /programs/dish-help/

echo "Instalacion completa una vez que /programs/bin se agregue al PATH"
