#include "stdafx.h"
#include <QtGui>
#include <QFileDialog>
#include <QMenu>
#include <QStyleOptionFocusRect>
#include "main.h"
#include "qdisasmview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


const int MAX_DISASMLINECOUNT = 50;
const int HINT_CHARPOS = 52;
const int DISASMVIEW_WINDOW_WORDS = 30;  // The window of the memory read for the rows, in words
const int DISASMVIEW_ROWS_BEFORE_PC = 5;  // The window starts this many words before PC, so the PC row is at this index

static_assert(DISASMVIEW_ROWS_BEFORE_PC + 2 < DISASMVIEW_WINDOW_WORDS, "The PC row and its words must be in the window");


//////////////////////////////////////////////////////////////////////


QDisasmView::QDisasmView()
{
    m_wDisasmBaseAddr = 0;
    m_wDisasmNextBaseAddr = 0;
    m_cxDisasmBreakpointZone = 16;
    m_cyDisasmLine = 10;
    m_lastHitValid = false;
    m_lastHitAddress = m_lastHitValue = 0;

    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    this->setMinimumSize(cxChar * 55, cyLine * 10 + cyLine / 2);

    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);
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

    if (hitTest(event->y()))
    {
        char bufaddr[7], bufval[7];
        PrintOctalValue(bufaddr, m_lastHitAddress);
        PrintOctalValue(bufval, m_lastHitValue);
        menu.addAction(tr("Copy Address: %1").arg(bufaddr), this, SLOT(copyAddressOctal()));
        menu.addAction(tr("Copy Value: %1").arg(bufval), this, SLOT(copyValueOctal()));
        menu.addSeparator();
    }

    menu.addAction(m_SubtitleItems.isEmpty() ? tr("Show Subtitles...") : tr("Hide Subtitles"), this, SLOT(showHideSubtitles()));
    menu.exec(event->globalPos());
}

// Hit test for the context menu: returns true and fills m_lastHit* if y is over a disasm line
bool QDisasmView::hitTest(int y)
{
    m_lastHitValid = false;

    int lineindex = y / m_cyDisasmLine;
    if (lineindex < 0 || lineindex >= m_DisasmLineItems.count())
        return false;

    const DisasmLineItem& lineitem = m_DisasmLineItems[lineindex];
    if ((lineitem.type & (LINETYPE_DATA | LINETYPE_INSTR)) == 0)
        return false;  // Not a line with address and value

    m_lastHitAddress = lineitem.address;
    m_lastHitValue = lineitem.value;
    m_lastHitValid = true;
    return true;
}

void QDisasmView::copyAddressOctal()
{
    if (!m_lastHitValid)
        return;
    CopyWordOctalToClipboard(m_lastHitAddress);
}

void QDisasmView::copyValueOctal()
{
    if (!m_lastHitValid)
        return;
    CopyWordOctalToClipboard(m_lastHitValue);
}

