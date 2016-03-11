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

#ifndef _TOKENS_H
#define _TOKENS_H

#define TOKEN_NOTHING -2
#define TOKEN_ERROR -1
#define TOKEN_EOF 0
#define TOKEN_KEYWORD 1
#define TOKEN_PAREN_OPEN 2
#define TOKEN_PAREN_CLOSE 3
#define TOKEN_STRING 4
#define TOKEN_COMMA 5
#define TOKEN_NUMBER 6
#define TOKEN_HEX 7

struct _tokens
{
  char *code;
  char *next;
  char *start;
  int len;
};

void tokens_init(struct _tokens *tokens, char *code);
int tokens_next(struct _tokens *tokens);
void tokens_reset(struct _tokens *tokens);
void tokens_free(struct _tokens *tokens);

#endif

