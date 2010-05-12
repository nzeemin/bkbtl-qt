// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


// C RunTime Header Files
#include <string.h>
#include <stdio.h>

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
