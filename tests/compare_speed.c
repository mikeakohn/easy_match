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
#include <stdint.h>
#include <string.h>

#include "compiler.h"

union _perftime
{
  struct _split
  {
    uint32_t lo;
    uint32_t hi;
  } split;
  uint64_t count;
};

#define TIMER_START \
  asm __volatile__ \
  ( \
    "rdtsc" : "=a" (perf_start.split.lo), "=d" (perf_start.split.hi) \
  );

#define TIMER_STOP \
  asm __volatile__ \
  ( \
    "rdtsc" : "=a" (perf_end.split.lo), "=d" (perf_end.split.hi) \
  );

int main(int argc, char *argv[])
{
  match_t match;
  FILE *in;
  int *lines;
  int ptr, ch;
  int line_count;
  char *buffer;
  int buffer_len;
  int line_start;
  int count;
  int i;
  union _perftime perf_start;
  union _perftime perf_end;

  if (argc != 3)
  {
    printf("Usage: test <file> <string>\n");
    exit(0);
  }

  match = compiler_generate(argv[2]);

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

  line_count = 0;

  while(1)
  {
    ch = getc(in);
    if (ch == '\n' || ch == EOF) { line_count++; }
    if (ch == EOF) { break; }
  }

  buffer_len = ftell(in);
  buffer = (char *)malloc(buffer_len);
  lines = (int *)malloc(line_count * sizeof(int));
  fseek(in, 0, SEEK_SET);
  line_count = 0;

  ptr = 0;
  line_start = ptr;

  while(1)
  {
    ch = getc(in);
    if (ch == '\r') { continue; }
    if (ch == '\n' || ch == EOF)
    {
      buffer[ptr++] = 0;
      lines[line_count++] = line_start;

      if (ch == EOF) { break; }
      line_start = ptr;

      continue;
    }

    buffer[ptr++] = ch;
  }

  count = 0;
  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int result = match(buffer + lines[i]);
    if (result == 1) { count++; }
  }
  TIMER_STOP
  printf("count=%d cpu=%ld\n", count, perf_end.count - perf_start.count);

  int len = strlen("int ");
  count = 0;
  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    if (strncmp(buffer + lines[i], "int ", len) == 0) { count++; }
  }
  TIMER_STOP
  printf("count=%d cpu=%ld\n", count, perf_end.count - perf_start.count);

  compiler_free(match);

  free(buffer);
  free(lines);

  return 0;
}

