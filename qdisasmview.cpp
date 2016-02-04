#include "stdafx.h"
#include <QtGui>
#include <QStyleOptionFocusRect>
#include "qdisasmview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"

QDisasmView::QDisasmView()
{
    m_wDisasmBaseAddr = 0;
    m_wDisasmNextBaseAddr = 0;

    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    this->setMinimumSize(cxChar * 55, cyLine * 10 + cyLine / 2);

    setFocusPolicy(Qt::ClickFocus);
}

void QDisasmView::updateData()
{
    CProcessor* pDisasmPU = g_pBoard->GetCPU();
    ASSERT(pDisasmPU != NULL);
    m_wDisasmBaseAddr = pDisasmPU->GetPC();
}

void QDisasmView::focusInEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}
void QDisasmView::focusOutEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}

void QDisasmView::paintEvent(QPaintEvent * /*event*/)
{
    if (g_pBoard == NULL) return;

    QPainter painter(this);
    painter.fillRect(0,0, this->width(), this->height(), Qt::white);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cyLine = fontmetrics.height();

    CProcessor* pDisasmPU = g_pBoard->GetCPU();
    ASSERT(pDisasmPU != NULL);

    // Draw disasseble for the current processor
    quint16 prevPC = g_wEmulatorPrevCpuPC;
    int yFocus = DrawDisassemble(painter, pDisasmPU, m_wDisasmBaseAddr, prevPC);

    // Draw focus rect
    if (hasFocus())
    {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.state |= QStyle::State_KeyboardFocusChange;
        option.backgroundColor = QColor(Qt::gray);
        option.rect = QRect(0, yFocus - cyLine + 1, 1000, cyLine);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

int QDisasmView::DrawDisassemble(QPainter &painter, CProcessor *pProc, unsigned short base, unsigned short previous)
{
    int result = -1;

    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = painter.pen().color();

    quint16 proccurrent = pProc->GetPC();
    quint16 current = base;

    // Читаем из памяти процессора в буфер
    const int nWindowSize = 30; //this->height() / cyLine;
    quint16 memory[nWindowSize + 2];
    for (int idx = 0; idx < nWindowSize; idx++) {
        int addrtype;
        memory[idx] = g_pBoard->GetWordView(
                current + idx * 2 - 10, pProc->IsHaltMode(), TRUE, &addrtype);
    }

    quint16 address = current - 10;
    quint16 disasmfrom = current;
    if ((previous >= address) && previous < current)
        disasmfrom = previous;

    int length = 0;
    quint16 wNextBaseAddr = 0;
    int y = cyLine;
    for (int index = 0; index < nWindowSize; index++)  // Рисуем строки
    {
//        if (m_okDisasmSubtitles)  // Subtitles - комментарий к блоку
//        {
//            DisasmSubtitleItem* pSubItem = DisasmView_FindSubtitle(address, SUBTYPE_BLOCKCOMMENT);
//            if (pSubItem != NULL && pSubItem->comment != NULL)
//            {
//                LPCTSTR strBlockSubtitle = pSubItem->comment;
//
//                ::SetTextColor(hdc, COLOR_SUBTITLE);
//                TextOut(hdc, x + 21 * cxChar, y, strBlockSubtitle, (int) wcslen(strBlockSubtitle));
//                ::SetTextColor(hdc, colorText);
//
//                y += cyLine;
//            }
//        }

        DrawOctalValue(painter, 5 * cxChar, y, address);  // Address
        // Value at the address
        quint16 value = memory[index];
        painter.setPen(Qt::gray);
        DrawOctalValue(painter, 13 * cxChar, y, value);
        painter.setPen(colorText);

        // Current position
        if (address == current)
        {
            painter.drawText(1 * cxChar, y, _T("  >"));
            result = y;  // Remember line for the focus rect
        }
        if (address == proccurrent)
        {
            BOOL okPCchanged = proccurrent != previous;
            if (okPCchanged) painter.setPen(Qt::red);
            painter.drawText(1 * cxChar, y, _T("PC"));
            painter.setPen(colorText);
            painter.drawText(3 * cxChar, y, _T(">>"));
        }
        else if (address == previous)
        {
            painter.setPen(Qt::blue);
            painter.drawText(1 * cxChar, y, _T("  >"));
        }

        BOOL okData = FALSE;
//        if (m_okDisasmSubtitles)  // Show subtitle
//        {
//            DisasmSubtitleItem* pSubItem = DisasmView_FindSubtitle(address, SUBTYPE_COMMENT | SUBTYPE_DATA);
//            if (pSubItem != NULL && (pSubItem->type & SUBTYPE_DATA) != 0)
//                okData = TRUE;
//            if (pSubItem != NULL && (pSubItem->type & SUBTYPE_COMMENT) != 0 && pSubItem->comment != NULL)
//            {
//                LPCTSTR strSubtitle = pSubItem->comment;
//
//                ::SetTextColor(hdc, COLOR_SUBTITLE);
//                TextOut(hdc, 52 * cxChar, y, strSubtitle, (int) wcslen(strSubtitle));
//                ::SetTextColor(hdc, colorText);
//
//                // Строку с субтитром мы можем использовать как опорную для дизассемблера
//                if (disasmfrom > address)
//                    disasmfrom = address;
//            }
//        }

        if (address >= disasmfrom && length == 0)
        {
            TCHAR strInstr[8];
            TCHAR strArg[32];
            if (okData)  // По этому адресу лежат данные -- нет смысла дизассемблировать
            {
                _tcscpy(strInstr, _T("data"));
                PrintOctalValue(strArg, *(memory + index));
                length = 1;
            }
            else
            {
                length = DisassembleInstruction(memory + index, address, strInstr, strArg);
            }
            if (index + length <= nWindowSize)
            {
                painter.drawText(21 * cxChar, y, strInstr);
                painter.drawText(29 * cxChar, y, strArg);
            }
            painter.setPen(colorText);
            if (wNextBaseAddr == 0)
                wNextBaseAddr = address + length * 2;
        }
        if (length > 0) length--;

        address += 2;
        y += cyLine;
    }

    m_wDisasmNextBaseAddr = wNextBaseAddr;

    return result;
}