void QDisasmView::mouseMoveEvent(QMouseEvent * event)
{
    if (event->x() < m_cxDisasmBreakpointZone)
        setCursor(QCursor(Qt::PointingHandCursor));
    else
        setCursor(QCursor(Qt::ArrowCursor));

    QWidget::mouseMoveEvent(event);
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
                    Global_RedrawDebugView();
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
        dlg.setNameFilter(tr("Disasm subtitles (*.lst)"));
        if (dlg.exec() == QDialog::Rejected)
            return;
        QString fileName = dlg.selectedFiles().at(0);

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            AlertWarning(tr("Failed to open the subtitles file."));
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

// Update after Run or Step
void QDisasmView::updateData()
{
    CProcessor* pProc = g_pBoard->GetCPU();
    ASSERT(pProc != nullptr);
    m_wDisasmBaseAddr = pProc->GetPC();

    m_DisasmLineItems.clear();
    m_Hint.count = 0;
    m_okDisasmJumpPredict = false;

    quint16 proccurrent = pProc->GetPC();
    quint16 current = m_wDisasmBaseAddr;
    quint16 previous = g_wEmulatorPrevCpuPC;

    // Read from the processor memory to the buffer; the two words after the window are for the instructions
    // in the last rows, which can have extra words. The addresses wrap around 16 bits, so the positions in
    // the window are word indexes, counted from the window start; no comparison of the addresses.
    const int nWindowSize = DISASMVIEW_WINDOW_WORDS;
    quint16 memory[nWindowSize + 2];
    int addrtype[nWindowSize + 2];
    quint16 windowStart = (quint16)(current - DISASMVIEW_ROWS_BEFORE_PC * 2);
    for (int idx = 0; idx < nWindowSize + 2; idx++)
    {
        memory[idx] = g_pBoard->GetWordView(
                (quint16)(windowStart + idx * 2), true, addrtype + idx);
    }

    quint16 address = windowStart;

    // The index of the word where decoding starts: the PC row, or the previous PC when the decoding
    // from there lands exactly on the PC row. Otherwise the walk can run through the PC row as an operand word
    // of the previous instruction, and the PC row would get no mnemonic and no hint.
    int disasmfrom = DISASMVIEW_ROWS_BEFORE_PC;
    quint16 previousOffset = (quint16)(previous - windowStart);  // in bytes
    if (previousOffset < DISASMVIEW_ROWS_BEFORE_PC * 2)
    {
        int walk = previousOffset / 2;
        while (walk < DISASMVIEW_ROWS_BEFORE_PC)
        {
            quint16 walkAddress = (quint16)(windowStart + walk * 2);
            // A row that the subtitles mark as data is one word, as in the main loop below
            bool okWalkData = false;
            if (!m_SubtitleItems.isEmpty())
            {
                const DisasmSubtitleItem* pWalkSub = findSubtitle(walkAddress, SUBTYPE_COMMENT | SUBTYPE_DATA);
                okWalkData = (pWalkSub != nullptr && (pWalkSub->type & SUBTYPE_DATA) != 0);
            }
            int walkLength = 1;
            if (!okWalkData)
            {
                char strInstr[16];
                char strArg[40];
                walkLength = DisassembleInstruction(memory + walk, walkAddress, strInstr, strArg);
                if (walkLength < 1) walkLength = 1;
            }
            walk += walkLength;
        }
        if (walk == DISASMVIEW_ROWS_BEFORE_PC)
            disasmfrom = previousOffset / 2;
    }

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
                if (disasmfrom > index)
                    disasmfrom = index;
            }
        }

        if ((lineitem.type & LINETYPE_DATA) == 0)
            lineitem.type |= LINETYPE_INSTR;  // if it's not a data then an instruction

        if (index >= disasmfrom && length == 0)
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
                    if (Disasm_CheckForJump(memory + index, address, &lineitem.jumpdelta))
                    {
                        lineitem.type |= LINETYPE_JUMP;
                    }

                    if (address == proccurrent)  // For current instruction, prepare the instruction hints
                    {
                        Disasm_GetHint(memory + index, pProc, g_pBoard, &m_Hint);
                        m_okDisasmJumpPredict = m_Hint.jumpPredict;
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
    QColor colorBreakpoint = Common_GetColorShifted(palette(), COLOR_BREAKPOINT);
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
    int xHint = HINT_CHARPOS * cxChar;
    QColor colorBackground = palette().color(QPalette::Base);
    QColor colorText = palette().color(QPalette::Text);
    QColor colorPrev = Common_GetColorShifted(palette(), COLOR_PREVIOUS);
    QColor colorChanged = Common_GetColorShifted(palette(), COLOR_VALUECHANGED);
    QColor colorValue = Common_GetColorShifted(palette(), COLOR_VALUE);
    QColor colorValueRom = Common_GetColorShifted(palette(), COLOR_VALUEROM);
    QColor colorSubtitle = Common_GetColorShifted(palette(), COLOR_SUBTITLE);
    QColor colorJump = Common_GetColorShifted(palette(), COLOR_JUMP);
    QColor colorWindow = palette().color(QPalette::Window);

    quint16 proccurrent = pProc->GetPC();
    // The window is always built around PC (updateData), so the PC row is the row DISASMVIEW_ROWS_BEFORE_PC:
    // the current-line bar and the PC marker rely on this
    ASSERT(current == proccurrent);

    // Draw breakpoint zone
    painter.fillRect(0, 0, m_cxDisasmBreakpointZone, this->height(), colorWindow);

    // Draw current line background
    if (m_SubtitleItems.isEmpty())  //NOTE: Subtitles can move lines down
    {
        int yCurrent = DISASMVIEW_ROWS_BEFORE_PC * cyLine + fontmetrics.descent();
        painter.fillRect(0, yCurrent, xHint, cyLine, colorWindow);
    }

    int y = cyLine;
    for (int lineindex = 0; lineindex < m_DisasmLineItems.count(); lineindex++)  // Draw the lines
    {
        const DisasmLineItem& lineitem = m_DisasmLineItems[lineindex];
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
            drawBreakpoint(painter, 0, y, (cxChar + cyLine) / 2);
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

            if (address == proccurrent)
            {
                int hintLines = m_Hint.count > 0 ? m_Hint.count : 1;
                int cyHint = cyLine * hintLines;
                int cxHint = 0;  // Width of the longest hint line
                for (int i = 0; i < m_Hint.count; i++)
                    cxHint = qMax(cxHint, fontmetrics.horizontalAdvance(QString::fromLatin1(m_Hint.lines[i])));
                cxHint = qMax(cxHint, 24 * cxChar);
                QLinearGradient gradient(xHint - cxChar, 0, xHint + 25 * cxChar, 0);
                gradient.setColorAt(0, colorWindow);
                gradient.setColorAt(1, colorBackground);
                painter.fillRect(xHint - cxChar, y - cyLine + fontmetrics.descent(), 25 * cxChar, cyHint, gradient);
                painter.fillRect(xHint + 24 * cxChar, y - cyLine + fontmetrics.descent(), cxHint - 24 * cxChar, cyHint, colorBackground);

                if (m_Hint.count > 0)  // For current instruction, draw "Instruction Hints"
                {
                    QColor hintcolor = Common_GetColorShifted(palette(), m_Hint.isJump ? COLOR_JUMPHINT : COLOR_HINT);
                    painter.setPen(hintcolor);
                    for (int i = 0; i < m_Hint.count; i++)
                        painter.drawText(xHint, y + i * cyLine, m_Hint.lines[i]);
                    painter.setPen(colorText);
                }
            }
        }

        y += cyLine;
    }

    return result;
}


//////////////////////////////////////////////////////////////////////
