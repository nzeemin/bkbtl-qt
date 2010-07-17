#include "stdafx.h"
#include <QtGui>
#include "qdebugview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


//////////////////////////////////////////////////////////////////////


QDebugView::QDebugView(QWidget *parent) :
        QWidget(parent)
{
    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    this->setMinimumSize(cxChar * 55, cyLine * 14 + cyLine / 2);
    this->setMaximumHeight(cyLine * 14 + cyLine / 2);
}

void QDebugView::updateData()
{
    CProcessor* pCPU = g_pBoard->GetCPU();
    ASSERT(pCPU != NULL);

    // Get new register values and set change flags
    for (int r = 0; r < 8; r++) {
        WORD value = pCPU->GetReg(r);
        m_okDebugCpuRChanged[r] = (m_wDebugCpuR[r] != value);
        m_wDebugCpuR[r] = value;
    }
    WORD pswCPU = pCPU->GetPSW();
    m_okDebugCpuRChanged[8] = (m_wDebugCpuR[8] != pswCPU);
    m_wDebugCpuR[8] = pswCPU;
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

    drawProcessor(painter, pDebugPU, cxChar * 6, 1 * cyLine, arrR, arrRChanged);

    // Draw stack
    drawMemoryForRegister(painter, 6, pDebugPU, 35 * cxChar, 1 * cyLine);

    drawPorts(painter, 57 * cxChar, 1 * cyLine);
}

void QDebugView::drawProcessor(QPainter &painter, const CProcessor *pProc, int x, int y, WORD *arrR, BOOL *arrRChanged)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = painter.pen().color();

    painter.drawRect(x - cxChar, y - cyLine/2, 28 * cxChar, cyLine * 13);

    // Registers
    for (int r = 0; r < 8; r++) {
        painter.setPen(QColor(arrRChanged[r] ? Qt::red : colorText));

        LPCTSTR strRegName = REGISTER_NAME[r];
        painter.drawText(x, y + (1 + r) * cyLine, strRegName);

        WORD value = arrR[r]; //pProc->GetReg(r);
        DrawOctalValue(painter, x + cxChar * 3, y + (1 + r) * cyLine, value);
        DrawBinaryValue(painter, x + cxChar * 10, y + (1 + r) * cyLine, value);
    }
    painter.setPen(colorText);

    // PSW value
    painter.setPen(QColor(arrRChanged[8] ? Qt::red : colorText));
    painter.drawText(x, y + 10 * cyLine, _T("PS"));
    WORD psw = arrR[8]; // pProc->GetPSW();
    DrawOctalValue(painter, x + cxChar * 3, y + 10 * cyLine, psw);
    painter.drawText(x + cxChar * 10, y + 9 * cyLine, _T("       HP  TNZVC"));
    DrawBinaryValue(painter, x + cxChar * 10, y + 10 * cyLine, psw);

    painter.setPen(colorText);

    // Processor mode - HALT or USER
    BOOL okHaltMode = pProc->IsHaltMode();
    painter.drawText(x, y + 12 * cyLine, okHaltMode ? _T("HALT") : _T("USER"));

    // "Stopped" flag
    BOOL okStopped = pProc->IsStopped();
    if (okStopped)
        painter.drawText(x + 6 * cxChar, y + 12 * cyLine, _T("STOP"));
}

void QDebugView::drawMemoryForRegister(QPainter &painter, int reg, CProcessor *pProc, int x, int y)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = painter.pen().color();

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
        painter.setPen(wChanged != 0 ? Qt::red : colorText);
        DrawOctalValue(painter, x + 12 * cxChar, y, value);
        painter.setPen(colorText);

        // Текущая позиция
        if (address == current) {
            painter.drawText(x + 2 * cxChar, y, _T(">>"));
            painter.setPen(m_okDebugCpuRChanged[reg] != 0 ? Qt::red : colorText);
            painter.drawText(x, y, REGISTER_NAME[reg]);
            painter.setPen(colorText);
        }

        address += 2;
        y += cyLine;
    }
}

void QDebugView::drawPorts(QPainter &painter, int x, int y)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    painter.drawText(x, y, _T("Port"));

    WORD value;
    y += cyLine;
    value = g_pBoard->GetPortView(0177660);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177660);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("keyb state"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177662);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177662);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("keyb data"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177664);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177664);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("scroll"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177706);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177706);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("timer reload"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177710);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177710);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("timer value"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177712);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177712);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("timer manage"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177714);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177714);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("parallel"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177716);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177716);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("system"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177130);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177130);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("floppy state"));
    y += cyLine;
    value = g_pBoard->GetPortView(0177132);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177132);
    DrawOctalValue(painter, x + 8 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 16 * cxChar, y, _T("floppy data"));
}


//////////////////////////////////////////////////////////////////////
