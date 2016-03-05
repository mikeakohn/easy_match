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

static int generate_strlen(struct _generate *generate, int len);
static int generate_code(struct _generate *generate, uint8_t opcodes, ...);

int generate_init(struct _generate *generate, uint8_t *code)
{
  generate->code = code;
  generate->ptr = 0;
  generate->reg = 0;

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
  int n;

  generate_strlen(generate, len);

  n = 0;
  while (n < len)
  {
    if ((len - n) >= 8)
    {
      // mov rbx, 0x8877665544332211: 0x48 0xbb 0x11 ...
      generate_code(generate, 8, 0x48, 0xbb,
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
    generate_code(generate, 2, 0x74, 0x01);

    // ret: 0xc3
    generate_code(generate, 1, 0xc3);
  }

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
  // inc eax: 0xff 0xc0
  generate_code(generate, 2, 0xff, 0xc0);

  // ret: 0xc3
  generate_code(generate, 1, 0xc3);

#if 0
  FILE *out = fopen("/tmp/debug.bin", "wb");
  fwrite(generate->code, 1, generate->ptr, out);
  fclose(out);
#endif

  return 0;
}

static int generate_strlen(struct _generate *generate, int len)
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

  // jge skip_exit: 0x73 0x03
  generate_code(generate, 2, 0x7d, 0x01);

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


