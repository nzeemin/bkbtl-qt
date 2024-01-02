#ifndef COMMON_H
#define COMMON_H
// Common.h

#pragma once

#include <QtGlobal>
#include <QColor>

class QFont;
class QPainter;
class QPalette;
class QString;


//////////////////////////////////////////////////////////////////////
// Defines for compilation under MinGW and GCC

#ifndef _TCHAR_DEFINED
typedef char TCHAR;
#define _tfopen     fopen
#define _tfsopen    _fsopen
#define _tcscpy     strcpy
#define _tcscpy_s   strcpy_s
#define _tstat      _stat
#define _tcsrchr    strrchr
#define _tcsicmp    _stricmp
#define _tcscmp     strcmp
#define _tcslen     strlen
#define _sntprintf  _snprintf
#define _T(x)       x
#endif

#ifdef __GNUC__
//#define _stat       stat
#define _stricmp    strcasecmp
#define _snprintf   snprintf
#endif

typedef char * LPTSTR;
typedef const char * LPCTSTR;

#ifdef __GNUC__
#define CALLBACK
#else
#define CALLBACK __stdcall
#endif

typedef void *HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(qint32)-1)
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name


//////////////////////////////////////////////////////////////////////
// Assertions checking - MFC-like ASSERT macro

#ifdef _DEBUG

bool AssertFailedLine(const char * lpszFileName, int nLine);
#define ASSERT(f)          (void) ((f) || !AssertFailedLine(__FILE__, __LINE__) || (__debugbreak(), 0))
#define VERIFY(f)          ASSERT(f)

#else   // _DEBUG

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)f)

#endif // !_DEBUG


//////////////////////////////////////////////////////////////////////


void AlertInfo(const QString &sMessage);
void AlertWarning(const QString &sMessage);
bool AlertOkCancel(const QString &sMessage);


//////////////////////////////////////////////////////////////////////
// DebugPrint

#if !defined(PRODUCT)

void DebugPrint(const char* message);
void DebugPrintFormat(const char* pszFormat, ...);
void DebugLog(const char* message);
void DebugLogFormat(const char* pszFormat, ...);

#endif // !defined(PRODUCT)


//////////////////////////////////////////////////////////////////////


// Processor register names
const LPCTSTR REGISTER_NAME[] = { "R0", "R1", "R2", "R3", "R4", "R5", "SP", "PC" };

const int BK_SCREEN_WIDTH = 512;
const int BK_SCREEN_HEIGHT = 256;

#define COLOR_VALUECHANGED  qRgb(255,40,40)
#define COLOR_PREVIOUS      qRgb(40,40,255)
#define COLOR_MEMORYROM     qRgb(40,40,255)
#define COLOR_MEMORYIO      qRgb(128,255,128)
#define COLOR_MEMORYNA      qRgb(128,128,128)
#define COLOR_SUBTITLE      qRgb(0,128,0)
#define COLOR_VALUE         qRgb(128,128,128)
#define COLOR_VALUEROM      qRgb(128,128,192)
#define COLOR_JUMP          qRgb(80,192,224)
#define COLOR_JUMPYES       qRgb(128,255,128)
#define COLOR_JUMPNO        qRgb(192,192,192)
#define COLOR_JUMPHINT      qRgb(40,128,160)
#define COLOR_HINT          qRgb(40,40,160)

QFont Common_GetMonospacedFont();
QColor Common_GetColorShifted(const QPalette& palette, QRgb rgb);
void Common_Cleanup();
void PrintOctalValue(char* buffer, quint16 value);
void PrintHexValue(char* buffer, quint16 value);
void PrintBinaryValue(char* buffer, quint16 value);
void DrawOctalValue(QPainter &painter, int x, int y, quint16 value);
void DrawHexValue(QPainter &painter, int x, int y, quint16 value);
void DrawBinaryValue(QPainter &painter, int x, int y, quint16 value);
bool ParseOctalValue(const char* text, quint16* pValue);
bool ParseOctalValue(const QString &text, quint16* pValue);
bool ParseHexValue(const char* text, quint16* pValue);

void CopyTextToClipboard(const char* text);
void CopyWordOctalToClipboard(uint16_t value);
void CopyWordHexToClipboard(uint16_t value);
void CopyWordBinaryToClipboard(uint16_t value);

quint16 Translate_BK_Unicode(quint8 ch);


//////////////////////////////////////////////////////////////////////
#endif // COMMON_H
