#!/bin/bash

# Dependencias
name=libgcrypt
if [ ! -f "/usr/lib/$name.so" ]; then
	echo "$name no se encuentra instalado en /usr/lib"
	exit
fi
if [ ! -d "/var/log/shell" ]; then
	mkdir -v /var/log/shell
	chmod +w /var/log/shell
fi
