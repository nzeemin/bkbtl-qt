// Common.cpp

#include "stdafx.h"
#include <QMessageBox>
#include <QFont>
#include <QPainter>
#include <QCoreApplication>


//////////////////////////////////////////////////////////////////////


BOOL AssertFailedLine(const char * lpszFileName, int nLine)
{
    TCHAR buffer[360];
    _sntprintf(buffer, 360,
#ifdef _UNICODE
            _T("ASSERTION FAILED\n\nFile: %S\nLine: %d\n\n")
#else
            _T("ASSERTION FAILED\n\nFile: %s\nLine: %d\n\n")
#endif
            _T("Press Abort to stop the program, Retry to break to the debugger, or Ignore to continue execution."),
            lpszFileName, nLine);
    int result = QMessageBox::question(NULL, _T("BK Back to Life"), buffer, QMessageBox::Abort, QMessageBox::Retry, QMessageBox::Ignore);
    switch (result)
    {
        case QMessageBox::Retry:
            return TRUE;
        case QMessageBox::Ignore:
            return FALSE;
        case QMessageBox::Abort:
            QCoreApplication::exit(255);
    }
    return FALSE;
}

void AlertWarning(LPCTSTR sMessage)
{
    QMessageBox::warning(NULL, _T("BK Back to Life"), sMessage, QMessageBox::Ok);
}

BOOL AlertOkCancel(LPCTSTR sMessage)
{
    int result = QMessageBox::question(NULL, _T("BK Back to Life"), sMessage, QMessageBox::Ok, QMessageBox::Cancel);
    return (result == QMessageBox::Ok);
}


//////////////////////////////////////////////////////////////////////
// DebugPrint and DebugLog

#if !defined(PRODUCT)

void DebugPrint(LPCTSTR /*message*/)
{
    //TODO: Implement in this environment
}

void DebugPrintFormat(LPCTSTR pszFormat, ...)
{
    TCHAR buffer[512];

    va_list ptr;
    va_start(ptr, pszFormat);
    _sntprintf(buffer, 512, pszFormat, ptr);
    va_end(ptr);

    DebugPrint(buffer);
}

const LPCTSTR TRACELOG_FILE_NAME = _T("trace.log");
const LPCTSTR TRACELOG_NEWLINE = _T("\r\n");

FILE* Common_LogFile = NULL;

void DebugLog(LPCTSTR message)
{
    if (Common_LogFile == NULL)
    {
        Common_LogFile = ::_tfopen(TRACELOG_FILE_NAME, _T("a+b"));
    }

    ::fseek(Common_LogFile, 0, SEEK_END);

    size_t dwLength = strlen(message) * sizeof(TCHAR);
    ::fwrite(message, 1, dwLength, Common_LogFile);
}

void DebugLogFormat(LPCTSTR pszFormat, ...)
{
    TCHAR buffer[512];

    va_list ptr;
    va_start(ptr, pszFormat);
    _sntprintf(buffer, 512, pszFormat, ptr);
    va_end(ptr);

    DebugLog(buffer);
}


#endif // !defined(PRODUCT)


//////////////////////////////////////////////////////////////////////


static QFont* g_MonospacedFont = NULL;

QFont Common_GetMonospacedFont()
{
    if (g_MonospacedFont != NULL)
        return *g_MonospacedFont;

    g_MonospacedFont = new QFont(_T("Lucida Console"), 9, QFont::Normal, false);
    g_MonospacedFont->setFixedPitch(true);

    return *g_MonospacedFont;
}

void Common_Cleanup()
{
    if (g_MonospacedFont != NULL)
        delete g_MonospacedFont;
}


// Print octal 16-bit value to buffer
// buffer size at least 7 characters
void PrintOctalValue(TCHAR* buffer, WORD value)
{
    for (int p = 0; p < 6; p++) {
        int digit = value & 7;
        buffer[5 - p] = _T('0') + digit;
        value = (value >> 3);
    }
    buffer[6] = 0;
}
// Print hex 16-bit value to buffer
// buffer size at least 5 characters
void PrintHexValue(TCHAR* buffer, WORD value)
{
    for (int p = 0; p < 4; p++) {
        int digit = value & 15;
        buffer[3 - p] = (digit < 10) ? _T('0') + (TCHAR)digit : _T('a') + (TCHAR)(digit - 10);
        value = (value >> 4);
    }
    buffer[4] = 0;
}
// Print binary 16-bit value to buffer
// buffer size at least 17 characters
void PrintBinaryValue(TCHAR* buffer, WORD value)
{
    for (int b = 0; b < 16; b++) {
        int bit = (value >> b) & 1;
        buffer[15 - b] = bit ? _T('1') : _T('0');
    }
    buffer[16] = 0;
}

