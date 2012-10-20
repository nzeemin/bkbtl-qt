#ifndef COMMON_H
#define COMMON_H
// Common.h

#pragma once

#include <QtGlobal>

class QFont;
class QPainter;
class QString;


//////////////////////////////////////////////////////////////////////
// Defines for compilation under MinGW and GCC

#ifndef _TCHAR_DEFINED
#ifdef	_UNICODE
typedef wchar_t TCHAR;
#define _tfopen     _wfopen
#define _tfsopen    _wfsopen
#define _tcscpy     wcscpy
#define _tstat      _wstat
#define _tcsrchr    wcsrchr
#define _tcsicmp    _wcsicmp
#define _sntprintf  _snwprintf
#else
typedef char TCHAR;
#define _tfopen     fopen
#define _tfsopen    _fsopen
#define _tcscpy     strcpy
#define _tstat      _stat
#define _tcsrchr    strrchr
#define _tcsicmp    _stricmp
#define _sntprintf  _snprintf
#endif
#define _T(x)       x
#endif

#ifdef __GNUC__
//#define _stat       stat
#define _stricmp    strcasecmp
#define _snprintf   snprintf
#endif

#ifdef	_UNICODE
typedef const wchar_t * LPCTSTR;
#else
typedef const char * LPCTSTR;
#endif

//typedef qint32 LONG;
typedef quint32 DWORD;
typedef quint16 WORD;
typedef quint8 BYTE;

typedef int BOOL;
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

//#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define LOWORD(l)           ((WORD)(((DWORD)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD)(w)) >> 8) & 0xff))

#ifdef __GNUC__
 #ifdef __APPLE__
  #define CALLBACK
 #else
  #define CALLBACK __attribute__((stdcall))
 #endif
#else
 #define CALLBACK __stdcall
#endif

typedef void *HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(qint32)-1)
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name


//////////////////////////////////////////////////////////////////////
// Assertions checking - MFC-like ASSERT macro

#ifdef _DEBUG

BOOL AssertFailedLine(const char * lpszFileName, int nLine);
#define ASSERT(f)          (void) ((f) || !AssertFailedLine(__FILE__, __LINE__) || (__debugbreak(), 0))
#define VERIFY(f)          ASSERT(f)

#else   // _DEBUG

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)f)

#endif // !_DEBUG


//////////////////////////////////////////////////////////////////////


void AlertWarning(LPCTSTR sMessage);
BOOL AlertOkCancel(LPCTSTR sMessage);


//////////////////////////////////////////////////////////////////////
// DebugPrint

#if !defined(PRODUCT)

void DebugPrint(LPCTSTR message);
void DebugPrintFormat(LPCTSTR pszFormat, ...);
void DebugLog(LPCTSTR message);
void DebugLogFormat(LPCTSTR pszFormat, ...);

#endif // !defined(PRODUCT)


//////////////////////////////////////////////////////////////////////


// Processor register names
const LPCTSTR REGISTER_NAME[] = { _T("R0"), _T("R1"), _T("R2"), _T("R3"), _T("R4"), _T("R5"), _T("SP"), _T("PC") };

const int BK_SCREEN_WIDTH = 512;
const int BK_SCREEN_HEIGHT = 256;

QFont Common_GetMonospacedFont();
void Common_Cleanup();
void PrintOctalValue(TCHAR* buffer, WORD value);
void PrintBinaryValue(TCHAR* buffer, WORD value);
void DrawOctalValue(QPainter &painter, int x, int y, WORD value);
void DrawBinaryValue(QPainter &painter, int x, int y, WORD value);
BOOL ParseOctalValue(LPCTSTR text, WORD* pValue);
BOOL ParseOctalValue(const QString &text, WORD* pValue);

WORD Translate_BK_Unicode(BYTE ch);


//////////////////////////////////////////////////////////////////////
#endif // COMMON_H
