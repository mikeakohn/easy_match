/**
 *  Easy Match
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2016-2019 by Michael Kohn
 *
 */

#include "generate.h"

int generate_init(struct _generate *generate, uint8_t *code, int option)
{
  return -1;
}

int generate_starts_with(struct _generate *generate, char *match, int len, int not)
{
  return -1;
}

int generate_ends_with(struct _generate *generate, char *match, int len, int not)
{
  return -1;
}

int generate_match_at(struct _generate *generate, char *match, int len, int index, int not)
{
  return -1;
}

int generate_equals(struct _generate *generate, char *match, int len, int not)
{
  return -1;
}

int generate_contains(struct _generate *generate, char *match, int len, int not)
{
  return -1;
}

int generate_and(struct _generate *generate)
{
  return -1;
}

int generate_or(struct _generate *generate)
{
  return -1;
}

int generate_skip(struct _generate *generate, int offset_insert, int offset_goto, int reg, int skip_value, int pop_to_reg)
{
  return -1;
}

int generate_string_const_add(struct _generate *generate, int offset)
{
  return -1;
}

int generate_finish(struct _generate *generate)
{
  return -1;
}


