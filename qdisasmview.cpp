#include "stdafx.h"
#include <QtGui>
#include <QFileDialog>
#include <QMenu>
#include <QStyleOptionFocusRect>
#include "qdisasmview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"

#define COLOR_SUBTITLE  qRgb(0,128,0)
#define COLOR_JUMP      qRgb(80,192,224)
#define COLOR_JUMPYES   qRgb(80,240,80)
#define COLOR_JUMPGRAY  qRgb(180,180,180)
#define COLOR_JUMPHINT  qRgb(40,128,160)
#define COLOR_HINT      qRgb(40,40,160)
#define COLOR_CURRENT   qRgb(255,255,224)

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

void QDisasmView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_SubtitleItems.isEmpty() ? "Show Subtitles..." : "Hide Subtitles", this, SLOT(showHideSubtitles()));
    menu.exec(event->globalPos());
}

void QDisasmView::showHideSubtitles()
{
    if (m_SubtitleItems.size() > 0)
    {
        m_SubtitleItems.clear();
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(_T("BKBTL subtitles (*.lst)"));
        if (dlg.exec() == QDialog::Rejected)
            return;
        QString fileName = dlg.selectedFiles().at(0);

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            AlertWarning(_T("Failed to open the file."));
            return;
        }

        QTextStream stream(&file);
        parseSubtitles(stream);
    }

    //TODO: updateTitle();
    repaint();
}

void QDisasmView::addSubtitle(quint16 addr, DisasmSubtitleType type, const QString &comment)
{
    DisasmSubtitleItem item;
    item.address = addr;
    item.type = type;
    item.comment = comment;
    m_SubtitleItems.append(item);
}

void QDisasmView::parseSubtitles(QTextStream &stream)
{
    QString blockComment;

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        int lineLength = line.length();

        if (lineLength == 0) continue;  // Skip empty lines

        QChar firstChar = line.at(0);
        if (firstChar.isDigit())  // Цифра -- считаем что это адрес
        {
            // Parse address
            int addrlen = 1;
            while (addrlen < lineLength && line.at(addrlen).isDigit()) addrlen++;
            quint16 address;
            if (!ParseOctalValue(line.left(addrlen), &address))
                continue;

            if (!blockComment.isEmpty())  // На предыдущей строке был комментарий к блоку
            {
                addSubtitle(address, SUBTYPE_BLOCKCOMMENT, blockComment);
                blockComment.clear();
            }

            // Skip separators
            int index = addrlen;
            while (index < lineLength)
            {
                QChar ch = line.at(index);
                if (ch == ' ' || ch == '\t' || ch == '$' || ch == ':')
                    index++;
                else
                    break;
            }
            if (index == lineLength) continue;

            // Search for comment start
            while (index < lineLength)
            {
                QChar ch = line.at(index);
                if (ch == ';')
                    break;
                index++;
            }
            if (index == lineLength) continue;

            QString comment = line.mid(index).trimmed();
            if (comment.length() > 1)
            {
                addSubtitle(address, SUBTYPE_COMMENT, comment);
            }
        }
        else if (firstChar == ';')
        {
            blockComment = line.trimmed();
            //TODO: Собирать многострочные комментарии над блоком
        }
    }
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

const DisasmSubtitleItem * QDisasmView::findSubtitle(quint16 address, quint16 typemask)
{
    if (m_SubtitleItems.isEmpty())
        return 0;

    const DisasmSubtitleItem * item = m_SubtitleItems.constData();
    while (item->type != 0)
    {
        if (item->address > address)
            return 0;
        if (item->address == address &&
            (item->type &typemask) != 0)
            return item;
        ++item;
    }

    return 0;
}

bool QDisasmView::checkForJump(const quint16 *memory, int *pDelta)
{
    quint16 instr = *memory;

    // BR, BNE, BEQ, BGE, BLT, BGT, BLE
    // BPL, BMI, BHI, BLOS, BVC, BVS, BHIS, BLO
    if ((instr & 0177400) >= 0000400 && (instr & 0177400) < 0004000 ||
        (instr & 0177400) >= 0100000 && (instr & 0177400) < 0104000)
    {
        *pDelta = ((int)(char)(instr & 0xff)) + 1;
        return true;
    }

    // SOB
    if ((instr & ~(quint16)0777) == PI_SOB)
    {
        *pDelta = -(GetDigit(instr, 1) * 8 + GetDigit(instr, 0)) + 1;
        return true;
    }

    // CALL, JMP
    if (instr == 0004767 || instr == 0000167)
    {
        *pDelta = ((short)(memory[1]) + 4) / 2;
        return true;
    }

    return false;
}

