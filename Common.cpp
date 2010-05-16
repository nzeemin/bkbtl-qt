// Common.cpp

#include "stdafx.h"
#include <QMessageBox>
#include <QFont>
#include <QPainter>


//////////////////////////////////////////////////////////////////////


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

void DebugPrint(LPCTSTR message)
{
    //TODO: Implement in this environment
}

void DebugPrintFormat(LPCTSTR pszFormat, ...)
{
    //TODO: Implement in this environment
}

const LPCTSTR TRACELOG_FILE_NAME = _T("trace.log");
const LPCTSTR TRACELOG_NEWLINE = _T("\r\n");

HANDLE Common_LogFile = NULL;

void DebugLog(LPCTSTR message)
{
    //TODO: Implement in this environment
}

void DebugLogFormat(LPCTSTR pszFormat, ...)
{
    //TODO: Implement in this environment
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
        TCHAR ch = text.at(p).toAscii();
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


//////////////////////////////////////////////////////////////////////
