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

#include <string.h>

#include "generate.h"

// r0 = string
// r1 = strlen(string)
// r2 = temp
// r3 = match
// r4 = temp
// r5-r11 = result stack

static int generate_check_len(struct _generate *generate, int len, int equals);
static int generate_match(struct _generate *generate, char *match, int len, int not);

int generate_init(struct _generate *generate, uint8_t *code, int option)
{
  // mov r3, #0xff: 0xff,0x30,0xa0,0xe3
  generate_code(generate, 4, 0xff, 0x30, 0xa0, 0xe3);

  // orr r3, r3, #0xff00: 0xff,0x3c,0x83,0xe3
  generate_code(generate, 4, 0xff, 0x3c, 0x83, 0xe3);

  // orr r3, r3, #0xff0000: 0xff,0x38,0x83,0xe3
  generate_code(generate, 4, 0xff, 0x38, 0x83, 0xe3);

  // orr r3, r3, #0xff000000: 0xff,0x34,0x83,0xe3
  generate_code(generate, 4, 0xff, 0x34, 0x83, 0xe3);

  // stm sp, { r4,r5,r6,r7,r8,r9,r10,r11 }: 0xf0,0x0f,0x0d,0xe9
  generate_code(generate, 4, 0xf0, 0x0f, 0x2d, 0xe9);

  if (option != 1)
  {
    // mov r1, #0: 0x00,0x10,0xa0,0xe3
    generate_code(generate, 4, 0x00, 0x10, 0xa0, 0xe3);

    // ldrb r2, [r0,r1]: 0x01,0x20,0xd0,0xe7
    generate_code(generate, 4, 0x01, 0x20, 0xd0, 0xe7);

    // cmp r2, #0: 0x00,0x00,0x52,0xe3
    generate_code(generate, 4, 0x00, 0x00, 0x52, 0xe3);

    // addne r1, r1, #1: 0x01,0x10,0x81,0x12
    generate_code(generate, 4, 0x01, 0x10, 0x81, 0x12);

    // bne 4: 0xfb 0xff 0xff 0x1a
    generate_code(generate, 4, 0xfb, 0xff, 0xff, 0x1a);
  }

  return 0;
}

int generate_starts_with(struct _generate *generate, char *match, int len, int not)
{
  generate_check_len(generate, len, STRLEN_ATLEAST);
  if (generate_match(generate, match, len, not) == -1) { return -1; }

  return 0;
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
  generate_check_len(generate, len, STRLEN_EQUALS);
  if (generate_match(generate, match, len, not) == -1) { return -1; }

  return 0;
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
  //generate_code(generate, 4, 0x01, 0x00, 0xa0, 0xe1);

  int push_list = push_list = (1 << generate->reg_max) - 1;

  // r4 is needed for temp. r5-r11 could have been used for calulation.
  push_list = (push_list << 5) | 0x10;

  // ldm sp, { r4,r5,r6,r7,r8,r9,r10,r11 }: 0xf0,0x0f,0x1d,0xe9
  generate_code(generate, 4, push_list & 0xff, (push_list >> 8), 0xbd, 0xe8);
  generate->code[16] = push_list & 0xff;
  generate->code[17] = push_list >> 8;

  // mov r0, r5: 0x05,0x00,0xa0,0xe1
  //generate_code(generate, 4, 0x05, 0x00, 0xa0, 0xe1);

  // bx lr: 0x1e 0xff 0x2f 0xe1
  generate_code(generate, 4, 0x1e, 0xff, 0x2f, 0xe1);

  uint8_t *strings = generate->code + generate->ptr;

  memcpy(strings, generate->strings, generate->strings_ptr);

  generate->code[0] = ((uint32_t)strings) & 0xff;
  generate->code[4] = (((uint32_t)strings) >> 8) & 0xff;
  generate->code[8] = (((uint32_t)strings) >> 16) & 0xff;
  generate->code[12] = (((uint32_t)strings) >> 24) & 0xff;

  return 0;
}

