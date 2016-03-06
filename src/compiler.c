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
#include <string.h>
#include <sys/mman.h>

#include "compiler.h"
#include "generate.h"
#include "tokens.h"

#define MAX_CODE_SIZE 4096

#define GENERATE(command) \
  token_type = tokens_next(&tokens); \
  if (token_type != TOKEN_PAREN_OPEN) \
  { \
    error = 1; \
    break; \
  } \
 \
  token_type = tokens_next(&tokens); \
  if (generate_##command(&generate, tokens.next) != 0) \
  { \
    error = 1; \
    break; \
  } \
 \
  token_type = tokens_next(&tokens); \
  if (token_type != TOKEN_PAREN_CLOSE) \
  { \
    error = 1; \
    break; \
  } \
 \
  if (not == 1) \
  { \
    if (generate_not(&generate) != 0) \
    { \
      error = 1; \
      break; \
    } \
    not = 0; \
  }

match_t compiler_generate(char *code)
{
  match_t match;
  struct _tokens tokens;
  struct _generate generate;
  int token_type;
  int error = 0;
  int not = 0;

  memset(&generate, 0, sizeof(generate));

  match = mmap(NULL, MAX_CODE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

  generate_init(&generate, (uint8_t *)match);
  tokens_init(&tokens, code);

  while(1)
  {
    token_type = tokens_next(&tokens);
    if (token_type == TOKEN_EOF) { break; }

    if (strcmp(tokens.next, "startswith") == 0) { generate.startswith++; }
    else if (strcmp(tokens.next, "endswith") == 0) { generate.endswith++; }
    else if (strcmp(tokens.next, "equals") == 0) { generate.equals++; }
    else if (strcmp(tokens.next, "contains") == 0) { generate.contains++; }
    else if (strcmp(tokens.next, "and") == 0) { generate.and++; }
    else if (strcmp(tokens.next, "or") == 0) { generate.or++; }
  }

  tokens_reset(&tokens);

#if 0
  printf("startswith: %d\n", generate.startswith);
  printf("  endswith: %d\n", generate.endswith);
  printf("    equals: %d\n", generate.equals);
  printf("  contains: %d\n", generate.contains);
  printf("       and: %d\n", generate.and);
  printf("        or: %d\n", generate.or);
#endif

  while(1)
  {
    token_type = tokens_next(&tokens);
    if (token_type == TOKEN_EOF) { break; }

    if (strcmp(tokens.next, "not") == 0)
    {
      not = 1;
    }
      else
    if (strcmp(tokens.next, "startswith") == 0)
    {
      GENERATE(startswith);
    }
      else
    if (strcmp(tokens.next, "endswith") == 0)
    {
      GENERATE(endswith);
    }
      else
    if (strcmp(tokens.next, "equals") == 0)
    {
      GENERATE(equals);
    }
      else
    if (strcmp(tokens.next, "contains") == 0)
    {
      GENERATE(contains);
    }
      else
    {
      error = 1;
      break;
    }

    //printf("%d) %s\n", token_type, tokens.next);
    if (token_type == TOKEN_ERROR) { break; }
  }

  generate_finish(&generate);
  tokens_free(&tokens);

  if (error == 1)
  {
    compiler_free(match);
    match = NULL;
  }

  return match;
}

void compiler_free(match_t match)
{
  munmap(match, MAX_CODE_SIZE);
}

