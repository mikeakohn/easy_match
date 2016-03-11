/**
 *  Easy Match
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPL
 *
 * Copyright 2016 by Michael Kohn
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"

int main(int argc, char *argv[])
{
  match_t match;
  FILE *in;
  char line[4096];
  int ptr, ch;

  if (argc != 3)
  {
    printf("Usage: test <file> <string>\n");
    exit(0);
  }

  match = compiler_generate(argv[2], OPTION_NONE);

  if (match == NULL)
  {
    printf("Error compiling.\n");
    return 0;
  }

  in = fopen(argv[1], "rb");
  if (in == NULL)
  {
    printf("Couldn't open file %s\n", argv[1]);
    compiler_free(match);
    exit(0); 
  }

  ptr = 0;
  while(1)
  {
    ch = getc(in);
    if (ch == '\r') { continue; }
    if (ch == '\n' || ch == EOF)
    {
      line[ptr++] = 0;
      int result = match(line);

      //if (result != 0) { printf("%s\n", line); }
      printf("%d) %s\n", result, line);

      if (ch == EOF) { break; }
      ptr = 0;
      continue;
    }

    line[ptr++] = ch;
    if (ptr == 4094) { printf("Error: Line overflow\n"); break; }
  }

  compiler_free(match);

  return 0;
}

