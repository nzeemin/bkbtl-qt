#include "stdafx.h"
#include <QtGui>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOptionFocusRect>
#include <QToolBar>
#include "main.h"
#include "qmemoryview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
#include "qdialogs.h"


//////////////////////////////////////////////////////////////////////


enum MemoryViewNumeralMode
{
    MEMMODENUM_OCT = 0,
    MEMMODENUM_HEX = 1,
};


QMemoryView::QMemoryView()
{
    m_ByteMode = Settings_GetDebugMemoryByte();
    m_NumeralMode = Settings_GetDebugMemoryNumeral();
    m_wBaseAddress = Settings_GetDebugMemoryAddress();
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

    m_toolbar = new QToolBar(this);
    m_toolbar->setGeometry(4, 4, 36, 2000);
    m_toolbar->setOrientation(Qt::Vertical);
    m_toolbar->setIconSize(QSize(24, 24));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setFocusPolicy(Qt::NoFocus);
    m_toolbar->setStyle(QStyleFactory::create("windows"));  // fix for macOS to remove gradient background

    QAction* actionGotoAddr = m_toolbar->addAction(QIcon(":/images/iconEditAddress.svg"), "");
    m_toolbar->addSeparator();
    QAction* actionWordByte = m_toolbar->addAction(QIcon(":/images/iconWordByte.svg"), "");
    QAction* actionOctalHex = m_toolbar->addAction(QIcon(":/images/iconHex.svg"), "");

    QObject::connect(actionGotoAddr, SIGNAL(triggered()), this, SLOT(gotoAddress()));
    QObject::connect(actionWordByte, SIGNAL(triggered()), this, SLOT(changeWordByteMode()));
    QObject::connect(actionOctalHex, SIGNAL(triggered()), this, SLOT(changeNumeralMode()));

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
    menu.addAction(tr("Go to Address..."), this, SLOT(gotoAddress()));
    menu.addSeparator();
    menu.addAction(tr("Words / Bytes"), this, SLOT(changeWordByteMode()));
    menu.addAction(tr("Octal / Hex"), this, SLOT(changeNumeralMode()));

    menu.exec(event->globalPos());
}

void QMemoryView::changeWordByteMode()
{
    m_ByteMode = !m_ByteMode;
    Settings_SetDebugMemoryByte(m_ByteMode);

    repaint();
}

void QMemoryView::changeNumeralMode()
{
    int newMode = m_NumeralMode ^ 1;
    m_NumeralMode = newMode;
    Settings_SetDebugMemoryNumeral(newMode);

    repaint();
}

void QMemoryView::scrollBy(qint16 delta)
{
    if (delta == 0) return;

    m_wBaseAddress = (quint16)(m_wBaseAddress + delta);
    m_wBaseAddress = m_wBaseAddress & ((quint16)~15);
    Settings_SetDebugMemoryAddress(m_wBaseAddress);

    repaint();
    updateScrollPos();
}

void QMemoryView::gotoAddress()
{
    quint16 value = m_wBaseAddress;
    QInputOctalDialog dialog(this, tr("Go To Address"), &value);
    if (dialog.exec() == QDialog::Rejected) return;

    // Scroll to the address
    m_wBaseAddress = value & ((quint16)~15);
    Settings_SetDebugMemoryAddress(m_wBaseAddress);

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
    m_wBaseAddress = (unsigned short)value & ((quint16)~15);
    Settings_SetDebugMemoryAddress(m_wBaseAddress);

    this->repaint();
}

