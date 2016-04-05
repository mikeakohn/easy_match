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

#include "generate.h"

#define TYPE_STRNCMP 1
#define TYPE_STRCMP 0

static int generate_check_len(struct _generate *generate, int len, int equals);
static int generate_save_rdi(struct _generate *generate);
static int generate_mov_rdi_end(struct _generate *generate, int len);
static int generate_test_reg(struct _generate *generate, int reg);
static int generate_set_reg(struct _generate *generate, int value);
static int generate_strncmp(struct _generate *generate, char *match, int len, int not, int type);
static int generate_match(struct _generate *generate, char *match, int len, int not);

int generate_init(struct _generate *generate, uint8_t *code, int option)
{
#ifdef WINDOWS
  // mov [rsp-0x18], rsi: 0x48,0x89,0x74,0x24,0xe8
  generate_code(generate, 5, 0x48, 0x89, 0x74, 0x24, 0xe8);
#endif

  if (generate->use_strncmp == 1)
  {
    // mov qword rsi, 0x7ffff8ffff1111: 0x48,0xbe,0x11,0x11,0xff,0xff,...
    generate_code(generate, 10, 0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  }

#ifdef WINDOWS
  // mov [rsp-0x20], rdi: 0x48,0x89,0x7c,0x24,0xe0
  generate_code(generate, 5, 0x48, 0x89, 0x7c, 0x24, 0xe0);

  // mov rdi, rcx: 0x48 0x89 0xcf
  generate_code(generate, 3, 0x48, 0x89, 0xcf);

  if (option != 1 && generate->use_strncmp == 0)
  {
    // mov rsi, rdx: 0x48 0x89 0xd6
    generate_code(generate, 3, 0x48, 0x89, 0xd6);
  }
#endif

  // mov [rsp-8], rbx: 0x48 0x89 0x5c 0x24 0xf8
  generate_code(generate, 5, 0x48, 0x89, 0x5c, 0x24, 0xf8);

  if (option != 1 && generate->use_strncmp == 0)
  {
    // mov rsi, rdi: 0x48 0x89 0xfe
    generate_code(generate, 3, 0x48, 0x89, 0xfe);

    // Need to do an strlen()

//#define OLDWAY
#ifdef OLDWAY
    // cmp byte [rsi], 0: 0x80 0x3e 0x00
    generate_code(generate, 3, 0x80, 0x3e, 0x00);

    // jz exit: 0x74 0x05
    generate_code(generate, 2, 0x74, 0x05);

    // inc rsi: 0x48 0xff 0xc6
    generate_code(generate, 3, 0x48, 0xff, 0xc6);

    // jmp repeat: 0xEB 0xF6
    generate_code(generate, 2, 0xeb, 0xf6);
#else

    // inc rsi: 0x48 0xff 0xc6
    generate_code(generate, 3, 0x48, 0xff, 0xc6);

    // cmp byte [rsi-1], 0: 0x80,0x7e,0xff,0x00
    generate_code(generate, 4, 0x80, 0x7e, 0xff, 0x00);

    // jnz exit: 0x75 0x03
    generate_code(generate, 2, 0x75, 0xf7);

    // dec rsi: 0x48,0xff,0xce
    generate_code(generate, 3, 0x48, 0xff, 0xce);
#endif

    // sub rsi, rdi: 0x48 0x29 0xfe
    generate_code(generate, 3, 0x48, 0x29, 0xfe);
  }

  return 0;
}

int generate_starts_with(struct _generate *generate, char *match, int len, int not)
{
  if (generate->use_strncmp == 0)
  {
    generate_check_len(generate, len, STRLEN_ATLEAST);
    if (generate_match(generate, match, len, not) == -1) { return -1; }
  }
    else
  {
    generate_save_rdi(generate);
    if (generate_strncmp(generate, match, len, not, TYPE_STRNCMP) == -1) { return -1; }

    // Restore rdi.
    // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
    generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);
  }

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
  if (index == 0)
  {
  }
    else
  if (index < 128)
  {
    generate_save_rdi(generate);

    // add rdi, 1: 0x48 0x83 0xc7 0x01
    generate_code(generate, 4, 0x48, 0x83, 0xc7, index);
  }
    else
  if (index < 32768)
  {
    generate_save_rdi(generate);

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
  if (generate->use_strncmp == 0)
  {
    generate_check_len(generate, len, STRLEN_EQUALS);
    if (generate_match(generate, match, len, not) == -1) { return -1; }
  }
    else
  {
    generate_save_rdi(generate);
    if (generate_strncmp(generate, match, len, not, TYPE_STRCMP) == -1) { return -1; }

    // Restore rdi.
    // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
    generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);
  }

  return 0;
}

