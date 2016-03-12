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
#include <stdarg.h>

#define STRLEN_ATLEAST 0
#define STRLEN_EQUALS 1

struct _generate
{
  uint8_t *code;
  int ptr;
  int reg;
  int starts_with;
  int ends_with;
  int match_at;
  int equals;
  int contains;
  int and;
  int or;
  int strlen_ptr;
  int strlen_is_far;
};

int generate_init(struct _generate *generate, uint8_t *code, int option);
int generate_starts_with(struct _generate *generate, char *match, int len, int not);
int generate_ends_with(struct _generate *generate, char *match, int len, int not);
int generate_match_at(struct _generate *generate, char *match, int len, int index, int not);
int generate_equals(struct _generate *generate, char *match, int len, int not);
int generate_contains(struct _generate *generate, char *match, int len, int not);
int generate_and(struct _generate *generate);
int generate_or(struct _generate *generate);
int generate_skip(struct _generate *generate, int offset_insert, int offset_goto, int reg, int skip_value);
int generate_finish(struct _generate *generate);

// Common functions
int generate_code(struct _generate *generate, uint8_t len, ...);
int generate_insert(struct _generate *generate, int offset, int len);

#endif

