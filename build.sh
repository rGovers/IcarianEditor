#!/bin/bash

# cc -o buildC -IIcarianEngine/deps/CUBE/include/ build.c
cc -o buildC -IIcarianEngine/deps/CUBE/include/ build.c -g -fsanitize=address
if [ $? -ne 0 ]; then
    echo "Build bootstrap failed"
    exit 1
fi

./buildC "$@"