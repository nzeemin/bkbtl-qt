#include "stdafx.h"
#include <QtGui>
#include <QStyleFactory>
#include <QStyleOptionFocusRect>
#include <QToolBar>
#include "qdebugview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


//////////////////////////////////////////////////////////////////////


QDebugView::QDebugView(QWidget *mainWindow) :
    QWidget()
{
    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    this->setMinimumSize(cxChar * 55, cyLine * 16 + cyLine / 2);
    this->setMaximumHeight(cyLine * 16 + cyLine / 2);

    m_toolbar = new QToolBar(this);
    m_toolbar->setGeometry(4, 4, 36, cyLine * 16);
    m_toolbar->setOrientation(Qt::Vertical);
    m_toolbar->setIconSize(QSize(24, 24));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setFocusPolicy(Qt::NoFocus);
    m_toolbar->setStyle(QStyleFactory::create("windows"));  // fix for macOS to remove gradient background

    QAction* actionStepInto = m_toolbar->addAction(QIcon(":/images/iconStepInto.svg"), "");
    QAction* actionStepOver = m_toolbar->addAction(QIcon(":/images/iconStepOver.svg"), "");

    QObject::connect((const QObject*)actionStepInto, SIGNAL(triggered()), mainWindow, SLOT(debugStepInto()));
    QObject::connect((const QObject*)actionStepOver, SIGNAL(triggered()), mainWindow, SLOT(debugStepOver()));

    setFocusPolicy(Qt::ClickFocus);

    memset(m_wDebugCpuR, 255, sizeof(m_wDebugCpuR));
    memset(m_okDebugCpuRChanged, 1, sizeof(m_okDebugCpuRChanged));
}

void QDebugView::updateData()
{
    CProcessor* pCPU = g_pBoard->GetCPU();
    ASSERT(pCPU != nullptr);

    // Get new register values and set change flags
    m_wDebugCpuR6Old = m_wDebugCpuR[6];
    for (int r = 0; r < 8; r++)
    {
        quint16 value = pCPU->GetReg(r);
        m_okDebugCpuRChanged[r] = (m_wDebugCpuR[r] != value);
        m_wDebugCpuR[r] = value;
    }
    quint16 pswCPU = pCPU->GetPSW();
    m_okDebugCpuRChanged[8] = (m_wDebugCpuR[8] != pswCPU);
    m_wDebugCpuR[8] = pswCPU;
}

void QDebugView::focusInEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}
void QDebugView::focusOutEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}

