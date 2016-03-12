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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "generate.h"

#define STRLEN_ATLEAST 0
#define STRLEN_EQUALS 1
//#define MATCH_OPTION_NONE 0
//#define MATCH_OPTION_RESTORE_RDI 1

static int generate_check_len(struct _generate *generate, int len, int equals);
static int generate_mov_rdi_end(struct _generate *generate, int len);
static int generate_code(struct _generate *generate, uint8_t opcodes, ...);
static int generate_set_reg(struct _generate *generate, int value);
static int generate_insert(struct _generate *generate, int offset, int len);
static int generate_match(struct _generate *generate, char *match, int len, int not);

int generate_init(struct _generate *generate, uint8_t *code, int option)
{
  generate->code = code;
  generate->ptr = 0;
  generate->reg = 0;

#ifdef WINDOWS
  // mov [rsp-24], rcx: 0x48 0x89 0x4c 0x24 0xe8
  generate_code(generate, 5, 0x48, 0x89, 0x4c, 0x24, 0xe8);

  // mov [rsp-32], rdx: 0x48 0x89 0x54 0x24 0xe0
  generate_code(generate, 5, 0x48, 0x89, 0x54, 0x24, 0xe0);

  // mov rdi, rcx: 0x48 0x89 0xcf
  generate_code(generate, 3, 0x48, 0x89, 0xcf);

  if (option != 1)
  {
    // mov rsi, rdx: 0x48 0x89 0xd6
    generate_code(generate, 3, 0x48, 0x89, 0xd6);
  }
#endif

  // mov [rsp-8], rbx: 0x48 0x89 0x5c 0x24 0xf8
  generate_code(generate, 5, 0x48, 0x89, 0x5c, 0x24, 0xf8);

  // xor eax, eax: 0x31  0xC0
  //generate_code(generate, 2, 0x31, 0xc0);

  if (option != 1)
  {
    // mov rsi, rdi: 0x48 0x89 0xfe
    generate_code(generate, 3, 0x48, 0x89, 0xfe);

    // Need to do an strlen()

    // cmp byte [rsi], 0: 0x80 0x3e 0x00
    generate_code(generate, 3, 0x80, 0x3e, 0x00);

    // jz exit: 0x74 0x05
    generate_code(generate, 2, 0x74, 0x05);

    // inc rsi: 0x48 0xff 0xc6
    generate_code(generate, 3, 0x48, 0xff, 0xc6);

    // jmp repeat: 0xEB 0xF6
    generate_code(generate, 2, 0xeb, 0xf6);

    // sub rsi, rdi: 0x48 0x29 0xfe
    generate_code(generate, 3, 0x48, 0x29, 0xfe);
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
  if (generate_mov_rdi_end(generate, len) != 0) { return -1; }

  if (generate_match(generate, match, len, not) == -1) { return -1; }

  // Restore rdi.
  // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);

  return 0;
}

int generate_match_at(struct _generate *generate, char *match, int len, int index, int not)
{
  // Save rdi
  // mov [rsp-16], rdi: 0x48 0x89 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x89, 0x7c, 0x24, 0xf0);

  if (index == 0)
  {
  }
    else
  if (index < 128)
  {
    // add rdi, 1: 0x48 0x83 0xc7 0x01
    generate_code(generate, 4, 0x48, 0x83, 0xc7, index);
  }
    else
  if (index < 32768)
  {
    // add rdi, 128: 0x48 0x81 0xc7 0x80 0x00 0x00 0x00
    generate_code(generate, 7, 0x48, 0x81, 0xc7,
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
    // Restore rdi.
    // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
    generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);
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

  // Save rdi
  // mov [rsp-16], rdi: 0x48 0x89 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x89, 0x7c, 0x24, 0xf0);

  generate_set_reg(generate, not);

  generate_check_len(generate, len, STRLEN_ATLEAST);

  // mov rcx, rdi: 0x48 0x89 0xf9
  //generate_code(generate, 3, 0x48, 0x89, 0xf9);

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

  //if (generate_mov_rdi_end(generate, len) != 0) { return -1; }

  label = generate->ptr;

  if (generate_match(generate, match, len, not) == -1) { return -1; }

  // inc rdi: 0x48 0xff 0xc7
  generate_code(generate, 3, 0x48, 0xff, 0xc7);

  switch(generate->reg)
  {
    case 0:
      // test eax, eax: 0x85 0xc0
      generate_code(generate, 2, 0x85, 0xc0);
      break;
    case 1:
      // test r8, r8: 0x4d 0x85 0xc0
      generate_code(generate, 3, 0x4d, 0x85, 0xc0);
      break;
    case 2:
      // test r9, r9: 0x4d 0x85 0xc9
      generate_code(generate, 3, 0x4d, 0x85, 0xc9);
      break;
    case 3:
      // test r10, r10: 0x4d 0x85 0xd2
      generate_code(generate, 3, 0x4d, 0x85, 0xd2);
      break;
    case 4:
      // test r11, r11: 0x4d 0x85 0xdb
      generate_code(generate, 3, 0x4d, 0x85, 0xdb);
      break;
    default:
      return -1;
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

  // cmp [rsp-16], rdi: 0x48 0x39 0x7c 0x24 0xf0
  //generate_code(generate, 5, 0x48, 0x39, 0x7c, 0x24, 0xf0);

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

  // Restore rdi

  // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);

  return 0;
}

int generate_and(struct _generate *generate)
{
  switch(generate->reg)
  {
    case 2:
      // and rax, r8: 0x4c 0x21 0xc0
      generate_code(generate, 3, 0x4c, 0x21, 0xc0);
      break;
    case 3:
      // and r8, r9: 0x4d 0x21 0xc8
      generate_code(generate, 3, 0x4d, 0x21, 0xc8);
      break;
    case 4:
      // and r9, r10: 0x4d 0x21 0xd1
      generate_code(generate, 3, 0x4d, 0x21, 0xd1);
      break;
    case 5:
      // and r10, r11: 0x4d 0x21 0xda
      generate_code(generate, 3, 0x4d, 0x21, 0xda);
      break;
    default:
      return -1;
  }

  generate->reg--;

  return 0;
}

int generate_or(struct _generate *generate)
{
  switch(generate->reg)
  {
    case 2:
      // or rax, r8: 0x4c 0x09 0xc0
      generate_code(generate, 3, 0x4c, 0x09, 0xc0);
      break;
    case 3:
      // or r8, r9: 0x4d 0x09 0xc8
      generate_code(generate, 3, 0x4d, 0x09, 0xc8);
      break;
    case 4:
      // or r9, r10: 0x4d 0x09 0xd1
      generate_code(generate, 3, 0x4d, 0x09, 0xd1);
      break;
    case 5:
      // or r10, r11: 0x4d 0x09 0xda
      generate_code(generate, 3, 0x4d, 0x09, 0xda);
      break;
    default:
      return -1;
  }

  generate->reg--;

  return 0;
}

int generate_finish(struct _generate *generate)
{
  // inc eax: 0xff 0xc0
  //generate_code(generate, 2, 0xff, 0xc0);

  // mov rbx, [rsp-8]: 0x48 0x8b 0x5c 0x24 0xf8
  generate_code(generate, 5, 0x48, 0x8b, 0x5c, 0x24, 0xf8);

  // ret: 0xc3
  generate_code(generate, 1, 0xc3);

//#define DEBUG
#ifdef DEBUG
  FILE *out = fopen("/tmp/debug.bin", "wb");
  fwrite(generate->code, 1, generate->ptr, out);
  fclose(out);
#endif

  return 0;
}

static int generate_check_len(struct _generate *generate, int len, int equals)
{
  if (len < 128)
  {
    // cmp rsi, 1: 0x48 0x83 0xfe 0x01
    generate_code(generate, 4, 0x48, 0x83, 0xfe, len);
  }
    else
  {
    // cmp rsi, 128: 0x48 0x81 0xfe 0x80 0x00 0x00 0x00
    generate_code(generate, 7, 0x48, 0x81, 0xfe, 
      len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);
  }

  // Note to self: make this possible to be a short jump
  if (equals == STRLEN_ATLEAST)
  {
    // jl skip_exit: 0x0f, 0x8c, 0x00, 0x00, 0x00, 0x00
    //generate_code(generate, 6, 0x0f, 0x8c, 0x00, 0x00, 0x00, 0x00);

    // jl skip_exit: 0x7c, 0x00
    generate_code(generate, 2, 0x7c, 0x00);
  }
    else
  {
    // jne skip_exit: 0x0f, 0x85, 0x00, 0x00, 0x00, 0x00
    //generate_code(generate, 6, 0x0f, 0x85, 0x00, 0x00, 0x00, 0x00);

    // jne skip_exit: 0x75, 0x00
    generate_code(generate, 2, 0x75, 0x00);
  }

  generate->strlen_ptr = generate->ptr;

  return 0;
}

static int generate_mov_rdi_end(struct _generate *generate, int len)
{
  // Move rdi to the end of the string.
  // mov [rsp-16], rdi: 0x48 0x89 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x89, 0x7c, 0x24, 0xf0);

  // add rdi, rsi: 0x48 0x01 0xf7
  generate_code(generate, 3, 0x48, 0x01, 0xf7);

  if (len < 128)
  {
    // sub rdi, 100: 0x48 0x83 0xef 0x64
    generate_code(generate, 4, 0x48, 0x83, 0xef, len);
  }
    else
  if (len < 32768)
  {
    // sub rdi, 128: 0x48 0x81 0xef 0x80 0x00 0x00 0x00
    generate_code(generate, 7, 0x48, 0x81, 0xef,
      len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);
  }
    else
  {
    return -1;
  }

  return 0;
}

static int generate_code(struct _generate *generate, uint8_t len, ...)
{
  va_list argp;
  int i, data;

  va_start(argp, len);

  for (i = 0; i < len; i++)
  {
    data = va_arg(argp, int);
    generate->code[generate->ptr++] = data;
  }

  va_end(argp);

  return 0;
}

static int generate_set_reg(struct _generate *generate, int value)
{
  if (generate->reg > 4) { return -1; }

  if (generate->reg == 0)
  {
    if (value == 0)
    {
      // xor eax, eax: 0x31 0xc0
      generate_code(generate, 2, 0x31, 0xc0);
      return 2;
    }
      else
    {
      // mov eax, 1: 0xb8 0x01 0x00 0x00 0x00
      generate_code(generate, 5, 0xb8, 0x01, 0x00, 0x00, 0x00);
      return 5;
    }
  }
    else
  {
    if (value == 0)
    {
      uint8_t reg_bytes[] = { 0xc0, 0xc9, 0xd2, 0xdb, 0xe4, 0xed, 0xf6, 0xff };
      // xor r8,r8: 0x4d 0x31 0xc0
      generate_code(generate, 3, 0x4d, 0x31, reg_bytes[generate->reg -1 ]);
      return 3;
    }
      else
    {
      // mov r8, 1: 0x41 0xb8 0x01 0x00 0x00 0x00
      generate_code(generate, 6, 0x41, 0xb8 + generate->reg - 1, 0x01, 0x00, 0x00, 0x00);
      return 6;
    }
  }
}

static int generate_insert(struct _generate *generate, int offset, int len)
{
  int i = generate->ptr;

  while(i >= offset)
  {
    generate->code[i + len] = generate->code[i];
    i--;
  }

  generate->ptr += len;

  return 0;
}

static int generate_match(struct _generate *generate, char *match, int len, int not)
{
  int jmp_exit[4096];
  int jmp_exit_count = 0;
  int distance;
  int n;

  // Reg must be rax, r8, r9, r10, r11
  if (generate->reg > 4) { return -1; }

  n = 0;
  while (n < len)
  {
    if ((len - n) >= 8)
    {
      // mov rbx, 0x8877665544332211: 0x48 0xbb 0x11 ...
      generate_code(generate, 10, 0x48, 0xbb,
        match[n+0], match[n+1], match[n+2], match[n+3],
        match[n+4], match[n+5], match[n+6], match[n+7]);

      if (n == 0)
      {
        // mov rdx, [rdi]: 0x48 0x8b 0x17
        generate_code(generate, 3, 0x48, 0x8b, 0x17);
      }
        else
      if (n < 128)
      {
        // mov rdx, [rdi+1]: 0x48 0x8b 0x57 0x01
        generate_code(generate, 4, 0x48, 0x8b, 0x57, n);
      }
        else
      {
        // mov rdx, [rdi+128]: 0x48 0x8b 0x97 0x80 0x00 0x00 0x00
        generate_code(generate, 7, 0x48, 0x8b, 0x97,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff);
      }

      // cmp rdx, rbx: 0x48 0x39 0xda
      generate_code(generate, 3, 0x48, 0x39, 0xda);

      n += 8;
    }
      else
    if ((len - n) >= 4)
    {
      if (n == 0)
      {
        // cmp dword [rdi], 0xff: 0x81 0x3f 0xff 0x00 0x00 0x00
        generate_code(generate, 6, 0x81, 0x3f,
          match[n+0], match[n+1], match[n+2], match[n+3]);
      }
        else
      if (n < 128)
      {
        // cmp dword [rdi+1], 0xff: 0x81 0x7f 0x01 0xff 0x00 0x00 0x00
        generate_code(generate, 7, 0x81, 0x7f, n,
          match[n+0], match[n+1], match[n+2], match[n+3]);
      }
        else
      {
        // cmp dword [rdi+128], 0xff: 0x81 0xbf 0x80 0x00 0x00 0x00 0xff 0x00
        generate_code(generate, 10, 0x81, 0xbf,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff,
          match[n+0], match[n+1], match[n+2], match[n+3]);
      }

      n += 4;
    }
      else
    if ((len - n) >= 2)
    {
      if (n == 0)
      {
        // cmp word [rdi], 0xff: 0x66 0x81 0x3f 0xff 0x00
        generate_code(generate, 5, 0x66, 0x81, 0x3f, match[n+0], match[n+1]);
      }
        else
      if (n < 128)
      {
        // cmp word [rdi+1], 0xff: 0x66 0x81 0x7f 0x01 0xff 0x00
        generate_code(generate, 6, 0x66, 0x81, 0x7f, n, match[n+0], match[n+1]);
      }
        else
      {
        // cmp word [rdi+128], 0xff: 0x66 0x81 0xbf 0x80 0x00 0x00 0x00 0xff 0x
        generate_code(generate, 9, 0x66, 0x81, 0xbf,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff,
          match[n+0], match[n+1]);
      }

      n += 2;
    }
      else
    {
      if (n == 0)
      {
        // cmp byte [rdi], 128: 0x80 0x3f 0x80
        generate_code(generate, 3, 0x80, 0x3f, match[n+0]);
      }
        else
      if (n < 128)
      {
        // cmp byte [rdi+1], 0xff: 0x80 0x7f 0x01 0xff
        generate_code(generate, 4, 0x80, 0x7f, n, match[n+0]);
      }
        else
      {
        // cmp byte [rdi+128], 0xff: 0x80 0xbf 0x80 0x00 0x00 0x00 0xff
        generate_code(generate, 7, 0x80, 0xbf,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff,
          match[n+0]);
      }

      n++;
    }

    // Note to self: This could be optimized by 2 bytes with some work

    // jne skip_exit: 0x0f, 0x85, 0x01, 0x00, 0x00, 0x00
    //generate_code(generate, 6, 0x0f, 0x85, 0x00, 0x00, 0x00, 0x00);

    // jne skip_exit: 0x75, 0x01
    generate_code(generate, 2, 0x75, 0x00);

    if (jmp_exit_count == 4096) { return -1; }

    jmp_exit[jmp_exit_count++] = generate->ptr;
  }

  generate_set_reg(generate, not ^ 1);

  // jmp exit_block: 0xEB 0x00
  generate_code(generate, 2, 0xeb, 0x00);

  //int label = generate->ptr;

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

