#!/bin/bash

echo "Bootstrapping Build"

cc -o buildC -IIcarianEngine/deps/CUBE/include/ build.c -O3
# cc -o buildC -IIcarianEngine/deps/CUBE/include/ build.c -g -fsanitize=address
if [ $? -ne 0 ]; then
    echo "Build bootstrap failed"
    exit 1
fi

echo "Building"

./buildC "$@"