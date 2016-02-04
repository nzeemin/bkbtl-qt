// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _MSC_VER
//NOTE: I know, we use unsafe string copy functions
#define _CRT_SECURE_NO_WARNINGS
#endif

// C RunTime Header Files
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _MSC_VER
#include <stdlib.h>
#endif

#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include <memory.h>

#ifndef __GNUC__
#include <tchar.h>
#endif

#include "Common.h"
