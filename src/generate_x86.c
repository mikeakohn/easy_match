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

static int generate_check_len(struct _generate *generate, int len, int equals);
static int generate_mov_edi_end(struct _generate *generate, int len);
static int generate_match(struct _generate *generate, char *match, int len, int not);

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
  generate_check_len(generate, len, STRLEN_ATLEAST);
  if (generate_match(generate, match, len, not) == -1) { return -1; }

  return 0;
}

int generate_ends_with(struct _generate *generate, char *match, int len, int not)
{
  generate_check_len(generate, len, STRLEN_ATLEAST);
  if (generate_mov_edi_end(generate, len) != 0) { return -1; }

  if (generate_match(generate, match, len, not) == -1) { return -1; }

  // Restore edi.
  // mov edi, [esp-12]: 0x8b 0x7c 0x24 0xf4
  generate_code(generate, 4, 0x8b, 0x7c, 0x24, 0xf4);

  return 0;
}

int generate_match_at(struct _generate *generate, char *match, int len, int index, int not)
{
  if (index == 0)
  {
    // Save edi
    if (generate->dest_reg_saved == 0)
    {
      // mov [esp-12], edi: 0x89 0x7c 0x24 0xf4
      generate_code(generate, 4, 0x89, 0x7c, 0x24, 0xf4);

      generate->dest_reg_saved = 1;
    }
  }
    else
  if (index < 128)
  {
    // add edi, 1: 0x83 0xc7 0x01
    generate_code(generate, 3, 0x83, 0xc7, index);
  }
    else
  if (index < 32768)
  {
    // add edi, 128: 0x81 0xc7 0x80 0x00 0x00 0x00
    generate_code(generate, 6, 0x81, 0xc7,
      index & 0xff, (index >> 8) & 0xff,
      (index >> 16) & 0xff, (index >> 24) & 0xff);
  }
    else
  {
    return -1;
  }

  generate_check_len(generate, len + index, STRLEN_ATLEAST);
  if (generate_match(generate, match, len, not) == -1) { return -1; }

  if (index != 0)
  {
    // Restore edi.
    // mov edi, [esp-12]: 0x8b 0x7c 0x24 0xf4
    generate_code(generate, 4, 0x8b, 0x7c, 0x24, 0xf4);
  }

  return 0;
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

static int generate_check_len(struct _generate *generate, int len, int equals)
{
  if (len < 128)
  {
    // cmp esi, 1: 0x83 0xfe 0x01
    generate_code(generate, 3, 0x83, 0xfe, len);
  }
    else
  {
    // cmp esi, 128: 0x81 0xfe 0x80 0x00 0x00 0x00
    generate_code(generate, 6, 0x81, 0xfe,
      len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);
  }

  if (equals == STRLEN_ATLEAST)
  {
    // jl skip_exit: 0x7c, 0x00
    generate_code(generate, 2, 0x7c, 0x00);
  }
    else
  {
    // jne skip_exit: 0x75, 0x00
    generate_code(generate, 2, 0x75, 0x00);
  }

  generate->strlen_ptr = generate->ptr;

  return 0;
}

static int generate_mov_edi_end(struct _generate *generate, int len)
{
  // Move rdi to the end of the string.

  // Save edi
  if (generate->dest_reg_saved == 0)
  {
    // mov [esp-12], edi: 0x89 0x7c 0x24 0xf4
    generate_code(generate, 4, 0x89, 0x7c, 0x24, 0xf4);

    generate->dest_reg_saved = 1;
  }

  // add edi, esi: 0x01 0xf7
  generate_code(generate, 2, 0x01, 0xf7);

  if (len < 128)
  {
    // sub edi, 1: 0x83 0xef 0x01
    generate_code(generate, 3, 0x83, 0xef, len);
  }
    else
  if (len < 32768)
  {
    // sub edi, 128: 0x81 0xef 0x80 0x00 0x00 0x00
    generate_code(generate, 6, 0x81, 0xef,
      len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);
  }
    else
  {
    return -1;
  }

  return 0;
}

static int generate_match(struct _generate *generate, char *match, int len, int not)
{
  int n;

  n = 0;
  while (n < len)
  {
    if ((len - n) >= 4)
    {
      if (n == 0)
      {
        // cmp dword [edi], 0xff: 0x81 0x3f 0xff 0x00 0x00 0x00
        generate_code(generate, 6, 0x81, 0x3f,
          match[n+0], match[n+1], match[n+2], match[n+3]);
      }
        else
      if (n < 128)
      {
        // cmp dword [edi+1], 0xff: 0x81 0x7f 0x01 0xff 0x00 0x00 0x00
        generate_code(generate, 7, 0x81, 0x7f, n,
          match[n+0], match[n+1], match[n+2], match[n+3]);
      }
        else
      {
        // cmp dword [edi+128], 0xff: 0x81 0xbf 0x80 0x00 0x00 0x00 0xff 0x00
        generate_code(generate, 10, 0x81, 0xbf,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff,
          match[n+0], match[n+1], match[n+2], match[n+3]);
      }
    }
      else
    if ((len - n) >= 2)
    {
      if (n == 0)
      {
        // cmp word [edi], 0xff: 0x66 0x81 0x3f 0xff 0x00
        generate_code(generate, 5, 0x66, 0x81, 0x3f, match[n+0], match[n+1]);
      }
        else
      if (n < 128)
      {
        // cmp word [edi+1], 0xff: 0x66 0x81 0x7f 0x01 0xff 0x00
        generate_code(generate, 6, 0x66, 0x81, 0x7f, n, match[n+0], match[n+1]);
      }
        else
      {
        // cmp word [edi+128], 0xff: 0x66 0x81 0xbf 0x80 0x00 0x00 0x00 0xff
        generate_code(generate, 9, 0x66, 0x81, 0xbf,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff,
          match[n+0], match[n+1]);
      }
    }
      else
    if ((len - n) >= 1)
    {
      if (n == 0)
      {
        // cmp byte [edi], 0xff: 0x80 0x3f 0xff
        generate_code(generate, 3, 0x80, 0x3f, 0xff);
      }
        else
      if (n < 128)
      {
        // cmp byte [edi+1], 0xff: 0x80 0x7f 0x01 0xff
        generate_code(generate, 4, 0x80, 0x7f, n, match[0]);
      }
        else
      {
        // cmp byte [edi+128], 0xff: 0x80 0xbf 0x80 0x00 0x00 0x00 0xff
        generate_code(generate, 7, 0x80, 0xbf,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff,
          match[0]);
      }
    }
  }

  return 0;
}

