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
#define OP_AND 1
#define OP_OR 2

#define FUNCTION_STARTS_WITH 0
#define FUNCTION_ENDS_WITH 1
#define FUNCTION_MATCH_AT 2
#define FUNCTION_EQUALS 3
#define FUNCTION_CONTAINS 4

static int compile_function(struct _generate *generate, struct _tokens *tokens, int function, int not)
{
  int token_type;
  int error = 0;
  int index = 0;

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
  if (token_type != TOKEN_STRING) { return 1; }

  switch(function)
  {
    case FUNCTION_STARTS_WITH:
      if (generate_starts_with(generate, tokens->next, not) != 0) { return 1; }
      break;
    case FUNCTION_ENDS_WITH:
      if (generate_ends_with(generate, tokens->next, not) != 0) { return 1; }
      break;
    case FUNCTION_MATCH_AT:
      if (generate_match_at(generate, tokens->next, index, not) != 0) { return 1; }
      break;
    case FUNCTION_EQUALS:
      if (generate_equals(generate, tokens->next, not) != 0) { return 1; }
      break;
    case FUNCTION_CONTAINS:
      if (generate_contains(generate, tokens->next, not) != 0) { return 1; }
      break;
    default:
      return 1;
  }

  token_type = tokens_next(tokens);
  if (token_type != TOKEN_PAREN_CLOSE) { return 1; }

  generate->reg++;

  return error;
}

static int compiler_evaluate(struct _generate *generate, struct _tokens *tokens, int precedent)
{
  int token_type;
  int error = 0;
  int not = 0;
  int opstack[2];
  int opstack_ptr = 0;
  //int command_count = 0;
  int pieces = 0;

  while(1)
  {
    token_type = tokens_next(tokens);
    if (token_type == TOKEN_EOF) { break; }

    if (strcmp(tokens->next, "not") == 0)
    {
      not = 1;
    }
      else
    if (strcmp(tokens->next, "starts_with") == 0)
    {
      if ((pieces & 1) != 0) { error = 1; break; }
      error = compile_function(generate, tokens, FUNCTION_STARTS_WITH, not);
      pieces++;
      not = 0;
    }
      else
    if (strcmp(tokens->next, "ends_with") == 0)
    {
      if ((pieces & 1) != 0) { error = 1; break; }
      error = compile_function(generate, tokens, FUNCTION_ENDS_WITH, not);
      pieces++;
      not = 0;
    }
      else
    if (strcmp(tokens->next, "match_at") == 0)
    {
      if ((pieces & 1) != 0) { error = 1; break; }
      error = compile_function(generate, tokens, FUNCTION_MATCH_AT, not);
      pieces++;
      not = 0;
    }
      else
    if (strcmp(tokens->next, "equals") == 0)
    {
      if ((pieces & 1) != 0) { error = 1; break; }
      error = compile_function(generate, tokens, FUNCTION_EQUALS, not);
      pieces++;
      not = 0;
    }
      else
    if (strcmp(tokens->next, "contains") == 0)
    {
      if ((pieces & 1) != 0) { error = 1; break; }
      error = compile_function(generate, tokens, FUNCTION_CONTAINS, not);
      pieces++;
      not = 0;
    }
      else
    if (strcmp(tokens->next, "and") == 0)
    {
      if ((pieces & 1) != 1) { error = 1; break; }
      opstack[opstack_ptr++] = OP_AND;
#if 0
      if (command_count == 0)
      {
        error = 1;
        break;
      }
#endif
      pieces++;
    }
      else
    if (strcmp(tokens->next, "or") == 0)
    {
      if ((pieces & 1) != 1) { error = 1; break; }
      opstack[opstack_ptr++] = OP_OR;
#if 0
      if (command_count == 0)
      {
        error = 1;
        break;
      }
#endif
      pieces++;
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
      if (opstack[opstack_ptr - 1] == OP_OR) { generate_or(generate); }
      else if (opstack[opstack_ptr - 1] == OP_AND) { generate_and(generate); }

      opstack_ptr--;
    }
  }

  return error == 0 ? 0: -1;
}

match_t compiler_generate(char *code)
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

  generate_init(&generate, (uint8_t *)match);
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

  if (compiler_evaluate(&generate, &tokens, 0) == -1) { error = 1; }

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

