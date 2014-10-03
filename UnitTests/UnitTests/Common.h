
#ifndef _UT_COMMON_H
#define _UT_COMMON_H

#define _CRT_RAND_S
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <math.h>
#include "Assert.h"

#define CHOOSE_TEST_OUTCOME(flag)   ((flag) ? g_pszPassed : g_pszFailed)

#endif // _UT_COMMON_H
