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

#endif

