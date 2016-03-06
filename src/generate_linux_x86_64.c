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

#define STRLEN_FITS 0
#define STRLEN_EQUALS 1
#define MATCH_OPTION_NONE 0
#define MATCH_OPTION_RESTORE_RDI 1

static int generate_strlen(struct _generate *generate, int len, int equals);
static int generate_code(struct _generate *generate, uint8_t opcodes, ...);
static int generate_match(struct _generate *generate, char *match, int len, int option);

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

  generate_strlen(generate, len, STRLEN_FITS);
  generate_match(generate, match, len, MATCH_OPTION_NONE);

  return 0;
}

int generate_endswith(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len, STRLEN_FITS);

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

  generate_match(generate, match, len, MATCH_OPTION_RESTORE_RDI);

  // Restore rdi.
  // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
  generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);

  return 0;
}

int generate_equals(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len, STRLEN_EQUALS);
  generate_match(generate, match, len, MATCH_OPTION_NONE);

  return 0;
}

int generate_contains(struct _generate *generate, char *match)
{
  int len = strlen(match);

  generate_strlen(generate, len, STRLEN_FITS);

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

  if (equals == STRLEN_FITS)
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

static int generate_match(struct _generate *generate, char *match, int len, int option)
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

    // je skip_exit: 0x73 0x01
    generate_code(generate, 2, 0x74, option == MATCH_OPTION_NONE ? 0x06 : 0x0b);

    // mov rbx, [rsp-8]: 0x48 0x8b 0x5c 0x24 0xf8
    generate_code(generate, 5, 0x48, 0x8b, 0x5c, 0x24, 0xf8);

    if (option)
    {
      // mov rdi, [rsp-16]: 0x48 0x8b 0x7c 0x24 0xf0
      generate_code(generate, 5, 0x48, 0x8b, 0x7c, 0x24, 0xf0);
    }

    // ret: 0xc3
    generate_code(generate, 1, 0xc3);
  }

  return 0;
}

