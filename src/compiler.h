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

#ifndef _COMPILER_H
#define _COMPILER_H

#define OPTION_NONE 0
#define OPTION_WITH_LEN 1

typedef int (*match_t)(char*);
typedef int (*match_with_len_t)(char*, int);

void *compiler_generate(char *code, int option);
void compiler_free(void *function);

// Compatibility macros for Windows
#ifdef WINDOWS
#define PROT_EXEC PAGE_EXECUTE
#define PROT_WRITE PAGE_EXECUTE_READWRITE
#define PROT_READ 0
#define MAP_FAILED NULL

#define mmap(addr, length, prot, flags, fd, offset) \
  VirtualAlloc(addr, length, MEM_RESERVE | MEM_COMMIT, prot);
#define mprotect(addr, len, prot) VirtualProtect(addr, len, prot, NULL)
#define munmap(addr, len) VirtualFree(addr, 0, MEM_RELEASE)
#define alloca _alloca
#endif

#endif

