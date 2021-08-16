#include "stdafx.h"
#include <QtGui>
#include <QFileDialog>
#include <QMenu>
#include <QStyleOptionFocusRect>
#include "qdisasmview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


const int MAX_DISASMLINECOUNT = 50;


//////////////////////////////////////////////////////////////////////


QDisasmView::QDisasmView()
{
    m_wDisasmBaseAddr = 0;
    m_wDisasmNextBaseAddr = 0;
    m_cxDisasmBreakpointZone = 16;
    m_cyDisasmLine = 10;

    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    this->setMinimumSize(cxChar * 55, cyLine * 10 + cyLine / 2);

    setFocusPolicy(Qt::ClickFocus);
}

void QDisasmView::updateWindowText()
{
    QString buffer = QString(tr("Disassemble"));
    if (!m_SubtitleItems.isEmpty())
        buffer.append(tr(" - Subtitles"));
    parentWidget()->setWindowTitle(buffer);
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
    menu.addAction(m_SubtitleItems.isEmpty() ? tr("Show Subtitles...") : tr("Hide Subtitles"), this, SLOT(showHideSubtitles()));
    menu.exec(event->globalPos());
}

void QDisasmView::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->x() < m_cxDisasmBreakpointZone)
        {
            int lineindex = event->y() / m_cyDisasmLine;
            if (lineindex >= 0 && lineindex < m_DisasmLineItems.count())
            {
                DisasmLineItem& lineitem = m_DisasmLineItems[lineindex];
                if (lineitem.type != LINETYPE_NONE)
                {
                    quint16 address = lineitem.address;
                    if (!Emulator_IsBreakpoint(address))
                    {
                        bool result = Emulator_AddCPUBreakpoint(address);
                        if (!result)
                            AlertWarning(tr("Failed to add breakpoint at %1.").arg(address, 6, 8, QLatin1Char('0')));
                    }
                    else
                    {
                        bool result = Emulator_RemoveCPUBreakpoint(address);
                        if (!result)
                            AlertWarning(tr("Failed to remove breakpoint at %1.").arg(address, 6, 8, QLatin1Char('0')));
                    }
                    repaint();
                }
            }
        }
    }

    QWidget::mousePressEvent(event);
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
        dlg.setNameFilter(tr("BKBTL subtitles (*.lst)"));
        if (dlg.exec() == QDialog::Rejected)
            return;
        QString fileName = dlg.selectedFiles().at(0);

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            AlertWarning(tr("Failed to open the file."));
            return;
        }

        QTextStream stream(&file);
        parseSubtitles(stream);
    }

    updateWindowText();
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
            bool okDirective = index < lineLength && line.at(index) == '.';

            // Search for comment start
            while (index < lineLength)
            {
                QChar ch = line.at(index);
                if (ch == ';')
                    break;
                index++;
            }

            QString comment = line.mid(index).trimmed();
            if (comment.length() > 1 || okDirective)
            {
                DisasmSubtitleType type = okDirective ? (DisasmSubtitleType)(SUBTYPE_COMMENT | SUBTYPE_DATA) : SUBTYPE_COMMENT;
                addSubtitle(address, type, comment);
            }
        }
        else if (firstChar == ';')
        {
            blockComment = line.trimmed();
            //TODO: Собирать многострочные комментарии над блоком
        }
    }
}

const DisasmSubtitleItem * QDisasmView::findSubtitle(quint16 address, quint16 typemask)
{
    if (m_SubtitleItems.isEmpty())
        return nullptr;

    const DisasmSubtitleItem * item = m_SubtitleItems.constData();
    while (item->type != 0)
    {
        if (item->address > address)
            return nullptr;
        if (item->address == address &&
            (item->type & typemask) != 0)
            return item;
        ++item;
    }

    return nullptr;
}

