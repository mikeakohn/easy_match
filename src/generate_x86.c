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

int generate_init(struct _generate *generate, uint8_t *code, int option)
{
  //  mov [esp-4], esi: 0x89 0x74 0x24 0xfc
  generate_code(generate, 4, 0x89, 0x74, 0x24, 0xfc);

  //  mov [esp-8], edi: 0x89 0x7c 0x24 0xf8
  generate_code(generate, 4, 0x89, 0x7c, 0x24, 0xf8);

  //  mov edi, [esp+4]: 0x8b 0x7c 0x24 0x04
  generate_code(generate, 4, 0x8b, 0x7c, 0x24, 0x04);

  if (option == 1)
  {
    // strlen() is on stack
    //  mov esi, [esp+8]: 0x8b 0x74 0x24 0x08
    generate_code(generate, 4, 0x8b, 0x74, 0x24, 0x08);
  }
    else
  {
    // Add strlen() code
    //  mov esi, edi: 0x89 0xfe
    generate_code(generate, 2, 0x89, 0xfe);

    // cmp byte [esi], 0: 0x80 0x3e 0x00
    generate_code(generate, 3, 0x80, 0x3e, 0x00);

    // jz exit: 0x74 0x03
    generate_code(generate, 2, 0x74, 0x03);

    // inc esi: 0x46
    generate_code(generate, 1, 0x46);

    // jmp repeat: 0xeb 0xf6
    generate_code(generate, 2, 0xeb, 0xf8);

    // sub esi, edi: 0x29 0xfe
    generate_code(generate, 2, 0x29, 0xfe);
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

int generate_skip(struct _generate *generate, int offset_insert, int offset_goto, int reg, int skip_value)
{
  return -1;
}

int generate_finish(struct _generate *generate)
{
  // mov eax, esi: 0x89 0xf0
  generate_code(generate, 2, 0x89, 0xf0);

  // Restore used registers
  //  mov esi, [esp-4]: 0x8b 0x74 0x24 0xfc
  generate_code(generate, 4, 0x8b, 0x74, 0x24, 0xfc);

  //  mov edi, [esp-8]: 0x8b 0x7c 0x24 0xf8
  generate_code(generate, 4, 0x8b, 0x7c, 0x24, 0xf8);

  // ret: 0xc3
  generate_code(generate, 1, 0xc3);

  return 0;
}


