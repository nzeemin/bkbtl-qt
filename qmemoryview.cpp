#include "stdafx.h"
#include <QtGui>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOptionFocusRect>
#include "qmemoryview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
#include "qdialogs.h"

QMemoryView::QMemoryView()
{
    m_ByteMode = false;
    m_wBaseAddress = 0;
    m_cyLineMemory = 0;
    m_nPageSize = 0;

    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    m_cyLine = cyLine;

    this->setFont(font);
    this->setMinimumSize(cxChar * 68, cyLine * 9 + cyLine / 2);

    m_scrollbar = new QScrollBar(Qt::Vertical, this);
    m_scrollbar->setRange(0, 65536 - 16);
    m_scrollbar->setSingleStep(16);
    QObject::connect(m_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scrollValueChanged()));

    setFocusPolicy(Qt::ClickFocus);
}

QMemoryView::~QMemoryView()
{
    delete m_scrollbar;
}

void QMemoryView::updateScrollPos()
{
    m_scrollbar->setValue(m_wBaseAddress);
}

void QMemoryView::updateData()
{
}

void QMemoryView::focusInEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}
void QMemoryView::focusOutEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}

void QMemoryView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction("Go to Address...", this, SLOT(gotoAddress()));
    menu.addSeparator();
    menu.addAction("Words / Bytes", this, SLOT(changeWordByteMode()));

    menu.exec(event->globalPos());
}

void QMemoryView::changeWordByteMode()
{
    m_ByteMode = !m_ByteMode;
    repaint();
}

void QMemoryView::scrollBy(quint16 delta)
{
    if (delta == 0) return;

    m_wBaseAddress = (WORD)(m_wBaseAddress + delta);
    m_wBaseAddress = m_wBaseAddress & ((WORD)~15);
    repaint();
    updateScrollPos();
}

void QMemoryView::gotoAddress()
{
    WORD value = m_wBaseAddress;
    QInputOctalDialog dialog(this, "Go To Address", "Address (octal):", &value);
    if (dialog.exec() == QDialog::Rejected) return;

    // Scroll to the address
    m_wBaseAddress = value & ((WORD)~15);
    repaint();
    updateScrollPos();
}

void QMemoryView::resizeEvent(QResizeEvent *)
{
    int cxScroll = this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    m_scrollbar->setGeometry(this->width() - cxScroll, 0, cxScroll, this->height());
    m_scrollbar->setPageStep((this->height() / m_cyLine - 2) * 16);
}

void QMemoryView::scrollValueChanged()
{
    int value = m_scrollbar->value();
    m_wBaseAddress = (unsigned short)value & ((WORD)~15);
    this->repaint();
}

void QMemoryView::paintEvent(QPaintEvent * /*event*/)
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

    TCHAR buffer[7];
    const TCHAR* ADDRESS_LINE = _T(" address  0      2      4      6      10     12     14     16");
    painter.drawText(0, cyLine, ADDRESS_LINE);

    // Calculate m_nPageSize
    m_nPageSize = this->height() / cyLine - 1;

    WORD address = m_wBaseAddress;
    int y = 2 * cyLine;
    for (;;) {  // Draw lines
        DrawOctalValue(painter, 2 * cxChar, y, address);

        int x = 10 * cxChar;
        ushort wchars[16];

        for (int j = 0; j < 8; j++) {  // Draw words as octal value
            // Get word from memory
            WORD word = 0;
            int addrtype;
            BOOL okHalt = FALSE;
            WORD wChanged = 0;

            okHalt = pDebugPU->IsHaltMode();
            word = g_pBoard->GetWordView(address, okHalt, FALSE, &addrtype);
            wChanged = Emulator_GetChangeRamStatus(address);

            if ((addrtype & (ADDRTYPE_IO | ADDRTYPE_DENY)) == 0)
            {
                painter.setPen(wChanged != 0 ? Qt::red : colorText);
                if (m_ByteMode)
                {
                    PrintOctalValue(buffer, (word & 0xff));
                    painter.drawText(x, y, buffer + 3);
                    PrintOctalValue(buffer, (word >> 8));
                    painter.drawText(x + 3 * cxChar + cxChar / 2, y, buffer + 3);
                }
                else
                    DrawOctalValue(painter, x, y, word);
            }

            // Prepare characters to draw at right
            BYTE ch1 = LOBYTE(word);
            ushort wch1 = Translate_BK_Unicode(ch1);
            if (ch1 < 32) wch1 = 0x00b7;
            wchars[j * 2] = wch1;
            BYTE ch2 = HIBYTE(word);
            ushort wch2 = Translate_BK_Unicode(ch2);
            if (ch2 < 32) wch2 = 0x00b7;
            wchars[j * 2 + 1] = wch2;

            address += 2;
            x += 7 * cxChar;
        }
        painter.setPen(colorText);

        // Draw characters at right
        int xch = x + cxChar;
        QString wstr = QString::fromUtf16(wchars, 16);
        painter.drawText(xch, y, wstr);

        y += cyLine;
        if (y > this->height()) break;
    }  // Draw lines

    // Draw focus rect
    if (hasFocus())
    {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.state |= QStyle::State_KeyboardFocusChange;
        option.backgroundColor = QColor(Qt::gray);
        option.rect = QRect(0, cyLine + 1, 85 * cxChar, cyLine * m_nPageSize);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void QMemoryView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_G:
        event->accept();
        gotoAddress();
        break;
    case Qt::Key_B:
        event->accept();
        changeWordByteMode();
        break;

    case Qt::Key_Up:
        event->accept();
        scrollBy(-16);
        break;
    case Qt::Key_Down:
        event->accept();
        scrollBy(16);
        break;

    case Qt::Key_PageUp:
        event->accept();
        scrollBy(-m_nPageSize * 16);
        break;
    case Qt::Key_PageDown:
        event->accept();
        scrollBy(m_nPageSize * 16);
        break;
    }
}

void QMemoryView::wheelEvent(QWheelEvent * event)
{
    if (event->orientation() == Qt::Horizontal)
        return;
    event->accept();

    int steps = -event->delta() / 60;
    scrollBy(steps * 16);
}
