// Common.cpp

#include "stdafx.h"
#include <QClipboard>
#include <QMessageBox>
#include <QGuiApplication>
#include <QFont>
#include <QPainter>
#include <QCoreApplication>
#include "main.h"
#include "mainwindow.h"


//////////////////////////////////////////////////////////////////////


bool AssertFailedLine(const char * lpszFileName, int nLine)
{
    char buffer[360];
    _snprintf(buffer, 360,
            QT_TRANSLATE_NOOP("Common",
                    "ASSERTION FAILED\n\nFile: %s\nLine: %d\n\n"
                    "Press Abort to stop the program, Retry to break to the debugger, or Ignore to continue execution."),
            lpszFileName, nLine);
    int result = QMessageBox::question(nullptr, QT_TRANSLATE_NOOP("Common", "BK Back to Life"),
            buffer, QMessageBox::Abort, QMessageBox::Retry, QMessageBox::Ignore);
    switch (result)
    {
    case QMessageBox::Retry:
        return true;
    case QMessageBox::Ignore:
        return false;
    case QMessageBox::Abort:
        QCoreApplication::exit(255);
    }
    return false;
}

void AlertInfo(const QString &sMessage)
{
    QMessageBox::information(nullptr, QT_TRANSLATE_NOOP("Common", "BK Back to Life"),
            sMessage, QMessageBox::Ok);
}

void AlertWarning(const QString &sMessage)
{
    QMessageBox::warning(nullptr, QT_TRANSLATE_NOOP("Common", "BK Back to Life"),
            sMessage, QMessageBox::Ok);
}

bool AlertOkCancel(const QString &sMessage)
{
    int result = QMessageBox::question(nullptr, QT_TRANSLATE_NOOP("Common", "BK Back to Life"),
            sMessage, QMessageBox::Ok, QMessageBox::Cancel);
    return (result == QMessageBox::Ok);
}


//////////////////////////////////////////////////////////////////////
// DebugPrint and DebugLog

#if !defined(PRODUCT)

void DebugPrint(const char* message)
{
    MainWindow* mainWindow = Global_getMainWindow();
    if (mainWindow != nullptr)
        mainWindow->consolePrint(message);
}

void DebugPrintFormat(const char* pszFormat, ...)
{
    char buffer[512];

    va_list ptr;
    va_start(ptr, pszFormat);
    vsnprintf(buffer, 512, pszFormat, ptr);
    va_end(ptr);

    DebugPrint(buffer);
}

const char* TRACELOG_FILE_NAME = "trace.log";
const char* TRACELOG_NEWLINE = "\r\n";

FILE* Common_LogFile = nullptr;

void DebugLog(const char* message)
{
    if (Common_LogFile == nullptr)
    {
        Common_LogFile = ::fopen(TRACELOG_FILE_NAME, "a+b");
        //TODO: Check if Common_LogFile == nullptr
    }

    ::fseek(Common_LogFile, 0, SEEK_END);

    size_t dwLength = strlen(message) * sizeof(char);
    ::fwrite(message, 1, dwLength, Common_LogFile);
}

void DebugLogFormat(const char* pszFormat, ...)
{
    char buffer[512];

    va_list ptr;
    va_start(ptr, pszFormat);
    vsnprintf(buffer, 512, pszFormat, ptr);
    va_end(ptr);

    DebugLog(buffer);
}


#endif // !defined(PRODUCT)


//////////////////////////////////////////////////////////////////////


static QFont* g_MonospacedFont = nullptr;

QFont Common_GetMonospacedFont()
{
    if (g_MonospacedFont != nullptr)
        return *g_MonospacedFont;

#ifdef __APPLE__
    const char* fontName = "Monaco";
    int fontSize = 10;
#else
    const char* fontName = "Lucida Console";
    int fontSize = 9;
#endif
    g_MonospacedFont = new QFont(fontName, fontSize, QFont::Normal, false);
    g_MonospacedFont->setFixedPitch(true);
    g_MonospacedFont->setStyleHint(QFont::TypeWriter);
    g_MonospacedFont->setHintingPreference(QFont::PreferNoHinting);

    return *g_MonospacedFont;
}

void Common_Cleanup()
{
    if (g_MonospacedFont != nullptr)
        delete g_MonospacedFont;
}

QColor Common_GetColorShifted(const QPalette& palette, QRgb rgb)
{
    QColor colorText = palette.color(QPalette::Text);
    QColor color((colorText.red() + qRed(rgb)) / 2, (colorText.green() + qGreen(rgb)) / 2, (colorText.blue() + qBlue(rgb)) / 2);
    return color;
}


// Print octal 16-bit value to buffer
// buffer size at least 7 characters
void PrintOctalValue(char* buffer, quint16 value)
{
    for (int p = 0; p < 6; p++)
    {
        int digit = value & 7;
        buffer[5 - p] = '0' + digit;
        value = (value >> 3);
    }
    buffer[6] = 0;
}
// Print hex 16-bit value to buffer
// buffer size at least 5 characters
void PrintHexValue(char* buffer, quint16 value)
{
    for (int p = 0; p < 4; p++)
    {
        int digit = value & 15;
        buffer[3 - p] = (digit < 10) ? '0' + (char)digit : 'a' + (char)(digit - 10);
        value = (value >> 4);
    }
    buffer[4] = 0;
}
// Print binary 16-bit value to buffer
// buffer size at least 17 characters
void PrintBinaryValue(char * buffer, quint16 value)
{
    for (int b = 0; b < 16; b++)
    {
        int bit = (value >> b) & 1;
        buffer[15 - b] = bit ? '1' : '0';
    }
    buffer[16] = 0;
}

