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

// r0 = string
// r1 = strlen(string)
// r4-r11 = result stack

int generate_init(struct _generate *generate, uint8_t *code, int option)
{
  if (option != 1)
  {
    // mov r1, r0: 0x00 0x10 0xa0 0xe1
    generate_code(generate, 4, 0x00, 0x10, 0xa0, 0xe1);

    // ldrb r2, [r1]: 0x00 0x20 0xd1 0xe5
    generate_code(generate, 4, 0x00, 0x20, 0xd1, 0xe5);

    // cmp r2, #1: 0x01 0x00 0x52 0xe3
    generate_code(generate, 4, 0x01, 0x00, 0x52, 0xe3);

    // addne r2, r2, #1: 0x01 0x20 0x82 0x12
    generate_code(generate, 4, 0x01, 0x20, 0x82, 0x12);

    // bne 0: 0xfe 0xff 0xff 0x1a
    generate_code(generate, 4, 0xfa, 0xff, 0xff, 0x1a);
  }

  return 0;
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

int generate_finish(struct _generate *generate)
{
  // DEBUG
  // mov r0, r1: 0x01,0x00,0xa0,0xe1
  generate_code(generate, 4, 0x01, 0x00, 0xa0, 0xe1);

  // bx lr: 0x1e 0xff 0x2f 0xe1
  generate_code(generate, 4, 0x1e, 0xff, 0x2f, 0xe1);

  return 0;
}


