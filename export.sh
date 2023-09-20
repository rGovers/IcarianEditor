#!/bin/bash

cc -o exportC -IIcarianEngine/deps/CUBE/include/ export.c
if [ $? -ne 0 ]; then
    echo "Build bootstrap failed"
    exit 1
fi

./exportC "$@"