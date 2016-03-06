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

#ifndef _GENERATE_H
#define _GENERATE_H

#include <stdint.h>

struct _generate
{
  uint8_t *code;
  int ptr;
  int reg;
  int startswith;
  int endswith;
  int equals;
  int contains;
  int and;
  int or;
};

int generate_init(struct _generate *generate, uint8_t *code);
int generate_not(struct _generate *generate);
int generate_startswith(struct _generate *generate, char *match);
int generate_endswith(struct _generate *generate, char *match);
int generate_equals(struct _generate *generate, char *match);
int generate_contains(struct _generate *generate, char *match);
int generate_finish(struct _generate *generate);

#endif

