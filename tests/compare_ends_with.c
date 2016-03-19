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
#include <time.h>

#include <pcre.h>

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

#define CYCLES_START \
  asm __volatile__ \
  ( \
    "rdtsc" : "=a" (perf_start.split.lo), "=d" (perf_start.split.hi) \
  );

#define CYCLES_STOP \
  asm __volatile__ \
  ( \
    "rdtsc" : "=a" (perf_end.split.lo), "=d" (perf_end.split.hi) \
  );

#define TIMER_START \
  clock_gettime(CLOCK_MONOTONIC, &tp_start);

#define TIMER_STOP \
  clock_gettime(CLOCK_MONOTONIC, &tp_stop);

double diff_time(struct timespec *tp_start, struct timespec *tp_stop)
{
  long nsec = tp_stop->tv_nsec - tp_start->tv_nsec;
  long sec = tp_stop->tv_sec - tp_start->tv_sec;

  if (nsec < 0) { sec--; nsec += 1000000000; }

  double t = (sec * 1000) + ((double)nsec / 1000000);

  return t;
}

int main(int argc, char *argv[])
{
  match_t match;
  match_with_len_t match_with_len;
  FILE *in;
  int *lines;
  int *lines_len;
  int ptr, ch;
  int line_count;
  char *buffer;
  int buffer_len;
  int line_start;
  int count;
  int i;
#if 0
  union _perftime perf_start;
  union _perftime perf_end;
#endif
  struct timespec tp_start;
  struct timespec tp_stop;
  const char *regex_error;
  int regex_error_offset;
  int regex_substr_vec[30];
  pcre_extra *regex_extra;
  pcre *regex_compiled;
  char *regex;
  char *startswith;

  if (argc != 5)
  {
    printf("Usage: test <file> <easy check> <regex> <strncmp>\n");
    exit(0);
  }

  regex = argv[3];
  startswith = argv[4];

  int where;
  int status = pcre_config(PCRE_CONFIG_JIT, &where);

  printf("config pcre_jit: status=%d where=%d\n", status, where);

  regex_compiled = pcre_compile(regex, 0, &regex_error, &regex_error_offset, NULL);

  if (regex_compiled == NULL)
  {
    printf("Error compiling regex\n");
    exit(1);
  }

  //regex_extra = pcre_study(regex_compiled, 0, &regex_error);
  regex_extra = pcre_study(regex_compiled, PCRE_STUDY_JIT_COMPILE, &regex_error);

  match = compiler_generate(argv[2], OPTION_NONE);

  if (match == NULL)
  {
    printf("Error compiling.\n");
    return 0;
  }

  match_with_len = compiler_generate(argv[2], OPTION_WITH_LEN);

  if (match_with_len == NULL)
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

  buffer_len = ftell(in) + 1;
  buffer = (char *)malloc(buffer_len);
  lines = (int *)malloc(line_count * sizeof(int));
  lines_len = (int *)malloc(line_count * sizeof(int));
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
      lines[line_count] = line_start;
      lines_len[line_count] = ptr - line_start;
      line_count++;
      buffer[ptr++] = 0;

      if (ch == EOF) { break; }
      line_start = ptr;

      continue;
    }

    buffer[ptr++] = ch;
  }

  printf("Easy Match\n");

  count = 0;
  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int result = match(buffer + lines[i]);
    if (result == 1) { count++; }
  }
  TIMER_STOP
  //printf("count=%d cpu=%ld\n", count, perf_end.count - perf_start.count);
  printf("count=%d msec=%f\n", count, diff_time(&tp_start, &tp_stop));

  printf("Easy Match (with len)\n");

  count = 0;
  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int result = match_with_len(buffer + lines[i], lines_len[i]);
    if (result == 1) { count++; }
  }
  TIMER_STOP
  //printf("count=%d cpu=%ld\n", count, perf_end.count - perf_start.count);
  printf("count=%d msec=%f\n", count, diff_time(&tp_start, &tp_stop));

  printf("strncmp()\n");

  int len = strlen(startswith);

  count = 0;
  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int index = strlen(buffer + lines[i]) - len;

    if (strcmp(buffer + lines[i] + index, startswith) == 0) { count++; }
  }
  TIMER_STOP
  //printf("count=%d cpu=%ld\n", count, perf_end.count - perf_start.count);
  printf("count=%d msec=%f\n", count, diff_time(&tp_start, &tp_stop));

  printf("PCRE\n");

  count = 0;
  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int regex_ret = pcre_exec(regex_compiled,
                              regex_extra,
                              buffer + lines[i],
                              strlen(buffer + lines[i]), // length of string
                              0,                 // Start looking at this point
                              0,                 // OPTIONS
                              regex_substr_vec,
                              30);               // Length of subStrVec

    if (regex_ret != PCRE_ERROR_NOMATCH) { count++; }
  }
  TIMER_STOP
  //printf("count=%d cpu=%ld\n", count, perf_end.count - perf_start.count);
  printf("count=%d msec=%f\n", count, diff_time(&tp_start, &tp_stop));

  if (regex_extra != NULL) { pcre_free(regex_extra); }
  pcre_free(regex_compiled);

  compiler_free(match);
  compiler_free(match_with_len);

  free(buffer);
  free(lines);

  return 0;
}

