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

#include <pcre.h>

#include "compiler.h"
#include "timer.h"

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
#ifdef CYCLES_COUNT 
  union _perftime perf_start;
  union _perftime perf_end;
#else
  struct timespec tp_start;
  struct timespec tp_stop;
#endif
  const char *regex_error;
  int regex_error_offset;
  int regex_substr_vec[30];
  pcre_extra *regex_extra;
  pcre *regex_compiled;
  char *regex;
  char *starts_with = NULL;
  char *ends_with = NULL;
  char *contains = NULL;
  char *equals = NULL;

  if (argc != 5)
  {
    printf("Usage: test <file> <easy check> <regex> <strncmp>\n");
    exit(0);
  }

  regex = argv[3];

  if (argv[4][0] == '^')
  {
    starts_with = argv[4] + 1;
  }
    else
  if (argv[4][0] == '@')
  {
    ends_with = argv[4] + 1;
  }
    else
  if (argv[4][0] == '+')
  {
    contains = argv[4] + 1;
  }
    else
  if (argv[4][0] == '=')
  {
    equals = argv[4] + 1;
  }

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

  printf("--- Easy Match Test --\n");
  printf("[ %s %s ]\n", argv[2], argv[3]);

  printf("Easy Match       ");

  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int result = match(buffer + lines[i]);
    if (result == 1) { count++; }
  }
  TIMER_STOP

  printf("Easy Match (len) ");

  TIMER_START
  for (i = 0; i < line_count; i++)
  {
    int result = match_with_len(buffer + lines[i], lines_len[i]);
    if (result == 1) { count++; }
  }
  TIMER_STOP

  printf("PCRE             ");

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

  if (starts_with != NULL)
  {
    printf("strncmp()        ");

    int len = strlen(starts_with);

    TIMER_START
    for (i = 0; i < line_count; i++)
    {
      if (strncmp(buffer + lines[i], starts_with, len) == 0) { count++; }
    }
    TIMER_STOP
  }
    else
  if (ends_with != NULL)
  {
    printf("strncmp()        ");

    int len = strlen(ends_with);

    TIMER_START
    for (i = 0; i < line_count; i++)
    {
      int index = strlen(buffer + lines[i]) - len;

      if (strcmp(buffer + lines[i] + index, ends_with) == 0) { count++; }
    }
    TIMER_STOP
  }
    else
  if (contains != NULL)
  {
    printf("strstr()         ");

    TIMER_START
    for (i = 0; i < line_count; i++)
    {
      if (strstr(buffer + lines[i], contains) != NULL) { count++; }
    }
    TIMER_STOP
  }
    else
  if (equals != NULL)
  {
    printf("strcmp()         ");

    TIMER_START
    for (i = 0; i < line_count; i++)
    {
      if (strcmp(buffer + lines[i], equals) == 0) { count++; }
    }
    TIMER_STOP
  }

  if (regex_extra != NULL) { pcre_free(regex_extra); }
  pcre_free(regex_compiled);

  compiler_free(match);
  compiler_free(match_with_len);

  free(buffer);
  free(lines);

  return 0;
}