static int generate_check_len(struct _generate *generate, int len, int equals)
{
  if (len > 255) { return -1; }

  //  cmp r1, #1: 0x01,0x00,0x51,0xe3
  generate_code(generate, 4, len, 0x00, 0x51, 0xe3);

  if (equals == STRLEN_ATLEAST)
  {
    // blt 0: 0xfe,0xff,0xff,0xba (yum)
    generate_code(generate, 4, 0x00, 0x00, 0x00, 0xba);
  }
    else
  {
    // bne 0: 0xfe,0xff,0xff,0x1a
    generate_code(generate, 4, 0x00, 0x00, 0x00, 0x1a);
  }

  generate->strlen_ptr = generate->ptr;

  return 0;
}

static int generate_match(struct _generate *generate, char *match, int len, int not)
{
  int jmp_exit[4096];
  int jmp_exit_count = 0;
  int distance;
  int n;

  // Reg must be r4-r11 
  if (generate->reg > 7) { return -1; }

  n = 0;
  while (n < len)
  {
    if ((len - n) >= 4)
    {
      // ldr r2, [r0], #4: 0x04,0x20,0x90,0xe4
      //generate_code(generate, 4, 0x04, 0x20, 0x90, 0xe4);

      // ldr r4, [r3], #4: 0x04,0x40,0x93,0xe4
      //generate_code(generate, 4, 0x04, 0x40, 0x93, 0xe4);

      // ldr r2, [r0, #1]: 0x01,0x20,0x90,0xe5
      generate_code(generate, 4, n, 0x20, 0x90, 0xe5);

      // ldr r4, [r3, #1]: 0x01,0x40,0x93,0xe5
      generate_code(generate, 4, n, 0x40, 0x93, 0xe5);

      n += 4;
    }
      else
    if ((len - n) >= 2 && n < 256)
    {
      // Newer ARMs should support this.

      // ldrh r2, [r0, #1]: 0xbf 0x2f 0xd0 0xe1
      generate_code(generate, 4, 0xb0 | (n & 0xf), 0x20 | (n >> 4), 0xd0, 0xe1);

      // ldrh r4, [r3, #1]: 0xbf 0x4f 0xd3 0xe1
      generate_code(generate, 4, 0xb0 | (n & 0xf), 0x40 | (n >> 4), 0xd0, 0xe1);

      n += 2;
    }
      else
    if ((len - n) >= 1)
    {
      // ldrb r2, [r0, #1]: 0x01,0x20,0xd0,0xe5
      generate_code(generate, 4, n, 0x20, 0xd0, 0xe5);

      // ldrb r4, [r3, #1]: 0x01,0x40,0xd3,0xe5
      generate_code(generate, 4, n, 0x40, 0xd3, 0xe5);

      n++;
    }

    // cmp r2, r4: 0x04,0x00,0x52,0xe1
    generate_code(generate, 4, 0x04, 0x00, 0x52, 0xe1);

    if (n == len)
    {
      // moveq r0, #1: 0x01,0x00,0xa0,0x03
      generate_code(generate, 4, 0x01, 0x00, 0xa0, 0x03);

      // movne r0, #0: 0x00,0x00,0xa0,0x13
      generate_code(generate, 4, 0x00, 0x00, 0xa0, 0x13);
    }
      else
    {
      // bne 0: 0xfe,0xff,0xff,0x1a
      generate_code(generate, 4, 0x00, 0x00, 0x00, 0x1a);

      jmp_exit[jmp_exit_count++] = generate->ptr;
    }
  }

  for (n = 0; n < jmp_exit_count; n++)
  {
    distance = (generate->ptr - (jmp_exit[n] + 8)) / 4;

    generate->code[jmp_exit[n] - 4] = distance & 0xff;
    generate->code[jmp_exit[n] - 3] = (distance >> 8) & 0xff;
    generate->code[jmp_exit[n] - 2] = (distance >> 16) & 0xff;
  }

  distance = (generate->ptr - (generate->strlen_ptr + 8)) / 4;

  generate->code[generate->strlen_ptr - 4] = distance & 0xff;
  generate->code[generate->strlen_ptr - 3] = (distance >> 8) & 0xff;
  generate->code[generate->strlen_ptr - 2] = (distance >> 16) & 0xff;

  return 0;
}


