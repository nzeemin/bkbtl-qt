#include "stdafx.h"
#include <QtGui>
#include <QScrollBar>
#include "main.h"
#include "qinstrhistoryview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


//////////////////////////////////////////////////////////////////////


QInstrHistoryView::QInstrHistoryView()
{
    m_lastIndex = 0;

    m_scrollBar = new QScrollBar(Qt::Vertical, this);
    m_scrollBar->setVisible(true);  // Always visible, disabled when there is nothing to scroll
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarChanged(int)));
}

// Update after Run or Step: take the snapshot of the history ring
void QInstrHistoryView::updateData()
{
    int length = Emulator_GetHistoryLength();
    m_snapshot.resize(length);
    for (int i = 0; i < length; i++)
        m_snapshot[i] = Emulator_GetHistoryRecord(i);
    m_lastIndex = 0;  // Every update (stop, step, reset) scrolls back to the most recent instruction
    updateScrollBar();
}

int QInstrHistoryView::rowCount() const
{
    QFontMetrics fontmetrics(Common_GetMonospacedFont());
    int rows = this->height() / fontmetrics.height();
    return rows > 0 ? rows : 1;
}

int QInstrHistoryView::maxIndex() const
{
    int max = m_snapshot.size() - rowCount();
    return max > 0 ? max : 0;
}

// The scrollbar top is the oldest instruction, the bottom is the most recent one
void QInstrHistoryView::updateScrollBar()
{
    int max = maxIndex();
    m_scrollBar->blockSignals(true);
    m_scrollBar->setRange(0, max);
    m_scrollBar->setPageStep(rowCount());
    m_scrollBar->setSingleStep(1);
    m_scrollBar->setValue(max - m_lastIndex);
    m_scrollBar->blockSignals(false);
}

void QInstrHistoryView::setLastIndex(int index)
{
    int max = maxIndex();
    if (index > max) index = max;
    if (index < 0) index = 0;
    if (index != m_lastIndex)
    {
        m_lastIndex = index;
        update();
    }
    updateScrollBar();
}

void QInstrHistoryView::scrollBarChanged(int value)
{
    setLastIndex(maxIndex() - value);
}

void QInstrHistoryView::wheelEvent(QWheelEvent *event)
{
    int steps = event->angleDelta().y() / 40;  // 120 units per notch = 3 rows
    if (steps == 0)
        return;
    event->accept();
    setLastIndex(m_lastIndex + steps);  // Wheel up shows older instructions
}

void QInstrHistoryView::resizeEvent(QResizeEvent * /*event*/)
{
    int sbWidth = m_scrollBar->sizeHint().width();
    m_scrollBar->setGeometry(this->width() - sbWidth, 0, sbWidth, this->height());
    setLastIndex(m_lastIndex);  // Re-clamp for the new number of rows, refresh the scrollbar
}

// Lines go from bottom to top: the most recent instruction is at the bottom
void QInstrHistoryView::paintEvent(QPaintEvent * /*event*/)
{
    QColor colorBackground = palette().color(QPalette::Base);
    QPainter painter(this);
    painter.fillRect(0, 0, this->width(), this->height(), colorBackground);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics = painter.fontMetrics();
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = palette().color(QPalette::Text);
    QColor colorInterrupt = Common_GetColorShifted(palette(), COLOR_JUMPHINT);  // Until it gets its own color
    painter.setPen(colorText);

    int x = cxChar;
    int y = this->height() - fontmetrics.descent() - cyLine / 4;  // Baseline of the bottom line

    for (int index = m_lastIndex; index < m_snapshot.size(); index++)
    {
        if (y + fontmetrics.descent() <= 0)
            break;  // The rest of the lines are above the top of the widget

        const InstructionRecord& record = m_snapshot[index];

        char bufaddr[7];
        PrintOctalValue(bufaddr, record.pc);

        char buffer[80];
        if (record.interrupt)  // words: vector, new PC, new PSW
        {
            _snprintf(buffer, sizeof(buffer) - 1, "%3d %s INT %06o %s -> %06o",
                    index, bufaddr, record.words[0], Emulator_GetInterruptName(record.words[0]), record.words[1]);
        }
        else
        {
            char strInstr[8];
            char strArg[32];
            DisassembleInstruction(record.words, record.pc, strInstr, strArg);
            _snprintf(buffer, sizeof(buffer) - 1, "%3d %s %s %s", index, bufaddr, strInstr, strArg);
        }
        buffer[sizeof(buffer) - 1] = 0;
        painter.setPen(record.interrupt ? colorInterrupt : colorText);
        painter.drawText(x, y, buffer);

        y -= cyLine;
    }
}

