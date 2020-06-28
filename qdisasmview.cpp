#include "stdafx.h"
#include <QtGui>
#include <QFileDialog>
#include <QMenu>
#include <QStyleOptionFocusRect>
#include "qdisasmview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


//////////////////////////////////////////////////////////////////////


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

void QDisasmView::updateWindowText()
{
    QString buffer = QString("Disassemble");
    if (!m_SubtitleItems.isEmpty())
        buffer.append(tr(" - Subtitles"));
    parentWidget()->setWindowTitle(buffer);
}

void QDisasmView::updateData()
{
    CProcessor* pDisasmPU = g_pBoard->GetCPU();
    ASSERT(pDisasmPU != nullptr);
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
        dlg.setNameFilter("BKBTL subtitles (*.lst)");
        if (dlg.exec() == QDialog::Rejected)
            return;
        QString fileName = dlg.selectedFiles().at(0);

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            AlertWarning("Failed to open the file.");
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

int QDisasmView::drawDisassemble(QPainter &painter, CProcessor *pProc, quint16 base, quint16 previous)
{
    int result = -1;

    QFontMetrics fontmetrics(painter.font());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.lineSpacing();
    QColor colorText = palette().color(QPalette::Text);
    QColor colorRed = Common_GetColorShifted(palette(), COLOR_RED);
    QColor colorBlue = Common_GetColorShifted(palette(), COLOR_BLUE);
    QColor colorHint = Common_GetColorShifted(palette(), COLOR_HINT);
    QColor colorJumpHint = Common_GetColorShifted(palette(), COLOR_JUMPHINT);
    QColor colorJump = Common_GetColorShifted(palette(), COLOR_JUMP);
    QColor colorJumpYes = Common_GetColorShifted(palette(), COLOR_JUMPYES);
    QColor colorJumpGray = Common_GetColorShifted(palette(), COLOR_JUMPGRAY);
    QColor colorSubtitle = Common_GetColorShifted(palette(), COLOR_SUBTITLE);

    quint16 proccurrent = pProc->GetPC();
    quint16 current = base;

    // Draw current line background
    if (m_SubtitleItems.isEmpty())  //NOTE: Subtitles can move lines down
    {
        QColor colorCurrent = palette().color(QPalette::Window);
        int yCurrent = (proccurrent - (current - 5)) * cyLine + fontmetrics.descent();
        painter.fillRect(0, yCurrent, this->width(), cyLine, colorCurrent);
    }

    // Читаем из памяти процессора в буфер
    const int nWindowSize = 30; //this->height() / cyLine;
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

    int length = 0;
    quint16 wNextBaseAddr = 0;
    int y = cyLine;
    for (int index = 0; index < nWindowSize; index++)  // Рисуем строки
    {
        if (!m_SubtitleItems.isEmpty())  // Subtitles - комментарий к блоку
        {
            const DisasmSubtitleItem * pSubItem = findSubtitle(address, SUBTYPE_BLOCKCOMMENT);
            if (pSubItem != nullptr && !pSubItem->comment.isEmpty())
            {
                painter.setPen(colorSubtitle);
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
            painter.drawText(1 * cxChar, y, "  >");
            result = y;  // Remember line for the focus rect
        }
        if (address == proccurrent)
        {
            bool okPCchanged = proccurrent != previous;
            if (okPCchanged) painter.setPen(colorRed);
            painter.drawText(1 * cxChar, y, "PC");
            painter.setPen(colorText);
            painter.drawText(3 * cxChar, y, ">>");
        }
        else if (address == previous)
        {
            painter.setPen(colorBlue);
            painter.drawText(1 * cxChar, y, "  >");
        }

        bool okData = false;
        if (!m_SubtitleItems.isEmpty())  // Show subtitle
        {
            const DisasmSubtitleItem* pSubItem = findSubtitle(address, SUBTYPE_COMMENT | SUBTYPE_DATA);
            if (pSubItem != nullptr && (pSubItem->type & SUBTYPE_DATA) != 0)
                okData = true;
            if (pSubItem != nullptr && (pSubItem->type & SUBTYPE_COMMENT) != 0 && !pSubItem->comment.isEmpty())
            {
                painter.setPen(colorSubtitle);
                painter.drawText(52 * cxChar, y, pSubItem->comment);
                painter.setPen(colorText);

                // Строку с субтитром мы можем использовать как опорную для дизассемблера
                if (disasmfrom > address)
                    disasmfrom = address;
            }
        }

        if (address >= disasmfrom && length == 0)
        {
            char strInstr[8];
            char strArg[32];
            if (okData)  // По этому адресу лежат данные -- нет смысла дизассемблировать
            {
                strcpy(strInstr, "data");
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
                        drawJump(painter, y, delta, (30 + strlen(strArg)) * cxChar, cyLine, colorJump);

                    if (address == proccurrent)
                    {
                        // For current instruction, draw "Instruction Hint"
                        QString strHint, strHint2;
                        bool jumppredict = getJumpConditionHint(memory + index, pProc, strHint);
                        if (!strHint.isEmpty())  // If we have the hint
                        {
                            painter.setPen(colorJumpHint);
                            painter.drawText(48 * cxChar, y, strHint);
                        }
                        else
                        {
                            int hint = getInstructionHint(memory + index, pProc, strHint, strHint2);
                            if (hint > 0)
                            {
                                painter.setPen(colorHint);
                                painter.drawText(52 * cxChar, y, strHint);
                                if (!strHint2.isEmpty())
                                    painter.drawText(52 * cxChar, y + cyLine, strHint2);
                                painter.setPen(colorText);
                            }
                        }

                        if (isjump && abs(delta) < 40)
                        {
                            QColor jumpcolor = jumppredict ? colorJumpYes : colorJumpGray;
                            drawJump(painter, y, delta, (30 + strlen(strArg)) * cxChar, cyLine, jumpcolor);
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


//////////////////////////////////////////////////////////////////////