void DrawOctalValue(QPainter &painter, int x, int y, WORD value)
{
    TCHAR buffer[7];
    PrintOctalValue(buffer, value);
    painter.drawText(x, y, buffer);
}
void DrawHexValue(QPainter &painter, int x, int y, WORD value)
{
    TCHAR buffer[7];
    PrintHexValue(buffer, value);
    painter.drawText(x, y, buffer);
}
void DrawBinaryValue(QPainter &painter, int x, int y, WORD value)
{
    TCHAR buffer[17];
    PrintBinaryValue(buffer, value);
    painter.drawText(x, y, buffer);
}

// Parse octal value from text
BOOL ParseOctalValue(LPCTSTR text, WORD* pValue)
{
    WORD value = 0;
    TCHAR* pChar = (TCHAR*) text;
    for (int p = 0; ; p++) {
        if (p > 6) return FALSE;
        TCHAR ch = *pChar;  pChar++;
        if (ch == 0) break;
        if (ch < _T('0') || ch > _T('7')) return FALSE;
        value = (value << 3);
        int digit = ch - _T('0');
        value += digit;
    }
    *pValue = value;
    return TRUE;
}

// Parse octal value from text
BOOL ParseOctalValue(const QString &text, WORD* pValue)
{
    WORD value = 0;
    for (int p = 0; p < text.length(); p++) {
        if (p > 6) return FALSE;
#ifdef	_UNICODE
        TCHAR ch = text.at(p).unicode();
#else
        TCHAR ch = text.at(p).toLatin1();
#endif
        if (ch == 0) break;
        if (ch < _T('0') || ch > _T('7')) return FALSE;
        value = (value << 3);
        int digit = ch - _T('0');
        value += digit;
    }
    *pValue = value;
    return TRUE;
}

// BK to Unicode conversion table
const WORD BK_CHAR_CODES[] = {
    0x3C0,  0x2534, 0x2665, 0x2510, 0x2561, 0x251C, 0x2514, 0x2550, 0x2564, 0x2660, 0x250C, 0x252C, 0x2568, 0x2193, 0x253C, 0x2551, 
    0x2524, 0x2190, 0x256C, 0x2191, 0x2663, 0x2500, 0x256B, 0x2502, 0x2666, 0x2518, 0x256A, 0x2565, 0x2567, 0x255E, 0x2192, 0x2593, 
    0x44E,  0x430,  0x431,  0x446,  0x434,  0x435,  0x444,  0x433,  0x445,  0x438,  0x439,  0x43A,  0x43B,  0x43C,  0x43D,  0x43E, 
    0x43F,  0x44F,  0x440,  0x441,  0x442,  0x443,  0x436,  0x432,  0x44C,  0x44B,  0x437,  0x448,  0x44D,  0x449,  0x447,  0x44A, 
    0x42E,  0x410,  0x411,  0x426,  0x414,  0x415,  0x424,  0x413,  0x425,  0x418,  0x419,  0x41A,  0x41B,  0x41C,  0x41D,  0x41E, 
    0x41F,  0x42F,  0x420,  0x421,  0x422,  0x423,  0x416,  0x412,  0x42C,  0x42B,  0x417,  0x428,  0x42D,  0x429,  0x427,  0x42A, 
};
// Translate one KOI8-R character to Unicode character
WORD Translate_BK_Unicode(BYTE ch)
{
    if (ch < 32) return 0x00b7;
    if (ch < 127) return (WORD) ch;
    if (ch == 127) return (WORD) 0x25A0;
    if (ch >= 128 && ch < 160) return 0x00b7;
    return BK_CHAR_CODES[ch - 160];
}


//////////////////////////////////////////////////////////////////////
