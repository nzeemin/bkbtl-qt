// Common.cpp

#include "stdafx.h"
#include <QMessageBox>

//////////////////////////////////////////////////////////////////////


void AlertWarning(LPCTSTR sMessage)
{
    QMessageBox::warning(NULL, _T("BK Back to Life"), sMessage, QMessageBox::Ok);
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


//////////////////////////////////////////////////////////////////////