bool QDisasmView::checkForJump(const quint16 *memory, int *pDelta)
{
    quint16 instr = *memory;

    // BR, BNE, BEQ, BGE, BLT, BGT, BLE
    // BPL, BMI, BHI, BLOS, BVC, BVS, BHIS, BLO
    if (((instr & 0177400) >= 0000400 && (instr & 0177400) < 0004000) ||
        ((instr & 0177400) >= 0100000 && (instr & 0177400) < 0104000))
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

// Prepare "Jump Hint" string, and also calculate condition for conditional jump
// Returns: jump prediction flag: true = will jump, false = will not jump
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
        // BHI:  IF ((C or Z) == 0)
        // BLOS: IF ((C or Z) == 1)
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

    if (instr == 000002 || instr == 000006)  // RTI, RTT
    {
        quint16 spvalue = pProc->GetSP();
        int addrtype;
        quint16 value = g_pBoard->GetWordView(spvalue, pProc->IsHaltMode(), false, &addrtype);
        buffer.sprintf("(SP)=%06o", value);  // "(SP)=XXXXXX"
        return true;
    }
    if (instr == 000003 || instr == 000004 ||  // IOT, BPT
        (instr >= 0104000 && instr <= 0104777))  // TRAP, EMT
    {
        quint16 intvec;
        if (instr == 000003) intvec = 000014;
        else if (instr == 000004) intvec = 000020;
        else if (instr < 0104400) intvec = 000030;
        else intvec = 000034;

        int addrtype;
        quint16 value = g_pBoard->GetWordView(intvec, pProc->IsHaltMode(), false, &addrtype);
        buffer.sprintf("(%06o)=%06o", intvec, value);  // "(VVVVVV)=XXXXXX"
        return true;
    }

    return true;  // All other jumps are non-conditional
}