void QDebugView::paintEvent(QPaintEvent * /*event*/)
{
    if (g_pBoard == nullptr) return;

    QColor colorBackground = palette().color(QPalette::Base);
    QPainter painter(this);
    painter.fillRect(0, 0, this->width(), this->height(), colorBackground);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    CProcessor* pDebugPU = g_pBoard->GetCPU();
    ASSERT(pDebugPU != nullptr);
    quint16* arrR = m_wDebugCpuR;
    bool* arrRChanged = m_okDebugCpuRChanged;
    quint16 oldSP = m_wDebugCpuR6Old;

    drawProcessor(painter, pDebugPU, 38 + cxChar * 2, 1 * cyLine, arrR, arrRChanged);

    // Draw stack
    drawMemoryForRegister(painter, 6, pDebugPU, 38 + 35 * cxChar, 1 * cyLine, oldSP);

    drawPorts(painter, 38 + 54 * cxChar, 1 * cyLine);

    bool okBreakpoints = drawBreakpoints(painter, 38 + 80 * cxChar, 1 * cyLine);

    // Draw focus rect
    if (hasFocus())
    {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.state |= QStyle::State_KeyboardFocusChange;
        option.backgroundColor = QColor(Qt::gray);
        option.rect = this->rect();
        option.rect.setLeft(option.rect.left() + 38);
        option.rect.setRight(38 + cxChar * 89);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void QDebugView::drawProcessor(QPainter &painter, const CProcessor *pProc, int x, int y, quint16 *arrR, bool *arrRChanged)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = palette().color(QPalette::Text);
    QColor colorChanged = Common_GetColorShifted(palette(), COLOR_VALUECHANGED);

    painter.setPen(QColor(Qt::gray));
    painter.drawRect(x - cxChar, y - cyLine / 2, 33 * cxChar, cyLine * 15 + cyLine / 2);

    // Registers
    for (int r = 0; r < 8; r++)
    {
        painter.setPen(QColor(arrRChanged[r] ? colorChanged : colorText));

        const char * strRegName = REGISTER_NAME[r];
        painter.drawText(x, y + (1 + r) * cyLine, strRegName);

        quint16 value = arrR[r]; //pProc->GetReg(r);
        DrawOctalValue(painter, x + cxChar * 3, y + (1 + r) * cyLine, value);
        DrawHexValue(painter, x + cxChar * 10, y + (1 + r) * cyLine, value);
        DrawBinaryValue(painter, x + cxChar * 15, y + (1 + r) * cyLine, value);
    }
    painter.setPen(colorText);

    // PSW value
    painter.setPen(QColor(arrRChanged[8] ? colorChanged : colorText));
    painter.drawText(x, y + 11 * cyLine, "PS");
    quint16 psw = arrR[8]; // pProc->GetPSW();
    DrawOctalValue(painter, x + cxChar * 3, y + 11 * cyLine, psw);
    DrawHexValue(painter, x + cxChar * 10, y + 11 * cyLine, psw);
    painter.drawText(x + cxChar * 15, y + 10 * cyLine, "       HP  TNZVC");
    DrawBinaryValue(painter, x + cxChar * 15, y + 11 * cyLine, psw);

    painter.setPen(colorText);

    // Processor mode - HALT or USER
    bool okHaltMode = pProc->IsHaltMode();
    painter.drawText(x, y + 13 * cyLine, okHaltMode ? "HALT" : "USER");

    // "Stopped" flag
    bool okStopped = pProc->IsStopped();
    if (okStopped)
        painter.drawText(x + 6 * cxChar, y + 13 * cyLine, "STOP");
}

void QDebugView::drawMemoryForRegister(QPainter &painter, int reg, CProcessor *pProc, int x, int y, quint16 oldValue)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = palette().color(QPalette::Text);
    QColor colorChanged = Common_GetColorShifted(palette(), COLOR_VALUECHANGED);
    QColor colorPrev = Common_GetColorShifted(palette(), COLOR_PREVIOUS);

    quint16 current = pProc->GetReg(reg);
    quint16 previous = oldValue;
    bool okExec = (reg == 7);

    // Reading from memory into the buffer
    quint16 memory[16];
    for (int idx = 0; idx < 16; idx++)
    {
        int addrtype;
        memory[idx] = g_pBoard->GetWordView(
                current + idx * 2 - 14, pProc->IsHaltMode(), okExec, &addrtype);
    }

    quint16 address = current - 16;
    for (int index = 0; index < 16; index++)    // Draw strings
    {
        // Address
        painter.setPen(colorText);
        DrawOctalValue(painter, x + 3 * cxChar, y, address);

        // Value at the address
        quint16 value = memory[index];
        quint16 wChanged = Emulator_GetChangeRamStatus(address);
        painter.setPen(wChanged != 0 ? colorChanged : colorText);
        DrawOctalValue(painter, x + 10 * cxChar, y, value);

        // Current position
        if (address == current)
        {
            painter.setPen(colorText);
            painter.drawText(x + 2 * cxChar, y, ">");
            painter.setPen(current != previous ? colorChanged : colorText);
            painter.drawText(x, y, REGISTER_NAME[reg]);
        }
        else if (address == previous)
        {
            painter.setPen(colorPrev);
            painter.drawText(x + 2 * cxChar, y, ">");
        }

        address += 2;
        y += cyLine;
    }

    painter.setPen(colorText);
}

void QDebugView::drawPorts(QPainter &painter, int x, int y)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    painter.drawText(x, y, tr("Port"));

    quint16 value;
    y += cyLine;
    value = g_pBoard->GetPortView(0177660);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177660);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "kbd state");
    y += cyLine;
    value = g_pBoard->GetPortView(0177662);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177662);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "kbd data");
    y += cyLine;
    value = g_pBoard->GetPortView(0177664);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177664);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "scroll");
    y += cyLine;
    value = g_pBoard->GetPortView(0177706);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177706);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "timer rel");
    y += cyLine;
    value = g_pBoard->GetPortView(0177710);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177710);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "timer val");
    y += cyLine;
    value = g_pBoard->GetPortView(0177712);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177712);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "timer ctl");
    y += cyLine;
    value = g_pBoard->GetPortView(0177714);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177714);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "parallel");
    y += cyLine;
    value = g_pBoard->GetPortView(0177716);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177716);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    //DrawBinaryValue(painter, x + 15 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "system");
    y += cyLine;
    value = g_pBoard->GetPortView(0177130);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177130);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "fdd state");
    y += cyLine;
    value = g_pBoard->GetPortView(0177132);
    DrawOctalValue(painter, x + 0 * cxChar, y, 0177132);
    DrawOctalValue(painter, x + 7 * cxChar, y, value);
    painter.drawText(x + 14 * cxChar, y, "fdd data");
}

bool QDebugView::drawBreakpoints(QPainter &painter, int x, int y)
{
    const quint16* pbps = Emulator_GetCPUBreakpointList();
    if (*pbps == 0177777)
        return false;

    QFontMetrics fontmetrics(painter.font());
    int cyLine = fontmetrics.height();

    painter.drawText(x, y, tr("Breakpts"));
    y += cyLine;
    while (*pbps != 0177777)
    {
        DrawOctalValue(painter, x, y, *pbps);
        y += cyLine;
        pbps++;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
