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

#include "tokens.h"

void tokens_init(struct _tokens *tokens, char *code)
{
  tokens->code = code;
  tokens->next = malloc(strlen(code) + 1);
  //tokens->ptr = 0;
  //tokens->old = 0;
}

int tokens_next(struct _tokens *tokens)
{
  int token_type = TOKEN_NOTHING;
  char *code = tokens->code;
  int ptr = 0;

  if (*code == 0) { return TOKEN_EOF; }

  do
  {
    // Remove whitespace
    while(1)
    {
      if (*code != ' ') { break; }
      code++;
    }

    // Check for parenthesis
    if (*code == '(')
    {
      tokens->next[ptr++] = '(';
      token_type = TOKEN_PAREN_OPEN;
      code++; 
      break;
    }

    if (*code == ')')
    {
      tokens->next[ptr++] = ')';
      token_type = TOKEN_PAREN_CLOSE;
      code++;
      break;
    }

    if (*code == '\'')
    {
      token_type = TOKEN_STRING;
      code++;
    }
      else
    {
      token_type = TOKEN_KEYWORD;
    }

    while(1)
    {
      if (token_type == TOKEN_KEYWORD)
      {
        if (!((*code >= 'a' && *code <= 'z') ||
             (*code >= 'A' && *code <= 'Z')))
        {
          break;
        }
      }

      if (*code == '\'' && token_type == TOKEN_STRING)
      {
        code++;
        break;
      }

      if (*code == 0 && token_type == TOKEN_STRING)
      {
        return TOKEN_ERROR;
      }

      tokens->next[ptr++] = *code;

      code++;
    }
 
  } while(0);

  tokens->next[ptr] = 0;
  tokens->code = code;

  return token_type;
}

void tokens_free(struct _tokens *tokens)
{
  free(tokens->next);
}

