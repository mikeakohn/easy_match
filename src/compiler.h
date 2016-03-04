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

typedef int (*match_t)(char*);

match_t compiler_generate(char *code);
void compiler_free(match_t function);

#endif

