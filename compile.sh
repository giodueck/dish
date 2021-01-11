#!/bin/bash

# Script to facilitate manual compiling of sources

gcc main.c dish.c builtins.c err.c -o dish -lcrypt