int generate_contains(struct _generate *generate, char *match, int len, int not)
{
  int label;
  int distance;
  int sub_len = len - 1;

  generate_save_rdi(generate);
  generate_set_reg(generate, not);
  generate_check_len(generate, len, STRLEN_ATLEAST);

  // mov ecx, esi: 0x89 0xf1
  generate_code(generate, 2, 0x89, 0xf1);

  if (sub_len < 128)
  {
    // sub ecx, 1: 0x83 0xe9 0x01
    generate_code(generate, 3, 0x83, 0xe9, sub_len);
  }
    else
  {
    // sub ecx, 128: 0x81 0xe9 0x80 0x00 0x00 0x00
    generate_code(generate, 6, 0x81, 0xe9,
      sub_len & 0xff, (sub_len >> 8) & 0xff, (sub_len >> 16) & 0xff, (sub_len >> 24) & 0xff);
  }

  label = generate->ptr;

  if (generate_match(generate, match, len, not) == -1) { return -1; }

  // inc rdi: 0x48 0xff 0xc7
  generate_code(generate, 3, 0x48, 0xff, 0xc7);

  // mov does't set flags, but an inc should.  This could be an
  // optimization.
  int count = generate_test_reg(generate, generate->reg);
  if (count == 0) { return -1; }

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
    generate_code(generate, 6, 0x0f, 0x85,
      distance & 0xff, (distance >> 8) & 0xff,
      (distance >> 16) & 0xff, (distance >> 24) & 0xff);
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

int generate_skip(struct _generate *generate, int offset_insert, int offset_goto, int reg, int skip_value, int pop_to_reg)
{
  int distance;
  int ptr_save = generate->ptr;
  uint8_t code[64];

  int count = generate_test_reg(generate, reg);
  if (count == 0) { return -1; }

  if (reg != pop_to_reg)
  {
    int condition = skip_value == 1 ? 0x45 : 0x44;
    int regs = (reg - 1);
    int opcode = 0x49;

    if (pop_to_reg == 0)
    {
      regs |= 0xc0;
    }
      else
    {
      regs |= pop_to_reg - 1;
      opcode = 0x4d;
    }

    // cmov[z/nz] pop_reg, reg
    generate_code(generate, 4, opcode, 0x0f, condition, regs);
  }

  distance = offset_goto - offset_insert;

  if (distance < 128)
  {
    if (skip_value == 0)
    {
      // jz skip_exit: 0x74, 0x00
      generate_code(generate, 2, 0x74, distance);
    }
      else
    {
      // jnz label: 0x75 label
      generate_code(generate, 2, 0x75, distance);
    }
  }
    else
  {
    if (skip_value == 0)
    {
      // jz label: 0x0f 0x84 label
      generate_code(generate, 6, 0x0f, 0x84,
        distance & 0xff, (distance >> 8) & 0xff,
        (distance >> 16) & 0xff, (distance >> 24) & 0xff);
    }
      else
    {
      // jnz label: 0x0f 0x85 label
      generate_code(generate, 6, 0x0f, 0x85,
        distance & 0xff, (distance >> 8) & 0xff,
        (distance >> 16) & 0xff, (distance >> 24) & 0xff);
    }
  }

  int size = generate->ptr - ptr_save;
  generate->ptr = ptr_save;

  memcpy(code, generate->code + generate->ptr, size);
  generate_insert(generate, offset_insert, size);
  memcpy(generate->code + offset_insert, code, size);

  return 0;
}

int generate_string_const_add(struct _generate *generate, int offset)
{
  // Not used in x86_64 right now.
  return 0;
}

int generate_finish(struct _generate *generate)
{
  // mov rbx, [rsp-8]: 0x48 0x8b 0x5c 0x24 0xf8
  generate_code(generate, 5, 0x48, 0x8b, 0x5c, 0x24, 0xf8);

#ifdef WINDOWS
  // mov rsi, [rsp-0x18]: 0x48,0x8b,0x74,0x24,0xe8
  generate_code(generate, 5, 0x48, 0x8b, 0x74, 0x24, 0xe8);

  // mov rdi, [rsp-0x20]: 0x48,0x8b,0x7c,0x24,0xe0
  generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xe0);
#endif

  // ret: 0xc3
  generate_code(generate, 1, 0xc3);

  if (generate->use_strncmp == 1)
  {
    uint8_t *strings = generate->code + generate->ptr;
    uint32_t index = 2;

#ifdef WINDOWS
    index += 5;
#endif

    memcpy(strings, generate->strings, generate->strings_ptr);

    generate->code[index+0] = ((uint64_t)strings) & 0xff;
    generate->code[index+1] = (((uint64_t)strings) >> 8) & 0xff;
    generate->code[index+2] = (((uint64_t)strings) >> 16) & 0xff;
    generate->code[index+3] = (((uint64_t)strings) >> 24) & 0xff;

    generate->code[index+4] = (((uint64_t)strings) >> 32) & 0xff;
    generate->code[index+5] = (((uint64_t)strings) >> 40) & 0xff;
    generate->code[index+6] = (((uint64_t)strings) >> 48) & 0xff;
    generate->code[index+7] = (((uint64_t)strings) >> 56) & 0xff;

    generate->ptr += generate->strings_ptr;
  }

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

static int generate_save_rdi(struct _generate *generate)
{
  if (generate->dest_reg_saved == 0)
  {
    // mov [rsp-16], rdi: 0x48 0x89 0x7c 0x24 0xf0
    generate_code(generate, 5, 0x48, 0x89, 0x7c, 0x24, 0xf0);

    generate->dest_reg_saved = 1;
  }

  return 0;
}

static int generate_mov_rdi_end(struct _generate *generate, int len)
{
  // Move rdi to the end of the string.

  generate_save_rdi(generate);

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

static int generate_test_reg(struct _generate *generate, int reg)
{
  int ptr_start = generate->ptr;

  switch(reg)
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
      return 0;
  }

  return generate->ptr - ptr_start;
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

static int generate_strncmp(struct _generate *generate, char *match, int len, int not, int type)
{
  int label;
  int distance;

  if (type == TYPE_STRCMP) { len++; }

  if (generate->need_cld == 1)
  {
    // cld: 0xfc
    generate_code(generate, 1, 0xfc);
  }

  generate_set_reg(generate, not ^ 1);

  // mov ecx, 1: 0xb9,0x01,0x00,0x00,0x00
  generate_code(generate, 5, 0xb9, len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);

  // repz cmpsb: 0xf3,0xa6
  generate_code(generate, 2, 0xf3, 0xa6);

  // jnz label: 0x75 label
  //generate_code(generate, 2, 0x75, 0);

   // jz exit: 0x74 0x05
  generate_code(generate, 2, 0x74, 0x00);

  label = generate->ptr;

  // Set value for didn't find match
  generate_set_reg(generate, not);

  distance = generate->ptr - label;
  generate->code[label - 1] = distance;

  // add rsi, rcx: 0x48,0x01,0xce
  generate_code(generate, 3, 0x48, 0x01, 0xce);

  // FIXME - This could be optimized by not storing the NULL terminator.
  if (type == TYPE_STRNCMP)
  {
    // inc esi: 0xff,0xc6
    generate_code(generate, 2, 0xff, 0xc6);
  }

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