bool QDisasmView::getJumpConditionHint(const quint16 *memory, const CProcessor *pProc, QString& buffer)
{
    buffer.clear();
    quint16 instr = *memory;
    quint16 psw = pProc->GetPSW();

    if (instr >= 0001000 && instr <= 0001777)  // BNE, BEQ
    {
        buffer.sprintf("Z=%c", (psw & PSW_Z) ? '1' : '0');
        // BNE: IF (Z == 0)
        // BEQ: IF (Z == 1)
        bool value = ((psw & PSW_Z) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0002000 && instr <= 0002777)  // BGE, BLT
    {
        buffer.sprintf("N=%c, V=%c", (psw & PSW_N) ? '1' : '0', (psw & PSW_V) ? '1' : '0');
        // BGE: IF ((N xor V) == 0)
        // BLT: IF ((N xor V) == 1)
        bool value = (((psw & PSW_N) != 0) != ((psw & PSW_V) != 0));
        return ((instr & 0400) == 0) ? !value : value;
    }

    if (instr >= 0003000 && instr <= 0003777)  // BGT, BLE
    {
        buffer.sprintf("N=%c, V=%c, Z=%c", (psw & PSW_N) ? '1' : '0', (psw & PSW_V) ? '1' : '0', (psw & PSW_Z) ? '1' : '0');
        // BGT: IF (((N xor V) or Z) == 0)
        // BLE: IF (((N xor V) or Z) == 1)
        bool value = ((((psw & PSW_N) != 0) != ((psw & PSW_V) != 0)) || ((psw & PSW_Z) != 0));
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0100000 && instr <= 0100777)  // BPL, BMI
    {
        buffer.sprintf("N=%c", (psw & PSW_N) ? '1' : '0');
        // BPL: IF (N == 0)
        // BMI: IF (N == 1)
        bool value = ((psw & PSW_N) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0101000 && instr <= 0101777)  // BHI, BLOS
    {
        buffer.sprintf("C=%c, Z=%c", (psw & PSW_C) ? '1' : '0', (psw & PSW_Z) ? '1' : '0');
        // BHI:  IF ((С or Z) == 0)
        // BLOS: IF ((С or Z) == 1)
        bool value = (((psw & PSW_C) != 0) || ((psw & PSW_Z) != 0));
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0102000 && instr <= 0102777)  // BVC, BVS
    {
        buffer.sprintf("V=%c", (psw & PSW_V) ? '1' : '0');
        // BVC: IF (V == 0)
        // BVS: IF (V == 1)
        bool value = ((psw & PSW_V) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0103000 && instr <= 0103777)  // BCC/BHIS, BCS/BLO
    {
        buffer.sprintf("C=%c", (psw & PSW_C) ? '1' : '0');
        // BCC/BHIS: IF (C == 0)
        // BCS/BLO:  IF (C == 1)
        bool value = ((psw & PSW_C) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0077000 && instr <= 0077777)  // SOB
    {
        int reg = (instr >> 6) & 7;
        quint16 regvalue = pProc->GetReg(reg);
        buffer.sprintf("R%d=%06o", reg, regvalue);  // "RN=XXXXXX"
        return (regvalue != 1);
    }

    if (instr >= 004000 && instr <= 004677)  // JSR (except CALL)
    {
        int reg = (instr >> 6) & 7;
        quint16 regvalue = pProc->GetReg(reg);
        buffer.sprintf("R%d=%06o", reg, regvalue);  // "RN=XXXXXX"
        return true;
    }
    if (instr >= 000200 && instr <= 000207)  // RTS / RETURN
    {
        quint16 spvalue = pProc->GetSP();
        int addrtype;
        quint16 value = g_pBoard->GetWordView(spvalue, pProc->IsHaltMode(), false, &addrtype);
        if (instr == 000207)  // RETURN
            buffer.sprintf("(SP)=%06o", value);  // "(SP)=XXXXXX"
        else  // RTS
        {
            int reg = instr & 7;
            quint16 regvalue = pProc->GetReg(reg);
            buffer.sprintf("R%d=%06o, (SP)=%06o", reg, regvalue, value);  // "RN=XXXXXX, (SP)=XXXXXX"
        }
        return true;
    }

    //TODO: RTI, RTT

    return true;  // All other jumps are non-conditional
}

bool QDisasmView::getInstructionHint(const quint16 *memory, const CProcessor *pProc, QString &buffer)
{
    buffer.clear();
    quint16 instr = *memory;

    // Source and Destination
    if ((instr & ~(uint16_t)0107777) == PI_MOV || (instr & ~(uint16_t)0107777) == PI_CMP ||
        (instr & ~(uint16_t)0107777) == PI_BIT || (instr & ~(uint16_t)0107777) == PI_BIC || (instr & ~(uint16_t)0107777) == PI_BIS ||
        (instr & ~(uint16_t)0007777) == PI_ADD || (instr & ~(uint16_t)0007777) == PI_SUB)
    {
        int srcreg = (instr >> 6) & 7;
        //int srcmod = (instr >> 9) & 7;
        uint16_t srcregval = pProc->GetReg(srcreg);
        int dstreg = instr & 7;
        //int dstmod = (instr >> 3) & 7;
        uint16_t dstregval = pProc->GetReg(dstreg);
        if ((srcreg != 7 && dstreg == 7) || (srcreg != 7 && dstreg == srcreg))
            buffer.sprintf("%s=%06o", REGISTER_NAME[srcreg], srcregval);  // "RN=XXXXXX"
        else if (srcreg == 7 && dstreg != 7)
            buffer.sprintf("%s=%06o", REGISTER_NAME[dstreg], dstregval);  // "RN=XXXXXX"
        else if (srcreg != 7 && dstreg != 7)
            buffer.sprintf("%s=%06o, %s=%06o", REGISTER_NAME[srcreg], srcregval, REGISTER_NAME[dstreg], dstregval);  // "RN=XXXXXX, RN=XXXXXX"
    }

    // Register and Destination
    if ((instr & ~(uint16_t)0777) == PI_MUL || (instr & ~(uint16_t)0777) == PI_DIV ||
        (instr & ~(uint16_t)0777) == PI_ASH || (instr & ~(uint16_t)0777) == PI_ASHC ||
        (instr & ~(uint16_t)0777) == PI_XOR)
    {
        int srcreg = (instr >> 6) & 7;
        uint16_t srcregval = pProc->GetReg(srcreg);
        int dstreg = instr & 7;
        //int dstmod = (instr >> 3) & 7;
        uint16_t dstregval = pProc->GetReg(dstreg);
        if ((srcreg != 7 && dstreg == 7) || (srcreg != 7 && dstreg == srcreg))
            buffer.sprintf("%s=%06o", REGISTER_NAME[srcreg], srcregval);  // "RN=XXXXXX"
        else if (srcreg == 7 && dstreg != 7)
            buffer.sprintf("%s=%06o", REGISTER_NAME[dstreg], dstregval);  // "RN=XXXXXX"
        else if (srcreg != 7 && dstreg != 7)
            buffer.sprintf("%s=%06o, %s=%06o", REGISTER_NAME[srcreg], srcregval, REGISTER_NAME[dstreg], dstregval);  // "RN=XXXXXX, RN=XXXXXX"
    }

    // Destination only
    if ((instr & ~(uint16_t)0100077) == PI_CLR || (instr & ~(uint16_t)0100077) == PI_COM ||
        (instr & ~(uint16_t)0100077) == PI_INC || (instr & ~(uint16_t)0100077) == PI_DEC || (instr & ~(uint16_t)0100077) == PI_NEG ||
        (instr & ~(uint16_t)0100077) == PI_TST ||
        (instr & ~(uint16_t)0100077) == PI_ASR || (instr & ~(uint16_t)0100077) == PI_ASL ||
        (instr & ~(uint16_t)077) == PI_JMP ||
        (instr & ~(uint16_t)077) == PI_SWAB || (instr & ~(uint16_t)077) == PI_SXT ||
        (instr & ~(uint16_t)077) == PI_MTPS || (instr & ~(uint16_t)077) == PI_MFPS)
    {
        int dstreg = instr & 7;
        //int dstmod = (instr >> 3) & 7;
        uint16_t dstregval = pProc->GetReg(dstreg);
        if (dstreg != 7)
        {
            buffer.sprintf("%s=%06o", REGISTER_NAME[dstreg], dstregval);  // "RN=XXXXXX"
            return true;
        }
    }

    // ADC, SBC, ROR, ROL: also show C flag
    if ((instr & ~(uint16_t)0100077) == PI_ADC || (instr & ~(uint16_t)0100077) == PI_SBC ||
        (instr & ~(uint16_t)0100077) == PI_ROR || (instr & ~(uint16_t)0100077) == PI_ROL)
    {
        int dstreg = instr & 7;
        //int dstmod = (instr >> 3) & 7;
        uint16_t dstregval = pProc->GetReg(dstreg);
        uint16_t psw = pProc->GetPSW();
        if (dstreg != 7)
        {
            buffer.sprintf("%s=%06o, C=%c", REGISTER_NAME[dstreg], dstregval, (psw & PSW_C) ? '1' : '0');  // "RN=XXXXXX, C=X"
            return true;
        }
    }

    // CLC..CCC, SEC..SCC -- show flags
    if (instr >= 0000241 && instr <= 0000257 || instr >= 0000261 && instr <= 0000277)
    {
        uint16_t psw = pProc->GetPSW();
        buffer.sprintf("C=%c, V=%c, Z=%c, N=%c",
                (psw & PSW_C) ? '1' : '0', (psw & PSW_V) ? '1' : '0', (psw & PSW_Z) ? '1' : '0', (psw & PSW_N) ? '1' : '0');
        return true;
    }

    return !buffer.isEmpty();
}

int QDisasmView::DrawDisassemble(QPainter &painter, CProcessor *pProc, quint16 base, quint16 previous)
{
    int result = -1;

    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = painter.pen().color();

    quint16 proccurrent = pProc->GetPC();
    quint16 current = base;

    // Draw current line background
    if (m_SubtitleItems.isEmpty())  //NOTE: Subtitles can move lines down
    {
        int yCurrent = (proccurrent - (current - 5)) * cyLine;
        painter.fillRect(0, yCurrent, 1000, cyLine, COLOR_CURRENT);
    }

    // Читаем из памяти процессора в буфер
    const int nWindowSize = 30; //this->height() / cyLine;
    quint16 memory[nWindowSize + 2];
    int addrtype[nWindowSize + 2];
    for (int idx = 0; idx < nWindowSize; idx++) {
        memory[idx] = g_pBoard->GetWordView(
                current + idx * 2 - 10, pProc->IsHaltMode(), true, addrtype + idx);
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
        if (!m_SubtitleItems.isEmpty())  // Subtitles - комментарий к блоку
        {
            const DisasmSubtitleItem * pSubItem = findSubtitle(address, SUBTYPE_BLOCKCOMMENT);
            if (pSubItem != NULL && !pSubItem->comment.isEmpty())
            {
                painter.setPen(QColor(COLOR_SUBTITLE));
                painter.drawText(21 * cxChar, y, pSubItem->comment);
                painter.setPen(colorText);

                y += cyLine;
            }
        }

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
            bool okPCchanged = proccurrent != previous;
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

        bool okData = false;
        if (!m_SubtitleItems.isEmpty())  // Show subtitle
        {
            const DisasmSubtitleItem* pSubItem = findSubtitle(address, SUBTYPE_COMMENT | SUBTYPE_DATA);
            if (pSubItem != NULL && (pSubItem->type & SUBTYPE_DATA) != 0)
                okData = true;
            if (pSubItem != NULL && (pSubItem->type & SUBTYPE_COMMENT) != 0 && !pSubItem->comment.isEmpty())
            {
                painter.setPen(QColor(COLOR_SUBTITLE));
                painter.drawText(52 * cxChar, y, pSubItem->comment);
                painter.setPen(colorText);

                // Строку с субтитром мы можем использовать как опорную для дизассемблера
                if (disasmfrom > address)
                    disasmfrom = address;
            }
        }

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

                if (m_SubtitleItems.isEmpty())  //NOTE: Subtitles can move lines down
                {
                    int delta;
                    bool isjump = checkForJump(memory + index, &delta);
                    if (isjump && abs(delta) < 40)
                        drawJump(painter, y, delta, (30 + _tcslen(strArg)) * cxChar, cyLine, COLOR_JUMP);

                    if (address == proccurrent)
                    {
                        // For current instruction, draw "Instruction Hint"
                        QString strHint;
                        bool jumppredict = getJumpConditionHint(memory + index, pProc, strHint);
                        if (!strHint.isEmpty())  // If we have the hint
                        {
                            painter.setPen(COLOR_JUMPHINT);
                            painter.drawText(48 * cxChar, y, strHint);
                        }
                        else if (getInstructionHint(memory + index, pProc, strHint))
                        {
                            painter.setPen(COLOR_HINT);
                            painter.drawText(52 * cxChar, y, strHint);
                        }

                        if (isjump && abs(delta) < 40)
                        {
                            QRgb jumpcolor = jumppredict ? COLOR_JUMPYES : COLOR_JUMPGRAY;
                            drawJump(painter, y, delta, (30 + _tcslen(strArg)) * cxChar, cyLine, jumpcolor);
                        }
                    }
                }
            }
            painter.setPen(colorText);
            if (index + length <= nWindowSize)
            {
                painter.drawText(21 * cxChar, y, strInstr);
                painter.drawText(29 * cxChar, y, strArg);
            }
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

void QDisasmView::drawJump(QPainter &painter, int yFrom, int delta, int x, int cyLine, QRgb color)
{
    int dist = abs(delta);
    if (dist < 2) dist = 2;
    if (dist > 20) dist = 16;

    int yTo = yFrom + delta * cyLine - (cyLine * 2 / 3);
    yFrom -= cyLine / 3;

    QPainterPath path;
    path.moveTo(x, yFrom);
    path.cubicTo(x + dist * 4, yFrom, x + dist * 12, yTo, x + 4, yTo);
    path.moveTo(x - 4, yTo);
    path.lineTo(x + 3, yTo - 1);
    path.moveTo(x - 4, yTo);
    path.lineTo(x + 3, yTo + 1);

    painter.setPen(color);
    painter.drawPath(path);
}