void QDisasmView::registerHint(const CProcessor *pProc,
        QString &hint1, QString& hint2,
        int regnum, int regmod, bool byteword, quint16 indexval)
{
    int addrtype = 0;
    quint16 regval = pProc->GetReg(regnum);
    quint16 srcval2 = 0;

    hint1.sprintf("%s=%06o", REGISTER_NAME[regnum], regval);  // "RN=XXXXXX"
    switch (regmod)
    {
    case 1:
    case 2:
        srcval2 = g_pBoard->GetWordView(regval, pProc->IsHaltMode(), false, &addrtype);
        if (byteword)
        {
            srcval2 = (regval & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
            hint2.sprintf("(%s)=%03o", REGISTER_NAME[regnum], srcval2);  // "(RN)=XXX"
        }
        else
        {
            hint2.sprintf("(%s)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN)=XXXXXX"
        }
        break;
    case 3:
        srcval2 = g_pBoard->GetWordView(regval, pProc->IsHaltMode(), false, &addrtype);
        hint2.sprintf("(%s)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN)=XXXXXX"
        //TODO: Show the real value in hint line 3
        break;
    case 4:
        if (byteword)
        {
            srcval2 = (regval & 1) ?
                    ((g_pBoard->GetWordView(regval - 1, pProc->IsHaltMode(), false, &addrtype)) & 0xff) :
                    ((g_pBoard->GetWordView(regval - 2, pProc->IsHaltMode(), false, &addrtype)) >> 8);
            hint2.sprintf("(%s-1)=%03o", REGISTER_NAME[regnum], srcval2);  // "(RN-1)=XXX"
        }
        else
        {
            srcval2 = g_pBoard->GetWordView(regval - 2, pProc->IsHaltMode(), false, &addrtype);
            hint2.sprintf("(%s-2)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN-2)=XXXXXX"
        }
        break;
    case 5:
        srcval2 = g_pBoard->GetWordView(regval - 2, pProc->IsHaltMode(), false, &addrtype);
        hint2.sprintf("(%s-2)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN+2)=XXXXXX"
        //TODO: Show the real value in hint line 3
        break;
    case 6:
        {
            quint16 addr2 = regval + indexval;
            srcval2 = g_pBoard->GetWordView(addr2 & ~1, pProc->IsHaltMode(), false, &addrtype);
            if (byteword)
            {
                srcval2 = (addr2 & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
                hint2.sprintf("(%s+%06o)=%03o", REGISTER_NAME[regnum], indexval, srcval2);  // "(RN+NNNNNN)=XXX"
            }
            else
            {
                hint2.sprintf("(%s+%06o)=%06o", REGISTER_NAME[regnum], indexval, srcval2);  // "(RN+NNNNNN)=XXXXXX"
            }
            break;
        }
    case 7:
        srcval2 = g_pBoard->GetWordView(regval + indexval, pProc->IsHaltMode(), false, &addrtype);
        hint2.sprintf("(%s+%06o)=%06o", REGISTER_NAME[regnum], indexval, srcval2);  // "(RN+NNNNNN)=XXXXXX"
        //TODO: Show the real value in hint line 3
        break;
    }
}

void QDisasmView::registerHintPC(const CProcessor *pProc,
        QString &hint1, QString& /*hint2*/,
        int regmod, bool byteword, quint16 curaddr, quint16 value)
{
    int addrtype = 0;
    quint16 srcval2 = 0;

    //TODO: else if (regmod == 2)
    if (regmod == 3)
    {
        srcval2 = g_pBoard->GetWordView(value, pProc->IsHaltMode(), false, &addrtype);
        if (byteword)
        {
            srcval2 = (value & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
            hint1.sprintf("(%06o)=%03o", value, srcval2);  // "(NNNNNN)=XXX"
        }
        else
        {
            hint1.sprintf("(%06o)=%06o", value, srcval2);  // "(NNNNNN)=XXXXXX"
        }
    }
    else if (regmod == 6)
    {
        quint16 addr2 = curaddr + value;
        srcval2 = g_pBoard->GetWordView(addr2, pProc->IsHaltMode(), false, &addrtype);
        if (byteword)
        {
            srcval2 = (addr2 & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
            hint1.sprintf("(%06o)=%03o", addr2, srcval2);  // "(NNNNNN)=XXX"
        }
        else
        {
            hint1.sprintf("(%06o)=%06o", addr2, srcval2);  // "(NNNNNN)=XXXXXX"
        }
    }
    //TODO: else if (regmod == 7)
}

void QDisasmView::instructionHint(const quint16 *memory, const CProcessor *pProc,
        QString& buffer, QString& buffer2,
        int srcreg, int srcmod, int dstreg, int dstmod)
{
    QString srchint1, dsthint1;
    QString srchint2, dsthint2;
    bool byteword = ((*memory) & 0100000) != 0;  // Byte mode (true) or Word mode (false)
    const quint16* curmemory = memory + 1;
    quint16 curaddr = pProc->GetPC() + 2;
    quint16 indexval = 0;

    if (srcreg >= 0)
    {
        if (srcreg == 7)
        {
            quint16 value = *(curmemory++);  curaddr += 2;
            registerHintPC(pProc, srchint1, srchint2, srcmod, byteword, curaddr, value);
        }
        else
        {
            if (srcmod == 6 || srcmod == 7) { indexval = *(curmemory++);  curaddr += 2; }
            registerHint(pProc, srchint1, srchint2, srcreg, srcmod, byteword, indexval);
        }
    }
    if (dstreg >= 0)
    {
        if (dstreg == 7)
        {
            quint16 value = *(curmemory++);  curaddr += 2;
            registerHintPC(pProc, dsthint1, dsthint2, dstmod, byteword, curaddr, value);
        }
        else
        {
            if (dstmod == 6 || dstmod == 7) { indexval = *(curmemory++);  curaddr += 2; }
            registerHint(pProc, dsthint1, dsthint2, dstreg, dstmod, byteword, indexval);
        }
    }

    if (!srchint1.isEmpty() && !dsthint1.isEmpty())
    {
        if (srchint1 == dsthint1)
            buffer = srchint1;
        else
            buffer = srchint1 + ", " + dsthint1;
    }
    else if (!srchint1.isEmpty())
        buffer = srchint1;
    else if (!dsthint1.isEmpty())
        buffer = dsthint1;

    if (!srchint2.isEmpty() && !dsthint2.isEmpty())
    {
        if (srchint2 == dsthint2)
            buffer2 = srchint2;
        else
            buffer2 = srchint2 + ", " + dsthint2;
    }
    else if (!srchint2.isEmpty())
        buffer2 = srchint2;
    else if (!dsthint2.isEmpty())
        buffer2 = dsthint2;
}

// Prepare "Instruction Hint" for a regular instruction (not a branch/jump one)
// Returns: number of hint lines; 0 = no hints
int QDisasmView::getInstructionHint(const quint16 *memory, const CProcessor *pProc,
        QString &buffer, QString &buffer2)
{
    buffer.clear();  buffer2.clear();
    quint16 instr = *memory;

    // Source and Destination
    if ((instr & ~(uint16_t)0107777) == PI_MOV || (instr & ~(uint16_t)0107777) == PI_CMP ||
        (instr & ~(uint16_t)0107777) == PI_BIT || (instr & ~(uint16_t)0107777) == PI_BIC || (instr & ~(uint16_t)0107777) == PI_BIS ||
        (instr & ~(uint16_t)0007777) == PI_ADD || (instr & ~(uint16_t)0007777) == PI_SUB)
    {
        int srcreg = (instr >> 6) & 7;
        int srcmod = (instr >> 9) & 7;
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        instructionHint(memory, pProc, buffer, buffer2, srcreg, srcmod, dstreg, dstmod);
    }

    // Register and Destination
    if ((instr & ~(uint16_t)0777) == PI_MUL || (instr & ~(uint16_t)0777) == PI_DIV ||
        (instr & ~(uint16_t)0777) == PI_ASH || (instr & ~(uint16_t)0777) == PI_ASHC ||
        (instr & ~(uint16_t)0777) == PI_XOR)
    {
        int srcreg = (instr >> 6) & 7;
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        instructionHint(memory, pProc, buffer, buffer2, srcreg, 0, dstreg, dstmod);
    }

    // Destination only
    if ((instr & ~(uint16_t)0100077) == PI_CLR || (instr & ~(uint16_t)0100077) == PI_COM ||
        (instr & ~(uint16_t)0100077) == PI_INC || (instr & ~(uint16_t)0100077) == PI_DEC || (instr & ~(uint16_t)0100077) == PI_NEG ||
        (instr & ~(uint16_t)0100077) == PI_TST ||
        (instr & ~(uint16_t)0100077) == PI_ASR || (instr & ~(uint16_t)0100077) == PI_ASL ||
        (instr & ~(uint16_t)077) == PI_SWAB || (instr & ~(uint16_t)077) == PI_SXT ||
        (instr & ~(uint16_t)077) == PI_MTPS || (instr & ~(uint16_t)077) == PI_MFPS)
    {
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        instructionHint(memory, pProc, buffer, buffer2, -1, -1, dstreg, dstmod);
    }

    // ADC, SBC, ROR, ROL: destination only, and also show C flag
    if ((instr & ~(uint16_t)0100077) == PI_ADC || (instr & ~(uint16_t)0100077) == PI_SBC ||
        (instr & ~(uint16_t)0100077) == PI_ROR || (instr & ~(uint16_t)0100077) == PI_ROL)
    {
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        if (dstreg != 7)
        {
            QString tempbuf;
            instructionHint(memory, pProc, tempbuf, buffer2, -1, -1, dstreg, dstmod);
            quint16 psw = pProc->GetPSW();
            buffer = tempbuf + ", C=" + ((psw & PSW_C) ? "1" : "0");  // "..., C=X"
        }
    }

    // CLC..CCC, SEC..SCC -- show flags
    if ((instr >= 0000241 && instr <= 0000257) || (instr >= 0000261 && instr <= 0000277))
    {
        uint16_t psw = pProc->GetPSW();
        buffer.sprintf("C=%c, V=%c, Z=%c, N=%c",
                (psw & PSW_C) ? '1' : '0', (psw & PSW_V) ? '1' : '0', (psw & PSW_Z) ? '1' : '0', (psw & PSW_N) ? '1' : '0');
    }

    // JSR, JMP -- show non-trivial cases only
    if ((instr & ~(uint16_t)0777) == PI_JSR && (instr & 077) != 067 && (instr & 077) != 037 ||
        (instr & ~(uint16_t)077) == PI_JMP && (instr & 077) != 067 && (instr & 077) != 037)
    {
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        instructionHint(memory, pProc, buffer, buffer2, -1, -1, dstreg, dstmod);
    }

    // HALT mode commands
    if (instr == PI_MFUS)
    {
        buffer.sprintf("R5=%06o, R0=%06o", pProc->GetReg(5), pProc->GetReg(0));  // "R5=XXXXXX, R0=XXXXXX"
    }
    if (instr == PI_MTUS)
    {
        buffer.sprintf("R0=%06o, R5=%06o", pProc->GetReg(0), pProc->GetReg(5));  // "R0=XXXXXX, R5=XXXXXX"
    }
    //TODO: MFPC, MTPC

    //TODO: MARK

    int result = 0;
    if (!buffer.isEmpty())
        result = 1;
    if (!buffer2.isEmpty())
        result = 2;
    return result;
}

// Update after Run or Step
void QDisasmView::updateData()
{
    CProcessor* pProc = g_pBoard->GetCPU();
    ASSERT(pProc != nullptr);
    m_wDisasmBaseAddr = pProc->GetPC();

    m_DisasmLineItems.clear();

    quint16 proccurrent = pProc->GetPC();
    quint16 current = m_wDisasmBaseAddr;
    quint16 previous = g_wEmulatorPrevCpuPC;

    // Read from the processor memory to the buffer
    const int nWindowSize = 30;
    quint16 memory[nWindowSize + 2];
    int addrtype[nWindowSize + 2];
    for (int idx = 0; idx < nWindowSize; idx++)
    {
        memory[idx] = g_pBoard->GetWordView(
                current + idx * 2 - 10, pProc->IsHaltMode(), true, addrtype + idx);
    }

    quint16 address = current - 10;
    quint16 disasmfrom = current;
    if ((previous >= address) && previous < current)
        disasmfrom = previous;

    // Prepare the list of lines in m_DisasmLineItems
    int lineindex = 0;
    int length = 0;
    for (int index = 0; index < nWindowSize; index++)  // Preparing lines
    {
        DisasmLineItem lineitem;
        memset(&lineitem, 0, sizeof(lineitem));
        lineitem.address = address;
        lineitem.value = memory[index];
        lineitem.addrtype = addrtype[index];

        bool okData = false;
        if (!m_SubtitleItems.isEmpty())
        {
            // Subtitles - find a comment for a block
            const DisasmSubtitleItem* pSubItem = findSubtitle(address, SUBTYPE_BLOCKCOMMENT);
            if (pSubItem != nullptr && !pSubItem->comment.isEmpty())
            {
                lineitem.type = LINETYPE_SUBTITLE;
                lineitem.pSubItem = pSubItem;
                // Opening next line
                lineindex++;
                if (lineindex >= MAX_DISASMLINECOUNT)
                    break;
                m_DisasmLineItems.append(lineitem);
                lineitem.address = address;
                lineitem.value = memory[index];
                lineitem.addrtype = addrtype[index];
            }

            // Subtitles - find a comment for an instruction or data
            pSubItem = findSubtitle(address, SUBTYPE_COMMENT | SUBTYPE_DATA);
            if (pSubItem != nullptr && (pSubItem->type & SUBTYPE_DATA) != 0)
            {
                okData = true;
                lineitem.type |= LINETYPE_DATA;
            }
            if (pSubItem != nullptr && (pSubItem->type & SUBTYPE_COMMENT) != 0 && pSubItem->comment != nullptr)
            {
                lineitem.type |= LINETYPE_SUBTITLE;
                lineitem.pSubItem = pSubItem;
                // Строку с субтитром мы можем использовать как опорную для дизассемблера
                if (disasmfrom > address)
                    disasmfrom = address;
            }
        }

        if ((lineitem.type & LINETYPE_DATA) == 0)
            lineitem.type |= LINETYPE_INSTR;  // if it's not a data then an instruction

        if (address >= disasmfrom && length == 0)
        {
            if (okData)  // We have non-instruction on the address -- no need to disassemble
            {
                length = 1;
            }
            else
            {
                lineitem.type |= LINETYPE_INSTR;
                length = DisassembleInstruction(memory + index, address, lineitem.strInstr, lineitem.strArg);

                if (m_SubtitleItems.isEmpty())  //NOTE: Subtitles can move lines down
                {
                    if (checkForJump(memory + index, &lineitem.jumpdelta))
                    {
                        lineitem.type |= LINETYPE_JUMP;
                    }

                    if (address == proccurrent)  // For current instruction, prepare the instruction hints
                    {
                        m_okDisasmJumpPredict = getJumpConditionHint(memory + index, pProc, m_strDisasmHint);
                        if (m_strDisasmHint.isEmpty())  // we don't have the jump hint
                        {
                            getInstructionHint(memory + index, pProc, m_strDisasmHint, m_strDisasmHint2);
                        }
                    }
                }
            }
        }

        m_DisasmLineItems.append(lineitem);
        if (length > 0) length--;

        address += 2;
        lineindex++;
        if (lineindex >= MAX_DISASMLINECOUNT)
            break;
    }
}

void QDisasmView::drawJump(QPainter &painter, int yFrom, int delta, int x, int cyLine, QColor color)
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

void QDisasmView::paintEvent(QPaintEvent * /*event*/)
{
    if (g_pBoard == nullptr) return;

    QColor colorBackground = palette().color(QPalette::Base);
    QPainter painter(this);
    painter.fillRect(0, 0, this->width(), this->height(), colorBackground);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cyLine = fontmetrics.height();

    CProcessor* pDisasmPU = g_pBoard->GetCPU();
    ASSERT(pDisasmPU != nullptr);

    // Draw disassembly for the current processor
    quint16 prevPC = g_wEmulatorPrevCpuPC;
    int yFocus = drawDisassemble(painter, pDisasmPU, m_wDisasmBaseAddr, prevPC);

    // Draw focus rect
    if (hasFocus())
    {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.state |= QStyle::State_KeyboardFocusChange;
        option.backgroundColor = QColor(Qt::gray);
        option.rect = QRect(0, yFocus - cyLine + fontmetrics.descent(), this->width(), cyLine);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void QDisasmView::drawBreakpoint(QPainter& painter, int x, int y, int size)
{
    QColor colorBreakpoint = qRgb(192, 0, 0);
    painter.setBrush(colorBreakpoint);
    painter.setPen(colorBreakpoint);
    painter.drawEllipse(x, y, size, -size);
    painter.setPen(palette().color(QPalette::Text));
    painter.setBrush(Qt::NoBrush);
}

int QDisasmView::drawDisassemble(QPainter &painter, CProcessor *pProc, quint16 current, quint16 previous)
{
    int result = -1;

    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.lineSpacing();
    m_cxDisasmBreakpointZone = cxChar * 2;
    m_cyDisasmLine = cyLine;
    QColor colorText = palette().color(QPalette::Text);
    QColor colorPrev = Common_GetColorShifted(palette(), COLOR_PREVIOUS);
    QColor colorChanged = Common_GetColorShifted(palette(), COLOR_VALUECHANGED);
    QColor colorValue = Common_GetColorShifted(palette(), COLOR_VALUE);
    QColor colorValueRom = Common_GetColorShifted(palette(), COLOR_VALUEROM);
    QColor colorSubtitle = Common_GetColorShifted(palette(), COLOR_SUBTITLE);
    QColor colorJump = Common_GetColorShifted(palette(), COLOR_JUMP);

    quint16 proccurrent = pProc->GetPC();

    // Draw current line background
    if (m_SubtitleItems.isEmpty())  //NOTE: Subtitles can move lines down
    {
        int yCurrent = (proccurrent - (current - 5)) * cyLine + fontmetrics.descent();
        QColor colorCurrent = palette().color(QPalette::Window);
        painter.fillRect(0, yCurrent, this->width(), cyLine, colorCurrent);
    }

    int y = cyLine;
    for (int lineindex = 0; lineindex < m_DisasmLineItems.count(); lineindex++)  // Draw the lines
    {
        DisasmLineItem& lineitem = m_DisasmLineItems[lineindex];
        if (lineitem.type == LINETYPE_NONE)
            break;
        quint16 address = lineitem.address;

        if ((lineitem.type & LINETYPE_SUBTITLE) != 0 && (lineitem.type & (LINETYPE_DATA | LINETYPE_INSTR)) == 0 &&
            lineitem.pSubItem != nullptr)  // Subtitles - comment for a block
        {
            const DisasmSubtitleItem * pSubItem = lineitem.pSubItem;

            painter.setPen(colorSubtitle);
            painter.drawText(21 * cxChar, y, pSubItem->comment);
            painter.setPen(colorText);

            y += cyLine;
            continue;
        }

        if (Emulator_IsBreakpoint(address))  // Breakpoint
        {
            drawBreakpoint(painter, cxChar / 2, y, cxChar);
        }

        painter.setPen(colorText);
        DrawOctalValue(painter, 5 * cxChar, y, address);  // Address
        // Value at the address
        quint16 value = lineitem.value;
        int memorytype = lineitem.addrtype;
        painter.setPen((memorytype == ADDRTYPE_ROM) ? colorValueRom : colorValue);
        DrawOctalValue(painter, 13 * cxChar, y, value);
        painter.setPen(colorText);

        // Current position
        if (address == current)
        {
            //painter.drawText(2 * cxChar, y, "  >");
            result = y;  // Remember line for the focus rect
        }
        if (address == proccurrent)
        {
            bool okPCchanged = proccurrent != previous;
            if (okPCchanged) painter.setPen(colorChanged);
            painter.drawText(2 * cxChar, y, "PC>");
            painter.setPen(colorText);
            painter.drawText(2 * cxChar, y, "  >");
        }
        else if (address == previous)
        {
            painter.setPen(colorPrev);
            painter.drawText(2 * cxChar, y, " > ");
        }

        int posAfterArgs = 30;
        if ((lineitem.type & (LINETYPE_DATA | LINETYPE_INSTR)) != 0)
        {
            painter.setPen(colorText);
            painter.drawText(21 * cxChar, y, lineitem.strInstr);
            painter.drawText(29 * cxChar, y, lineitem.strArg);
            posAfterArgs += (int)strlen(lineitem.strArg);
        }

        if ((lineitem.type & LINETYPE_SUBTITLE) != 0 && (lineitem.type & (LINETYPE_DATA | LINETYPE_INSTR)) != 0 &&
            lineitem.pSubItem != nullptr)  // Show subtitle comment for instruction or data
        {
            const DisasmSubtitleItem * pSubItem = lineitem.pSubItem;
            if (!pSubItem->comment.isEmpty())
            {
                painter.setPen(colorSubtitle);
                painter.drawText(52 * cxChar, y, pSubItem->comment);
                painter.setPen(colorText);
            }
        }

        if (m_SubtitleItems.isEmpty())  // We don't show jumps and hints with subtitles
        {
            bool isjump = (lineitem.type & LINETYPE_JUMP) != 0;

            if (isjump)
            {
                int delta = lineitem.jumpdelta;
                if (abs(delta) < 40)
                {
                    QColor jumpcolor = colorJump;
                    if (address == proccurrent)
                        jumpcolor = Common_GetColorShifted(palette(), m_okDisasmJumpPredict ? COLOR_JUMPYES : COLOR_JUMPNO);
                    drawJump(painter, y, delta, posAfterArgs * cxChar, cyLine, jumpcolor);
                }
            }

            if (address == proccurrent && !m_strDisasmHint.isEmpty())  // For current instruction, draw "Instruction Hints"
            {
                QColor hintcolor = Common_GetColorShifted(palette(), isjump ? COLOR_JUMPHINT : COLOR_HINT);
                painter.setPen(hintcolor);
                painter.drawText(52 * cxChar, y, m_strDisasmHint);
                if (!m_strDisasmHint2.isEmpty())
                    painter.drawText(52 * cxChar, y + cyLine, m_strDisasmHint2);
                painter.setPen(colorText);
            }
        }

        y += cyLine;
    }

    return result;
}


//////////////////////////////////////////////////////////////////////
