#!/bin/bash

# Dependencias
name=libgcrypt
if [ ! -f "/usr/lib/$name.so" ]; then
	echo "$name no se encuentra instalado en /usr/lib"
	exit
fi
