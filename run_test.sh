#!/usr/bin/env bash

./compare_starts_with src/generate_x86_64.c "starts_with('int ')" "^int " "int "
./compare_ends_with src/generate_x86_64.c "ends_with(');')" "\);" ");"

