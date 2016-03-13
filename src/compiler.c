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
#define OP_NONE 0
#define OP_AND 1
#define OP_OR 2

#define FUNCTION_NONE 0
#define FUNCTION_STARTS_WITH 1
#define FUNCTION_ENDS_WITH 2
#define FUNCTION_MATCH_AT 3
#define FUNCTION_EQUALS 4
#define FUNCTION_CONTAINS 5

struct _marker
{
  int ptr;
  int reg;
};

static int compile_function(struct _generate *generate, struct _tokens *tokens, int function, int not)
{
  int token_type;
  int error = 0;
  int index = 0;
  int len;
  char *match;

  token_type = tokens_next(tokens);
  if (token_type != TOKEN_PAREN_OPEN) { return 1; }

  if (function == FUNCTION_MATCH_AT)
  {
    token_type = tokens_next(tokens);
    if (token_type != TOKEN_NUMBER) { return 1; }

    index = atoi(tokens->next);

    token_type = tokens_next(tokens);
    if (token_type != TOKEN_COMMA) { return 1; }
  }

  token_type = tokens_next(tokens);

  if (token_type == TOKEN_KEYWORD)
  {
    uint64_t value;
    int size;

    len = 0;

    if (strcmp(tokens->next, "int8") == 0) { size = 1; }
    else if (strcmp(tokens->next, "int16") == 0) { size = 2; }
    else if (strcmp(tokens->next, "int32") == 0) { size = 4; }
    else if (strcmp(tokens->next, "int64") == 0) { size = 8; }
    else { return -1; }

    token_type = tokens_next(tokens);
    if (token_type != TOKEN_PAREN_OPEN) { return 1; }

    match = alloca(4096);

    while(1)
    {
      int n;

      token_type = tokens_next(tokens);
      if (token_type == TOKEN_NUMBER)
      {
        value = strtoll(tokens->next, NULL, 10);
      }
        else
      if (token_type == TOKEN_HEX)
      {
        value = strtoll(tokens->next, NULL, 16);
      }
        else
      {
        return 1;
      }

      for (n = 0; n < size; n++)
      {
        if (len == 4096) { return 1; }

        match[len++] = value & 0xff;
        value = value >> 8;
      }

      if (value != 0) { return 1; }

      token_type = tokens_next(tokens);
      if (token_type == TOKEN_PAREN_CLOSE) { break; }
      if (token_type != TOKEN_COMMA) { return 1; }
    }
  }
    else
  if (token_type == TOKEN_STRING)
  {
    match = tokens->next;
    len = strlen(match);
  }
    else
  {
    return 1;
  }

  switch(function)
  {
    case FUNCTION_STARTS_WITH:
      if (generate_starts_with(generate, match, len, not) != 0) { return 1; }
      break;
    case FUNCTION_ENDS_WITH:
      if (generate_ends_with(generate, match, len, not) != 0) { return 1; }
      break;
    case FUNCTION_MATCH_AT:
      if (generate_match_at(generate, match, len, index, not) != 0) { return 1; }
      break;
    case FUNCTION_EQUALS:
      if (generate_equals(generate, match, len, not) != 0) { return 1; }
      break;
    case FUNCTION_CONTAINS:
      if (generate_contains(generate, match, len, not) != 0) { return 1; }
      break;
    default:
      return 1;
  }

  token_type = tokens_next(tokens);
  if (token_type != TOKEN_PAREN_CLOSE) { return 1; }

  generate->reg++;

  return error;
}

static int compiler_get_function(char *token)
{
  if (strcmp(token, "starts_with") == 0) { return FUNCTION_STARTS_WITH; }
  else if (strcmp(token, "ends_with") == 0) { return FUNCTION_ENDS_WITH; }
  else if (strcmp(token, "match_at") == 0) { return FUNCTION_MATCH_AT; }
  else if (strcmp(token, "equals") == 0) { return FUNCTION_EQUALS; }
  else if (strcmp(token, "contains") == 0) { return FUNCTION_CONTAINS; }

  return FUNCTION_NONE;
}

static int compiler_short_circuit(struct _generate *generate, struct _marker *markers, int marker_count, int count, int skip_value, int pop_to_reg)
{
  int n;

  marker_count--;

  for (n = 0; n < count; n++)
  {
    marker_count--;

#if 0
printf("short_circuit %d> ptr=%d-%d marker_count=%d reg=%d pop_to_reg=%d\n", n,
  markers[marker_count].ptr, generate->ptr, marker_count,
  markers[marker_count].reg, pop_to_reg);
#endif

    generate_skip(generate, markers[marker_count].ptr, generate->ptr, markers[marker_count].reg, skip_value, pop_to_reg);
  }

  return 0;
}