void QMemoryView::paintEvent(QPaintEvent * /*event*/)
{
    if (g_pBoard == nullptr) return;

    QColor colorBackground = palette().color(QPalette::Base);

    QPainter painter(this);
    painter.fillRect(36 + 4, 0, this->width(), this->height(), colorBackground);

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = palette().color(QPalette::Text);
    QColor colorChanged = Common_GetColorShifted(palette(), COLOR_VALUECHANGED);
    QColor colorMemoryRom = Common_GetColorShifted(palette(), COLOR_MEMORYROM);
    QColor colorMemoryIO = Common_GetColorShifted(palette(), COLOR_MEMORYIO);
    QColor colorMemoryNA = Common_GetColorShifted(palette(), COLOR_MEMORYNA);

    CProcessor* pDebugPU = g_pBoard->GetCPU();
    ASSERT(pDebugPU != nullptr);

    m_cyLineMemory = cyLine;

    if (m_NumeralMode == MEMMODENUM_OCT)
        m_PostionIncrement = cxChar * 7;
    else
        m_PostionIncrement = cxChar * 5;
    if (m_ByteMode)
        m_PostionIncrement += cxChar;

    char buffer[7];
    const char* ADDRESS_LINE_OCT_WORDS = "   addr  0      2      4      6      10     12     14     16";
    const char* ADDRESS_LINE_OCT_BYTES = "   addr  0   1   2   3   4   5   6   7   10  11  12  13  14  15  16  17";
    const char* ADDRESS_LINE_HEX_WORDS = "   addr  0    2    4    6    8    a    c    e";
    const char* ADDRESS_LINE_HEX_BYTES = "   addr  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f";
    if (m_NumeralMode == MEMMODENUM_OCT && !m_ByteMode)
        painter.drawText(38, cyLine, ADDRESS_LINE_OCT_WORDS);
    else if (m_NumeralMode == MEMMODENUM_OCT && m_ByteMode)
        painter.drawText(38, cyLine, ADDRESS_LINE_OCT_BYTES);
    else if (m_ByteMode)
        painter.drawText(38, cyLine, ADDRESS_LINE_HEX_BYTES);
    else
        painter.drawText(38, cyLine, ADDRESS_LINE_HEX_WORDS);

    // Calculate m_nPageSize
    m_nPageSize = this->height() / cyLine - 1;

    quint16 address = m_wBaseAddress;
    int y = 2 * cyLine;
    for (;;)    // Draw lines
    {
        if (m_NumeralMode == MEMMODENUM_OCT)
            DrawOctalValue(painter, 38 + 1 * cxChar, y, address);
        else
            DrawHexValue(painter, 38 + 3 * cxChar, y, address);

        int x = 38 + 9 * cxChar;
        ushort wchars[16];
        for (int j = 0; j < 8; j++)    // Draw words as octal value
        {
            // Get word from memory
            quint16 word = 0;
            int addrtype;
            bool okHalt = false;
            quint16 wChanged = 0;

            okHalt = pDebugPU->IsHaltMode();
            word = g_pBoard->GetWordView(address, okHalt, false, &addrtype);
            wChanged = Emulator_GetChangeRamStatus(address);

            if ((addrtype & (ADDRTYPE_IO | ADDRTYPE_DENY)) == 0)
            {
                if (addrtype == ADDRTYPE_ROM)
                    painter.setPen(colorMemoryRom);
                else
                    painter.setPen(wChanged != 0 ? colorChanged : colorText);

                if (m_NumeralMode == MEMMODENUM_OCT && !m_ByteMode)
                    DrawOctalValue(painter, x, y, word);
                else if (m_NumeralMode == MEMMODENUM_OCT && m_ByteMode)
                {
                    PrintOctalValue(buffer, (word & 0xff));
                    painter.drawText(x, y, buffer + 3);
                    PrintOctalValue(buffer, (word >> 8));
                    painter.drawText(x + 4 * cxChar, y, buffer + 3);
                }
                else if (m_NumeralMode == MEMMODENUM_HEX && !m_ByteMode)
                    DrawHexValue(painter, x, y, word);
                else if (m_NumeralMode == MEMMODENUM_HEX && m_ByteMode)
                {
                    PrintHexValue(buffer, word);
                    painter.drawText(x, y, buffer + 2);
                    buffer[2] = 0;
                    painter.drawText(x + 3 * cxChar, y, buffer);
                }
            }
            else  // No value
            {
                if (addrtype == ADDRTYPE_IO)
                {
                    painter.setPen(colorMemoryIO);
                    painter.drawText(x, y, "  IO");
                }
                else
                {
                    painter.setPen(colorMemoryNA);
                    painter.drawText(x, y, "  NA");
                }
            }

            // Prepare characters to draw at right
            quint8 ch1 = (quint8)(word & 0xff); // LOBYTE
            ushort wch1 = Translate_BK_Unicode(ch1);
            if (ch1 < 32) wch1 = 0x00b7;
            wchars[j * 2] = wch1;
            quint8 ch2 = (quint8)((word >> 8) & 0xff); // HIBYTE
            ushort wch2 = Translate_BK_Unicode(ch2);
            if (ch2 < 32) wch2 = 0x00b7;
            wchars[j * 2 + 1] = wch2;

            address += 2;
            x += m_PostionIncrement;
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
        option.rect = QRect(38, cyLine + fontmetrics.descent(), 28 * cxChar + 8 * m_PostionIncrement, cyLine * m_nPageSize);
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
    case Qt::Key_W:
        event->accept();
        changeWordByteMode();
        break;
    case Qt::Key_H:
    case Qt::Key_O:
        event->accept();
        changeNumeralMode();
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


//////////////////////////////////////////////////////////////////////
