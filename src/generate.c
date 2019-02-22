/**
 *  Easy Match
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2016-2019 by Michael Kohn
 *
 */

#include "generate.h"

int generate_code(struct _generate *generate, uint8_t len, ...)
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

int generate_insert(struct _generate *generate, int offset, int len)
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


