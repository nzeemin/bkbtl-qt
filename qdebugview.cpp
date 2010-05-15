#include "stdafx.h"
#include <QtGui>
#include "qdebugview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


//////////////////////////////////////////////////////////////////////


QDebugView::QDebugView(QWidget *parent) :
        QWidget(parent)
{
    setMinimumSize(580, 172);
}

void QDebugView::paintEvent(QPaintEvent *event)
{
    if (g_pBoard == NULL) return;

    QPainter painter(this);
    painter.fillRect(0,0, this->width(), this->height(), Qt::white);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    CProcessor* pDebugPU = g_pBoard->GetCPU();
    ASSERT(pDebugPU != NULL);
    WORD* arrR = m_wDebugCpuR;
    BOOL* arrRChanged = m_okDebugCpuRChanged;

    painter.drawText(cxChar * 1, 2 * cyLine, _T("CPU"));

    DrawProcessor(painter, pDebugPU, cxChar * 6, 1 * cyLine, arrR, arrRChanged);

    // Draw stack
    DrawMemoryForRegister(painter, 6, pDebugPU, 35 * cxChar, 1 * cyLine);
}

void QDebugView::DrawProcessor(QPainter &painter, const CProcessor *pProc, int x, int y, WORD *arrR, BOOL *arrRChanged)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    painter.drawRect(x - cxChar, y - cyLine/2, 28 * cxChar, cyLine * 13);

    // Registers
    for (int r = 0; r < 8; r++) {
        //::SetTextColor(hdc, arrRChanged[r] ? COLOR_RED : colorText);

        LPCTSTR strRegName = REGISTER_NAME[r];
        painter.drawText(x, y + (1 + r) * cyLine, strRegName);

        WORD value = arrR[r]; //pProc->GetReg(r);
        DrawOctalValue(painter, x + cxChar * 3, y + (1 + r) * cyLine, value);
        DrawBinaryValue(painter, x + cxChar * 10, y + (1 + r) * cyLine, value);
    }
    //::SetTextColor(hdc, colorText);

    // PSW value
    //::SetTextColor(hdc, arrRChanged[8] ? COLOR_RED : colorText);
    painter.drawText(x, y + 10 * cyLine, _T("PS"));
    WORD psw = arrR[8]; // pProc->GetPSW();
    DrawOctalValue(painter, x + cxChar * 3, y + 10 * cyLine, psw);
    painter.drawText(x + cxChar * 10, y + 9 * cyLine, _T("       HP  TNZVC"));
    DrawBinaryValue(painter, x + cxChar * 10, y + 10 * cyLine, psw);

    //::SetTextColor(hdc, colorText);

    // Processor mode - HALT or USER
    BOOL okHaltMode = pProc->IsHaltMode();
    painter.drawText(x, y + 12 * cyLine, okHaltMode ? _T("HALT") : _T("USER"));

    // "Stopped" flag
    BOOL okStopped = pProc->IsStopped();
    if (okStopped)
        painter.drawText(x + 6 * cxChar, y + 12 * cyLine, _T("STOP"));
}

void QDebugView::DrawMemoryForRegister(QPainter &painter, int reg, CProcessor *pProc, int x, int y)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    WORD current = pProc->GetReg(reg);
    BOOL okExec = (reg == 7);

    // Читаем из памяти процессора в буфер
    WORD memory[16];
    for (int idx = 0; idx < 16; idx++) {
        int addrtype;
        memory[idx] = g_pBoard->GetWordView(
                current + idx * 2 - 14, pProc->IsHaltMode(), okExec, &addrtype);
    }

    WORD address = current - 10;
    for (int index = 0; index < 14; index++) {  // Рисуем строки
        // Адрес
        DrawOctalValue(painter, x + 4 * cxChar, y, address);

        // Значение по адресу
        WORD value = memory[index];
        WORD wChanged = Emulator_GetChangeRamStatus(address);
        //::SetTextColor(hdc, (wChanged != 0) ? RGB(255,0,0) : colorText);
        DrawOctalValue(painter, x + 12 * cxChar, y, value);
        //::SetTextColor(hdc, colorText);

        // Текущая позиция
        if (address == current) {
            painter.drawText(x + 2 * cxChar, y, _T(">>"));
            //::SetTextColor(hdc, m_okDebugCpuRChanged[reg] ? COLOR_RED : colorText);
            painter.drawText(x, y, REGISTER_NAME[reg]);
            //::SetTextColor(hdc, colorText);
        }

        address += 2;
        y += cyLine;
    }
}
