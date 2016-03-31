#!/usr/bin/env bash

textfile='src/generate_x86_64.c'

./compare_speed ${textfile} "starts_with('int ')" "^int " "^int "
./compare_speed ${textfile} "ends_with(');')" "\);$" "@);"
./compare_speed ${textfile} "starts_with('int ') or ends_with(');')" "^int|\);$" ""
./compare_speed ${textfile} "contains('int')" "int" "+int"
./compare_speed ${textfile} "equals('  return 0;')" "^  return 0;$" "=  return 0;"
./compare_speed ${textfile} "equals('  return 0')" "^  return 0$" "=  return 0"

