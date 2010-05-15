#include "stdafx.h"
#include <QtGui>
#include "qdisasmview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"

QDisasmView::QDisasmView()
{
    m_wDisasmBaseAddr = 0;
    m_wDisasmNextBaseAddr = 0;

    setMinimumSize(520, 240);
}

void QDisasmView::updateData()
{
    CProcessor* pDisasmPU = g_pBoard->GetCPU();
    ASSERT(pDisasmPU != NULL);
    m_wDisasmBaseAddr = pDisasmPU->GetPC();
}

void QDisasmView::paintEvent(QPaintEvent *event)
{
    if (g_pBoard == NULL) return;

    QPainter painter(this);
    painter.fillRect(0,0, this->width(), this->height(), Qt::white);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    CProcessor* pDisasmPU = g_pBoard->GetCPU();
    ASSERT(pDisasmPU != NULL);

    // Draw disasseble for the current processor
    WORD prevPC = g_wEmulatorPrevCpuPC;
    DrawDisassemble(painter, pDisasmPU, m_wDisasmBaseAddr, prevPC, 0, 1 * cyLine);
}

void QDisasmView::DrawDisassemble(QPainter &painter, CProcessor *pProc, unsigned short base, unsigned short previous, int x, int y)
{
    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = painter.pen().color();

    WORD proccurrent = pProc->GetPC();
    WORD current = base;

    // ������ �� ������ ���������� � �����
    const int nWindowSize = 30;
    WORD memory[nWindowSize + 2];
    for (int idx = 0; idx < nWindowSize; idx++) {
        int addrtype;
        memory[idx] = g_pBoard->GetWordView(
                current + idx * 2 - 10, pProc->IsHaltMode(), TRUE, &addrtype);
    }

    WORD address = current - 10;
    WORD disasmfrom = current;
    if (previous >= address && previous < current)
        disasmfrom = previous;

    int length = 0;
    WORD wNextBaseAddr = 0;
    for (int index = 0; index < nWindowSize; index++)  // ������ ������
    {
//        if (m_okDisasmSubtitles)  // Subtitles - ����������� � �����
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

        DrawOctalValue(painter, x + 5 * cxChar, y, address);  // Address
        // Value at the address
        WORD value = memory[index];
        painter.setPen(Qt::gray);
        DrawOctalValue(painter, x + 13 * cxChar, y, value);
        painter.setPen(colorText);

        // Current position
        if (address == current)
            painter.drawText(x + 1 * cxChar, y, _T("  >"));
        if (address == proccurrent)
        {
//            BOOL okPCchanged = DebugView_IsRegisterChanged(7);
//            if (okPCchanged) painter.setPen(Qt::red);
            painter.drawText(x + 1 * cxChar, y, _T("PC"));
            painter.setPen(colorText);
            painter.drawText(x + 3 * cxChar, y, _T(">>"));
        }
        else if (address == previous)
        {
            painter.setPen(Qt::blue);
            painter.drawText(x + 1 * cxChar, y, _T("  >"));
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
//                TextOut(hdc, x + 52 * cxChar, y, strSubtitle, (int) wcslen(strSubtitle));
//                ::SetTextColor(hdc, colorText);
//
//                // ������ � ��������� �� ����� ������������ ��� ������� ��� �������������
//                if (disasmfrom > address)
//                    disasmfrom = address;
//            }
//        }

        if (address >= disasmfrom && length == 0)
        {
            TCHAR strInstr[8];
            TCHAR strArg[32];
            if (okData)  // �� ����� ������ ����� ������ -- ��� ������ �����������������
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
                painter.drawText(x + 21 * cxChar, y, strInstr);
                painter.drawText(x + 29 * cxChar, y, strArg);
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
}
