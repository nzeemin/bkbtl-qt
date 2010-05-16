#include "stdafx.h"
#include <QtGui>
#include <QPainter>
#include "qmemoryview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"

QMemoryView::QMemoryView()
{
    m_wBaseAddress = 0;
    m_cyLineMemory = 0;
    m_nPageSize = 0;

    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    this->setMinimumSize(cxChar * 68, cyLine * 11 + cyLine / 2);
}

void QMemoryView::updateData()
{
}

void QMemoryView::paintEvent(QPaintEvent *event)
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

    QColor colorText = painter.pen().color();

    m_cyLineMemory = cyLine;

    const TCHAR* ADDRESS_LINE = _T(" address  0      2      4      6      10     12     14     16");
    painter.drawText(0, cyLine, ADDRESS_LINE);

    // Calculate m_nPageSize
    m_nPageSize = this->height() / cyLine - 1;

    WORD address = m_wBaseAddress;
    int y = 2 * cyLine;
    for (;;) {  // Draw lines
        DrawOctalValue(painter, 2 * cxChar, y, address);

        int x = 10 * cxChar;
        TCHAR wchars[16];

        for (int j = 0; j < 8; j++) {  // Draw words as octal value
            // Get word from memory
            WORD word = 0;
            int addrtype;
            BOOL okHalt = FALSE;
            WORD wChanged = 0;

            okHalt = g_pBoard->GetCPU()->IsHaltMode();
            word = g_pBoard->GetWordView(address, okHalt, FALSE, &addrtype);
            wChanged = Emulator_GetChangeRamStatus(address);

            if ((addrtype & (ADDRTYPE_IO | ADDRTYPE_DENY)) == 0)
            {
                painter.setPen(wChanged != 0 ? Qt::red : colorText);
                DrawOctalValue(painter, x, y, word);
            }

//            // Prepare characters to draw at right
//            BYTE ch1 = LOBYTE(word);
//            TCHAR wch1 = Translate_BK_Unicode(ch1);
//            if (ch1 < 32) wch1 = _T('·');
//            wchars[j * 2] = wch1;
//            BYTE ch2 = HIBYTE(word);
//            TCHAR wch2 = Translate_BK_Unicode(ch2);
//            if (ch2 < 32) wch2 = _T('·');
//            wchars[j * 2 + 1] = wch2;

            address += 2;
            x += 7 * cxChar;
        }
        painter.setPen(colorText);

//        // Draw characters at right
//        int xch = x + cxChar;
//        painter.drawText(hdc, xch, y, wchars);

        y += cyLine;
        if (y > this->height()) break;
    }
}
