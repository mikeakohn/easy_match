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
static int generate_save_edi(struct _generate *generate);
static int generate_mov_edi_end(struct _generate *generate, int len);
static int generate_set_reg(struct _generate *generate, int value);
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
  int label;
  int distance;

  generate_save_edi(generate);
  generate_set_reg(generate, not);
  generate_check_len(generate, len, STRLEN_ATLEAST);

  // mov ecx, esi: 0x89 0xf1
  generate_code(generate, 2, 0x89, 0xf1);

  if (len < 128)
  {
    // sub ecx, 1: 0x83 0xe9 0x01
    generate_code(generate, 3, 0x83, 0xe9, len);
  }
    else
  {
    // sub ecx, 128: 0x81 0xe9 0x80 0x00 0x00 0x00
    generate_code(generate, 6, 0x81, 0xe9,
      len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);
  }

  label = generate->ptr;

  if (generate_match(generate, match, len, not) == -1) { return -1; }

  // inc edi: 0x47
  generate_code(generate, 1, 0x47);

  switch(generate->reg)
  {
    case 0:
      // test eax, eax: 0x85 0xc0
      generate_code(generate, 2, 0x85, 0xc0);
      break;
    case 1:
      // test edx, edx: 0x85 0xd2
      generate_code(generate, 2, 0x85, 0xd2);
      break;
    default:
      // mov ebx, [esp-20]: 0x8b 0x5c 0x24 0xec
      generate_code(generate, 4, 0x8b, 0x5c, 0x24, 0xec - ((generate->reg - 2) * 4));

      // test ebx, ebx: 0x85 0xdb
      generate_code(generate, 2, 0x85, 0xdb);
      break;
  }

  if (not == 0)
  {
    // jne skip_exit: 0x75, 0x00
    generate_code(generate, 2, 0x75, 0x00);
  }
    else
  {
    // je skip_exit: 0x74, 0x00
    generate_code(generate, 2, 0x74, 0x00);
  }

  int label_skip_exit = generate->ptr;

  // dec ecx: 0xff 0xc9
  generate_code(generate, 2, 0xff, 0xc9);

  distance = generate->ptr - label;

  if (distance + 2 <= 128)
  {
    distance = -(distance + 2);

    // jnz label: 0x75 label
    generate_code(generate, 2, 0x75, distance);
  }
    else
  {
    distance = -(distance + 3);

    // jnz label: 0x0f 0x85 label
    generate_code(generate, 3, 0x0f, 0x85,
      distance & 0xff, (distance >> 8) & 0xff);
  }

  distance = generate->ptr - label_skip_exit;
  generate->code[label_skip_exit - 1] = distance;

  // strlen() check should jump here.
  distance = generate->ptr - generate->strlen_ptr;

  if (distance < 128)
  {
    generate->code[generate->strlen_ptr - 1] = distance;
  }
    else
  {
    if (generate->strlen_is_far == 0)
    {
      generate_insert(generate, generate->strlen_ptr, 4);

      generate->strlen_ptr += 4;
      distance = generate->ptr - generate->strlen_ptr;

      generate->code[generate->strlen_ptr - 6] = 0x0f;
      generate->code[generate->strlen_ptr - 5] = 0x8c;
    }

    generate->code[generate->strlen_ptr - 4] = distance & 0xff;
    generate->code[generate->strlen_ptr - 3] = (distance >> 8) & 0xff;
    generate->code[generate->strlen_ptr - 2] = (distance >> 16) & 0xff;
    generate->code[generate->strlen_ptr - 1] = (distance >> 24) & 0xff;
  }

  // Restore edi
  // mov edi, [esp-12]: 0x8b 0x7c 0x24 0xf4
  generate_code(generate, 4, 0x8b, 0x7c, 0x24, 0xf4);

  return 0;
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

  // mov ebx, [esp-16]: 0x8b 0x5c 0x24 0xf0
  generate_code(generate, 4, 0x8b, 0x5c, 0x24, 0xf0);

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

static int generate_save_edi(struct _generate *generate)
{
  if (generate->dest_reg_saved == 0)
  {
    // mov [esp-12], edi: 0x89 0x7c 0x24 0xf4
    generate_code(generate, 4, 0x89, 0x7c, 0x24, 0xf4);

    generate->dest_reg_saved = 1;
  }

  return 0;
}