void DrawOctalValue(QPainter &painter, int x, int y, quint16 value)
{
    char buffer[7];
    PrintOctalValue(buffer, value);
    painter.drawText(x, y, buffer);
}
void DrawHexValue(QPainter &painter, int x, int y, quint16 value)
{
    char buffer[7];
    PrintHexValue(buffer, value);
    painter.drawText(x, y, buffer);
}
void DrawBinaryValue(QPainter &painter, int x, int y, quint16 value)
{
    char buffer[17];
    PrintBinaryValue(buffer, value);
    painter.drawText(x, y, buffer);
}

// Parse 16-bit octal value from text
bool ParseOctalValue(const char* text, quint16* pValue)
{
    quint16 value = 0;
    char* pChar = (char*) text;
    for (int p = 0; ; p++)
    {
        if (p > 6) return false;
        char ch = *pChar;  pChar++;
        if (ch == 0) break;
        if (ch < '0' || ch > '7') return false;
        value = (value << 3);
        int digit = ch - '0';
        value += digit;
    }
    *pValue = value;
    return true;
}

// Parse 16-bit octal value from text
bool ParseOctalValue(const QString &text, quint16* pValue)
{
    quint16 value = 0;
    for (int p = 0; p < text.length(); p++)
    {
        if (p > 6) return false;
        char ch = text.at(p).toLatin1();
        if (ch == 0) break;
        if (ch < '0' || ch > '7') return false;
        value = (value << 3);
        int digit = ch - '0';
        value += digit;
    }
    *pValue = value;
    return true;
}

// Parse 16-bit hex value from text
bool ParseHexValue(const char* text, quint16* pValue)
{
    quint16 value = 0;
    char* pChar = (char*) text;
    for (int p = 0; ; p++)
    {
        if (p > 4) return false;
        char ch = *pChar;  pChar++;
        if (ch == 0) break;
        if (ch >= '0' && ch <= '9')
        {
            value = (value << 4);
            int digit = ch - '0';
            value += digit;
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            value = (value << 4);
            int digit = ch - 'a' + 10;
            value += digit;
        }
        else if (ch >= 'A' && ch <= 'F')
        {
            value = (value << 4);
            int digit = ch - 'A' + 10;
            value += digit;
        }
        else
            return false;
    }
    *pValue = value;
    return true;
}

void CopyTextToClipboard(const char* text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
    clipboard->setText(text);
}
void CopyWordOctalToClipboard(uint16_t value)
{
    char buffer[7];
    PrintOctalValue(buffer, value);
    CopyTextToClipboard(buffer);
}
void CopyWordHexToClipboard(uint16_t value)
{
    char bufferHex[5];
    PrintHexValue(bufferHex, value);
    CopyTextToClipboard(bufferHex);
}
void CopyWordBinaryToClipboard(uint16_t value)
{
    char bufferBin[17];
    PrintBinaryValue(bufferBin, value);
    CopyTextToClipboard(bufferBin);
}


// BK to Unicode conversion table
const quint16 BK_CHAR_CODES[] =
{
    0x3C0,  0x2534, 0x2665, 0x2510, 0x2561, 0x251C, 0x2514, 0x2550, 0x2564, 0x2660, 0x250C, 0x252C, 0x2568, 0x2193, 0x253C, 0x2551,
    0x2524, 0x2190, 0x256C, 0x2191, 0x2663, 0x2500, 0x256B, 0x2502, 0x2666, 0x2518, 0x256A, 0x2565, 0x2567, 0x255E, 0x2192, 0x2593,
    0x44E,  0x430,  0x431,  0x446,  0x434,  0x435,  0x444,  0x433,  0x445,  0x438,  0x439,  0x43A,  0x43B,  0x43C,  0x43D,  0x43E,
    0x43F,  0x44F,  0x440,  0x441,  0x442,  0x443,  0x436,  0x432,  0x44C,  0x44B,  0x437,  0x448,  0x44D,  0x449,  0x447,  0x44A,
    0x42E,  0x410,  0x411,  0x426,  0x414,  0x415,  0x424,  0x413,  0x425,  0x418,  0x419,  0x41A,  0x41B,  0x41C,  0x41D,  0x41E,
    0x41F,  0x42F,  0x420,  0x421,  0x422,  0x423,  0x416,  0x412,  0x42C,  0x42B,  0x417,  0x428,  0x42D,  0x429,  0x427,  0x42A,
};
// Translate one KOI8-R character to Unicode character
quint16 Translate_BK_Unicode(quint8 ch)
{
    if (ch < 32) return 0x00b7;
    if (ch < 127) return (quint16) ch;
    if (ch == 127) return (quint16) 0x25A0;
    if (ch >= 128 && ch < 160) return 0x00b7;
    return BK_CHAR_CODES[ch - 160];
}


//////////////////////////////////////////////////////////////////////
