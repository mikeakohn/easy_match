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

#include "generate.h"

int generate_init(struct _generate *generate, uint8_t *code)
{
  generate->code = code;
  generate->ptr = 0;
  generate->reg = 0;

  return 0;
}

int generate_not(struct _generate *generate)
{
  return 0;
}

int generate_startswith(struct _generate *generate, char *match)
{
  return 0;
}

int generate_endswith(struct _generate *generate, char *match)
{
  return -1;
}

int generate_equals(struct _generate *generate, char *match)
{
  return -1;
}

int generate_contains(struct _generate *generate, char *match)
{
  return -1;
}

int generate_finish(struct _generate *generate)
{
  // mov eax, 1: 0xB8  0x01  0x00  0x00  0x00
  generate->code[generate->ptr++] = 0xb8;
  generate->code[generate->ptr++] = 0x01;
  generate->code[generate->ptr++] = 0x00;
  generate->code[generate->ptr++] = 0x00;
  generate->code[generate->ptr++] = 0x00;

  // mov eax, eax: 0x31  0xC0
  //generate->code[generate->ptr++] = 0x31;
  //generate->code[generate->ptr++] = 0xc0;

  // ret: 0xc3
  generate->code[generate->ptr++] = 0xc3;

  return 0;
}