static int compiler_evaluate(struct _generate *generate, struct _tokens *tokens)
{
  int token_type;
  int error = 0;
  int not = 0;
  int opstack[2];
  int opstack_ptr = 0;
  int pieces = 0;
  int function;
  // For short circuiting (consecutive or's that evaluate true or and's false)
  struct _marker markers[128];
  int marker_count = 0;
  int marker_and_count = 0;
  //int marker_or_count = 0;
  //int curr_op = OP_NONE;

  while(1)
  {
    token_type = tokens_next(tokens);
    if (token_type == TOKEN_EOF) { break; }

    function = compiler_get_function(tokens->next);

    if (strcmp(tokens->next, "not") == 0)
    {
      not = 1;
      continue;
    }
      else
    if (function != FUNCTION_NONE)
    {
      if ((pieces & 1) != 0) { error = 1; break; }
      if (marker_count == 128) { error = 1; break; }
      markers[marker_count].reg = generate->reg;
      error = compile_function(generate, tokens, function, not);
      markers[marker_count].ptr = generate->ptr;
      marker_count++;
      pieces++;
      not = 0;
    }
      else
    if (strcmp(tokens->next, "and") == 0)
    {
      if ((pieces & 1) != 1) { error = 1; break; }
      opstack[opstack_ptr++] = OP_AND;
      marker_and_count++;
      pieces++;
    }
      else
    if (strcmp(tokens->next, "or") == 0)
    {
      if ((pieces & 1) != 1) { error = 1; break; }
      opstack[opstack_ptr++] = OP_OR;
      //marker_or_count++;
      pieces++;

      if (marker_and_count != 0)
      {
        compiler_short_circuit(generate, markers, marker_count, marker_and_count, 0, marker_and_count);
        marker_count -= marker_and_count;
      }
    }
      else
    {
      error = 1;
    }

    if (error == 1) { break; }

    if ((pieces & 1) == 1 && opstack_ptr != 0)
    {
      if (opstack[opstack_ptr - 1] == OP_AND)
      {
        generate_and(generate);
        opstack_ptr--;
      }
    }

    if ((pieces & 0) == 0 && opstack_ptr == 2)
    {
      if (opstack[opstack_ptr - 1] == OP_OR)
      {
        generate_or(generate);
        opstack_ptr--;
      }
    }

    //printf("%d) %s\n", token_type, tokens.next);
    if (token_type == TOKEN_ERROR) { error = 1; break; }
  }

  if (error == 0)
  {
    while (opstack_ptr > 0)
    {
      if (opstack[opstack_ptr - 1] == OP_OR) { break; }
      else if (opstack[opstack_ptr - 1] == OP_AND) { generate_and(generate); }

      opstack_ptr--;
    }

    if (marker_and_count != 0)
    {
      compiler_short_circuit(generate, markers, marker_count, marker_and_count, 0, marker_and_count);
      marker_count -= marker_and_count;
    }

    while (opstack_ptr > 0)
    {
      if (opstack[opstack_ptr - 1] == OP_OR) { generate_or(generate); }
      else if (opstack[opstack_ptr - 1] == OP_AND) { error = 1; break; }

      opstack_ptr--;
    }

    if (marker_count != 0)
    {
      compiler_short_circuit(generate, markers, marker_count, marker_count - 1, 1, 0);
    }
  }

  return error == 0 ? 0: -1;
}

void *compiler_generate(char *code, int option)
{
  match_t match;
  struct _tokens tokens;
  struct _generate generate;
  int token_type;
  int error = 0;

  memset(&generate, 0, sizeof(generate));

  match = mmap(NULL, MAX_CODE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (match == MAP_FAILED)
  {
    perror("mmap() failed");
    return NULL;
  }

  generate.code = (uint8_t *)match;

  generate_init(&generate, (uint8_t *)match, option);
  tokens_init(&tokens, code);

  while(1)
  {
    token_type = tokens_next(&tokens);
    if (token_type == TOKEN_EOF) { break; }

    if (strcmp(tokens.next, "starts_with") == 0) { generate.starts_with++; }
    else if (strcmp(tokens.next, "ends_with") == 0) { generate.ends_with++; }
    else if (strcmp(tokens.next, "match_at") == 0) { generate.match_at++; }
    else if (strcmp(tokens.next, "equals") == 0) { generate.equals++; }
    else if (strcmp(tokens.next, "contains") == 0) { generate.contains++; }
    else if (strcmp(tokens.next, "and") == 0) { generate.and++; }
    else if (strcmp(tokens.next, "or") == 0) { generate.or++; }
  }

  tokens_reset(&tokens);

#if 0
  printf("starts_with: %d\n", generate.starts_with);
  printf("  ends_with: %d\n", generate.ends_with);
  printf("  match_at: %d\n", generate.match_at);
  printf("    equals: %d\n", generate.equals);
  printf("  contains: %d\n", generate.contains);
  printf("       and: %d\n", generate.and);
  printf("        or: %d\n", generate.or);
#endif

  if (compiler_evaluate(&generate, &tokens) == -1) { error = 1; }

  generate_finish(&generate);
  tokens_free(&tokens);

  if (error == 1)
  {
    compiler_free(match);
    match = NULL;
  }

//#define DUMP_CODE
#ifdef DUMP_CODE
  FILE *out = fopen("/tmp/debug.bin", "wb");
  fwrite(generate.code, 1, generate.ptr, out);
  fclose(out);
#endif

  return match;
}

void compiler_free(void *match)
{
  munmap(match, MAX_CODE_SIZE);
}

