#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef DEBUG	//Verbose is not an option
#define _VERBOSE_TEST false
#define _VERBOSE_OPT_CHECK
#define _VERBOSE_OPT_SHOW ""
#define _VERBOSE_OPT_INFO ""
#else			//Verbose working
#define _VERBOSE_TEST ( (true) == (is_verbose()) )
#define _VERBOSE_OPT_CHECK verbose( (true) ); break;
#define _VERBOSE_OPT_SHOW "[-v]"
#define _VERBOSE_OPT_INFO "\t-v\t\t[verbose]\n"
#endif

#define KNRM  "\x1B[0m"		//Terminal color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"
#define STRING_SIZE 512
#define RESPONSE_SIZE 512

#define SERVER_TEST_TIME 3

typedef void *item;

/* UTILS */
void memory_error(char *msg);
void verbose(bool verbosity);
bool is_verbose();

