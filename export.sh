#!/bin/bash

echo "Bootstrapping Export"

cc -o exportC -IIcarianEngine/deps/CUBE/include/ -O3 export.c
if [ $? -ne 0 ]; then
    echo "Export bootstrap failed"
    exit 1
fi

echo "Exporting"

./exportC "$@"