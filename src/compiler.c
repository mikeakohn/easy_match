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

match_t compiler_generate(char *code)
{
  match_t match;
  struct _tokens tokens;
  struct _generate *generate;
  int token_type;

  match = mmap(NULL, MAX_CODE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

  tokens_init(&tokens, code);

  while(1)
  {
    token_type = tokens_next(&tokens);
    if (token_type == TOKEN_EOF) { break; }

    printf("%d) %s\n", token_type, tokens.next);
    if (token_type == TOKEN_ERROR) { break; }
  }

  tokens_free(&tokens);

  return NULL;
}

void compiler_free(match_t function)
{
  munmap(function, MAX_CODE_SIZE);
}

