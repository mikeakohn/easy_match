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

#include "tokens.h"

int main(int argc, char *argv[])
{
  int token_type;
  struct _tokens tokens;

  if (argc != 2)
  {
    printf("Usage: test <string>\n");
    exit(0);
  }

  tokens_init(&tokens, argv[1]);

  while(1)
  {
    token_type = tokens_next(&tokens);
    if (token_type == TOKEN_EOF) { break; }

    printf("%d) %s\n", token_type, tokens.next);
    if (token_type == TOKEN_ERROR) { break; }
  }

  tokens_free(&tokens);

  return 0;
}

