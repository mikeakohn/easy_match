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
  tokens->start = code;
  tokens->next = malloc(strlen(code) + 1);
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

    // Check for comma
    if (*code == ',')
    {
      tokens->next[ptr++] = ',';
      token_type = TOKEN_COMMA;
      code++; 
      break;
    }

    // If there is no parenthesis then this must be either a quote or
    // keyword (or possibly an illegal character).
    if (*code == '\'')
    {
      token_type = TOKEN_STRING;
      code++;
    }
      else
    if (*code >= '0' && *code <= '9')
    {
      token_type = TOKEN_NUMBER;
    }
      else
    {
      token_type = TOKEN_KEYWORD;
    }

    while(1)
    {
      // This is a keyword and the next character is not a letter so stop.
      if (token_type == TOKEN_KEYWORD)
      {
        if (!((*code >= 'a' && *code <= 'z') ||
             (*code >= 'A' && *code <= 'Z') ||
             *code == '_'))
        {
          break;
        }
      }

      // This is a number and the next character is not a number so stop.
      if (token_type == TOKEN_NUMBER)
      {
        if (!(*code >= '0' && *code <= '9'))
        {
          break;
        }
      }

      // This is a string and there is a closing quote, so stop.
      if (*code == '\'' && token_type == TOKEN_STRING)
      {
        code++;
        break;
      }

      // Did't get a closing quote so throw an error.
      if (*code == 0 && token_type == TOKEN_STRING)
      {
        tokens->next[0] = 0;
        return TOKEN_ERROR;
      }

      tokens->next[ptr++] = *code;

      code++;
    }
 
  } while(0);

  // The length of the token is 0 which means an illegal character was
  // was found.  Throw an error.
  if (ptr == 0)
  {
    tokens->next[0] = 0;
    return TOKEN_ERROR;
  }

  tokens->next[ptr] = 0;
  tokens->code = code;

  return token_type;
}

void tokens_reset(struct _tokens *tokens)
{
  tokens->code = tokens->start;
}

void tokens_free(struct _tokens *tokens)
{
  free(tokens->next);
}