static int generate_mov_edi_end(struct _generate *generate, int len)
{
  // Move rdi to the end of the string.

  generate_save_edi(generate);

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

static int generate_set_reg(struct _generate *generate, int value)
{
  if (generate->reg > 16) { return -1; }

  if (generate->reg == 0)
  {
    // xor eax, eax: 0x31 0xc0
    generate_code(generate, 2, 0x31, 0xc0);

    if (value == 0)
    {
      return 2;
    }
      else
    {
      // inc eax: 0x40
      generate_code(generate, 1, 0x40);
      return 3;
    }
  }
    else
  if (generate->reg == 1)
  {
    // xor edx, edx: 0x31 0xd2
    generate_code(generate, 2, 0x31, 0xd2);

    if (value == 0)
    {
      return 2;
    }
      else
    {
      // inc edx: 0x42
      generate_code(generate, 1, 0x42);
      return 3;
    }
  }
    else
  {
    int start = generate->ptr;

    // xor ebx, ebx: 0x31 0xdb
    generate_code(generate, 2, 0x31, 0xdb);

    if (value != 0)
    {
      // inc ebx: 0x43
      generate_code(generate, 1, 0x43);
    }

    // mov [esp-20], ebx: 0x89 0x5c 0x24 0xec
    generate_code(generate, 4, 0x89, 0x5c, 0x24, 0xec - ((generate->reg - 2) * 4));

    return generate->ptr - start;
  }
}

static int generate_match(struct _generate *generate, char *match, int len, int not)
{
  int jmp_exit[4096];
  int jmp_exit_count = 0;
  int distance;
  int n;

  if (generate->reg > 16) { return -1; }

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

  generate_set_reg(generate, not ^ 1);

  // jmp exit_block: 0xeb 0x00
  generate_code(generate, 2, 0xeb, 0x00);

  for (n = jmp_exit_count - 1; n >= 0; n--)
  {
    distance = generate->ptr - jmp_exit[n];

    if (distance < 128)
    {
      generate->code[jmp_exit[n] - 1] = distance;
    }
      else
    {
      generate_insert(generate, jmp_exit[n], 4);

      jmp_exit[n] += 4;
      distance = generate->ptr - jmp_exit[n];

      generate->code[jmp_exit[n] - 6] = 0x0f;
      generate->code[jmp_exit[n] - 5] = 0x85;
      generate->code[jmp_exit[n] - 4] = distance & 0xff;
      generate->code[jmp_exit[n] - 3] = (distance >> 8) & 0xff;
      generate->code[jmp_exit[n] - 2] = (distance >> 16) & 0xff;
      generate->code[jmp_exit[n] - 1] = (distance >> 24) & 0xff;
    } 
  } 
  
  distance = generate->ptr - generate->strlen_ptr;

  if (distance < 128)
  {
    generate->code[generate->strlen_ptr - 1] = distance;
    generate->strlen_is_far = 0;
  }
    else
  {
    generate_insert(generate, generate->strlen_ptr, 4);

    generate->strlen_ptr += 4;
    distance = generate->ptr - generate->strlen_ptr;

    if (generate->code[generate->strlen_ptr - 6] == 0x75)
    {
      generate->code[generate->strlen_ptr - 6] = 0x0f;
      generate->code[generate->strlen_ptr - 5] = 0x85;
    }
      else
    {
      generate->code[generate->strlen_ptr - 6] = 0x0f;
      generate->code[generate->strlen_ptr - 5] = 0x8c;
    }

    generate->code[generate->strlen_ptr - 4] = distance & 0xff;
    generate->code[generate->strlen_ptr - 3] = (distance >> 8) & 0xff;
    generate->code[generate->strlen_ptr - 2] = (distance >> 16) & 0xff;
    generate->code[generate->strlen_ptr - 1] = (distance >> 24) & 0xff;

    generate->strlen_is_far = 0;
  }

  n = generate_set_reg(generate, not);

  generate->code[generate->ptr - n - 1] = n;

  return 0;
}

