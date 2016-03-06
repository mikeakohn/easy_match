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

static int generate_strlen(struct _generate *generate, int len, int equals);
static int generate_code(struct _generate *generate, uint8_t opcodes, ...);
static int generate_match(struct _generate *generate, char *match, int len, int restore_rdi);

int generate_init(struct _generate *generate, uint8_t *code)
{
  generate->code = code;
  generate->ptr = 0;
  generate->reg = 0;

  // mov [rsp-8], rbx: 0x48 0x89 0x5c 0x24 0xf8
  generate_code(generate, 5, 0x48, 0x89, 0x5c, 0x24, 0xf8);

  // xor eax, eax: 0x31  0xC0
  generate_code(generate, 2, 0x31, 0xc0);

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

  return 0;
}

int generate_not(struct _generate *generate)
{
  // xor eax, 1: 0x83 0xf0 0x01
  generate_code(generate, 3, 0x83, 0xf0, 0x01);

  return 0;
}

int generate_startswith(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len, 0);
  generate_match(generate, match, len, 0);

  return 0;
}

int generate_endswith(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len, 0);

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

  generate_match(generate, match, len, 1);

  // Restore rdi.
  // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);

  return 0;
}

int generate_equals(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len, 1);
  generate_match(generate, match, len, 0);

  return 0;
}

int generate_contains(struct _generate *generate, char *match)
{
  return -1;
}

int generate_finish(struct _generate *generate)
{
  // inc eax: 0xff 0xc0
  generate_code(generate, 2, 0xff, 0xc0);

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

static int generate_strlen(struct _generate *generate, int len, int equals)
{
  if (len < 128)
  {
    // cmp rsi, 4: 0x48 0x83 0xfe 0x04
    generate_code(generate, 4, 0x48, 0x83, 0xfe, len);
  }
    else
  {
  // cmp rsi, 200: 0x48 0x81 0xfe 0xc8 0x00 0x00 0x00
  generate_code(generate, 7, 0x48, 0x81, 0xfe, 
    len & 0xff, (len >> 8) & 0xff, (len >> 16) & 0xff, (len >> 24) & 0xff);
  }

  if (equals == 0)
  {
    // jge skip_exit: 0x7d 0x03
    generate_code(generate, 2, 0x7d, 0x06);
  }
    else
  {
    // je skip_exit: 0x74 0x03
    generate_code(generate, 2, 0x74, 0x06);
  }

  // mov rbx, [rsp-8]: 0x48 0x8b 0x5c 0x24 0xf8
  generate_code(generate, 5, 0x48, 0x8b, 0x5c, 0x24, 0xf8);

  // ret: 0xc3
  generate_code(generate, 1, 0xc3);

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

static int generate_match(struct _generate *generate, char *match, int len, int restore_rdi)
{
  int n;

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
      // mov ebx, 0x12345678: 0xbb 0x78 0x56 0x34 0x12
      generate_code(generate, 5, 0xbb,
        match[n+0], match[n+1], match[n+2], match[n+3]);

      if (n == 0)
      {
        // mov edx, [rdi]: 0x8b 0x17
        generate_code(generate, 2, 0x8b, 0x17);
      }
        else
      if (n < 128)
      {
        // mov edx, [rdi+1]: 0x8b 0x57 0x01
        generate_code(generate, 3, 0x8b, 0x57, n);
      }
        else
      {
        // mov edx, [rdi+128]: 0x8b 0x97 0x80 0x00 0x00 0x00
        generate_code(generate, 6, 0x8b, 0x97,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff);
      }

      // cmp edx, ebx: 0x39 0xda
      generate_code(generate, 2, 0x39, 0xda);

      n += 4;
    }
      else
    if ((len - n) >= 2)
    {
      // mov bx, 0x1234: 0x66 0xbb 0x34 0x12
      generate_code(generate, 4, 0x66, 0xbb, match[n+0], match[n+1]);

      if (n == 0)
      {
        // mov dx, [rdi]: 0x66 0x8b 0x17
        generate_code(generate, 3, 0x66, 0x8b, 0x17);
      }
        else
      if (n < 128)
      {
        // mov dx, [rdi+1]: 0x66 0x8b 0x57 0x01
        generate_code(generate, 4, 0x66, 0x8b, 0x57, n);
      }
        else
      {
        // mov dx, [rdi+128]: 0x66 0x8b 0x97 0x80 0x00 0x00 0x00
        generate_code(generate, 7, 0x66, 0x8b, 0x97,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff);
      }

      // cmp dx, bx: 0x66 0x39 0xda
      generate_code(generate, 3, 0x66, 0x39, 0xda);

      n += 2;
    }
      else
    {
      // mov bl, 3: 0xb3 0x03
      generate_code(generate, 2, 0xb3, match[n+0]);

      if (n == 0)
      {
        // mov dl, [rdi]: 0x8a 0x17
        generate_code(generate, 2, 0x8a, 0x17);
      }
        else
      if (n < 128)
      {
        // mov dl, [rdi+1]: 0x8a 0x57 0x01
        generate_code(generate, 3, 0x8a, 0x57, n);
      }
        else
      {
        // mov dl, [rdi+128]: 0x8a 0x97 0x80 0x00 0x00 0x00
        generate_code(generate, 6, 0x8a, 0x97,
          n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff, (n >> 24) & 0xff);
      }

      // cmp dl, bl: 0x38 0xda
      generate_code(generate, 2, 0x38, 0xda);

      n++;
    }

    // je skip_exit: 0x73 0x01
    generate_code(generate, 2, 0x74, restore_rdi == 0 ? 0x06 : 0x0b);

    // mov rbx, [rsp-8]: 0x48 0x8b 0x5c 0x24 0xf8
    generate_code(generate, 5, 0x48, 0x8b, 0x5c, 0x24, 0xf8);

    if (restore_rdi)
    {
      // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
      generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);
    }

    // ret: 0xc3
    generate_code(generate, 1, 0xc3);
  }

  return 0;
}

