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

static int generate_strlen(struct _generate *generate, int len);

int generate_init(struct _generate *generate, uint8_t *code)
{
  generate->code = code;
  generate->ptr = 0;
  generate->reg = 0;

  // mov eax, eax: 0x31  0xC0
  generate->code[generate->ptr++] = 0x31;
  generate->code[generate->ptr++] = 0xc0;

  // mov rsi, rdi: 0x48  0x89  0xFE
  generate->code[generate->ptr++] = 0x48;
  generate->code[generate->ptr++] = 0x89;
  generate->code[generate->ptr++] = 0xfe;

  // Need to do an strlen()

  // cmp byte [rsi], 0:  0x80  0x3E  0x00
  generate->code[generate->ptr++] = 0x80;
  generate->code[generate->ptr++] = 0x3e;
  generate->code[generate->ptr++] = 0x00;

  // jz exit: 0x74 0x05
  generate->code[generate->ptr++] = 0x74;
  generate->code[generate->ptr++] = 0x05;

  // inc rsi:  0x48  0xFF  0xC6
  generate->code[generate->ptr++] = 0x48;
  generate->code[generate->ptr++] = 0xff;
  generate->code[generate->ptr++] = 0xc6;

  // jmp repeat: 0xEB 0xF6
  generate->code[generate->ptr++] = 0xeb;
  generate->code[generate->ptr++] = 0xf6;

  // sub rsi, rdi:  0x48  0x29  0xFE
  generate->code[generate->ptr++] = 0x48;
  generate->code[generate->ptr++] = 0x29;
  generate->code[generate->ptr++] = 0xfe;

  // mov rax, rsi:  0x48  0x89  0xF0
  //generate->code[generate->ptr++] = 0x48;
  //generate->code[generate->ptr++] = 0x89;
  //generate->code[generate->ptr++] = 0xf0;

  return 0;
}

int generate_not(struct _generate *generate)
{
  // xor eax, 1: 0x83  0xF0  0x01
  generate->code[generate->ptr++] = 0x83;
  generate->code[generate->ptr++] = 0xf0;
  generate->code[generate->ptr++] = 0x01;

  return 0;
}

int generate_startswith(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len);

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

  // ret: 0xc3
  generate->code[generate->ptr++] = 0xc3;

  return 0;
}

static int generate_strlen(struct _generate *generate, int len)
{
  if (len < 128)
  {
    // cmp rsi, 4:  0x48  0x83  0xFE  0x04
    generate->code[generate->ptr++] = 0x48;
    generate->code[generate->ptr++] = 0x83;
    generate->code[generate->ptr++] = 0xfe;
    generate->code[generate->ptr++] = len;
  }
    else
  {
    // cmp rsi, 200:  0x48  0x81  0xFE  0xC8  0x00  0x00  0x00
    generate->code[generate->ptr++] = 0x48;
    generate->code[generate->ptr++] = 0x81;
    generate->code[generate->ptr++] = 0xfe;
    generate->code[generate->ptr++] = len & 0xff;
    generate->code[generate->ptr++] = (len >> 8) & 0xff;
    generate->code[generate->ptr++] = (len >> 16) & 0xff;
    generate->code[generate->ptr++] = (len >> 24) & 0xff;
  }

  // jge exit: 0x73 0x03
  generate->code[generate->ptr++] = 0x7d;
  generate->code[generate->ptr++] = 0x01;

  // ret: 0xc3
  generate->code[generate->ptr++] = 0xc3;

  // jg exit_false: 0x7f 0xZZ (short)
  // jg exit_false: 0x0f 0x8f 0xZZ 0xZZ (near)
#if 0
  generate->code[generate->ptr++] = 0x0f;
  generate->code[generate->ptr++] = 0x8f;
  generate->code[generate->ptr++] = 0x00;
  generate->code[generate->ptr++] = 0x00;
  generate->exit_list[generate->exit_list_ptr++] = generate->ptr;
#endif

  return 0;
}

