#include "stdafx.h"
#include <QtGui>
#include <QPalette>
#include "qkeyboardview.h"
#include "Emulator.h"


//////////////////////////////////////////////////////////////////////


const int KEYBOARD_KEYS_ARRAY_WIDTH = 5;
// Keyboard key mapping to bitmap
const unsigned short m_arrKeyboardKeys[] = {
/*   x1,y1    w,h      code  AR2code  */
      3,  3, 51,34,    BK_KEY_REPEAT,  // ����
     56,  3, 51,34,    0003,  // ��
    109,  3, 51,34,    0231,  // Arrow right    =|=>|
    162,  3, 51,34,    0026,  // Arrow left     |<===
    215,  3, 51,34,    0027,  // Arrow right    |===>
    268,  3, 51,34,    0202,  // ��� ��
    321,  3, 51,34,    0204,  // ���� ���
    374,  3, 50,34,    0220,  // STEP
    426,  3, 51,34,    0014,  // ���
    478,  3, 70,34,    BK_KEY_STOP,  // STOP

      3,38,  34,34,    BK_KEY_BACKSHIFT,  // Big Arrow Down
     38,38,  34,34,    0073,  // ; +
     74,38,  34,34,    0061,  // 1 !
    109,38,  34,34,    0062,  // 2 "
    144,38,  34,34,    0063,  // 3 #
    179,38,  34,34,    0064,  // 4 $
    214,38,  35,34,    0065,  // 5 %
    250,38,  34,34,    0066,  // 6 &
    285,38,  34,34,    0067,  // 7 '
    320,38,  34,34,    0070,  // 8 (
    355,38,  34,34,    0071,  // 9 )
    391,38,  34,34,    0060,  // 0 {
    426,38,  34,34,    0055,  // - =
    461,38,  34,34,    0057,  // / ?
    498,38,  50,34,    0030,  // Backspace

      4,73,  50,34,    0015,  // TAB
     56,73,  34,34,    0112,  // � J
     91,73,  34,34,    0103,  // � C
    126,73,  35,34,    0125,  // � U
    162,73,  34,34,    0113,  // � K
    197,73,  34,34,    0105,  // � E
    232,73,  34,34,    0116,  // � N
    267,73,  34,34,    0107,  // � G
    302,73,  35,34,    0133,  // � [
    338,73,  34,34,    0135,  // � ]
    373,73,  34,34,    0132,  // � Z
    408,73,  34,34,    0110,  // � H
    443,73,  35,34,    0072,  // : *
    479,73,  34,34,    0137,  // � }
    514,73,  34,34,    0023, // ��

     12,109, 50,34,    0000,  // ��
     64,109, 34,34,    0106,  // � F
     99,109, 35,34,    0131,  // � Y
    135,109, 34,34,    0127,  // � W
    170,109, 34,34,    0101,  // � A
    205,109, 34,34,    0120,  // � P
    240,109, 34,34,    0122,  // � R
    275,109, 35,34,    0117,  // � O
    311,109, 34,34,    0114,  // � L
    346,109, 34,34,    0104,  // � D
    381,109, 34,34,    0126,  // � V
    416,109, 34,34,    0134,  // � Backslash
    452,109, 34,34,    0076,  // . >
    488,109, 50,34,    0012,  // ENTER

     12,144, 34,34,    BK_KEY_LOWER,  // ���
     47,144, 34,34,    BK_KEY_UPPER,  // ����
     82,144, 34,34,    0121,  // � Q
    117,144, 34,34,    0136,  // � ^
    152,144, 34,34,    0123,  // � S
    187,144, 35,34,    0115,  // � M
    223,144, 34,34,    0111,  // � I
    258,144, 34,34,    0124,  // � T
    293,144, 34,34,    0130,  // � X
    328,144, 34,34,    0102,  // � B
    363,144, 35,34,    0100,  // � @
    399,144, 34,34,    0074,  // , <

     12,179, 50,34,    0016,  // RUS
     64,179, 35,34,    BK_KEY_AR2,  // AR2
     99,179,281,34,    0040,  // Space bar
    382,179, 50,34,    0017,  // LAT

    434,144, 34,69,    0010,  // Left
    469,144, 35,34,    0032,  // Up
    469,179, 35,34,    0033,  // Down
    505,144, 34,69,    0031,  // Right
};
const int m_nKeyboardKeysCount = sizeof(m_arrKeyboardKeys) / sizeof(unsigned short) / KEYBOARD_KEYS_ARRAY_WIDTH;

QKeyboardView::QKeyboardView(QWidget *parent) :
        QWidget(parent)
{
    setMinimumSize(580, 228);
}

void QKeyboardView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QImage imageKeyboard(_T(":/images/keyboard.png"));

    // Center image
    int cxBitmap = (int) imageKeyboard.width();
    int cyBitmap = (int) imageKeyboard.height();
    m_nImageLeft = (this->width() - cxBitmap) / 2;
    m_nImageTop = (this->height() - cyBitmap) / 2;

    painter.drawImage(QPoint(m_nImageLeft, m_nImageTop), imageKeyboard);

    if (m_nImageLeft > 0)
    {
        painter.fillRect(0, 0, m_nImageLeft, this->height(), QColor(115,115,115));
        painter.fillRect(m_nImageLeft + cxBitmap, 0, m_nImageLeft + 1, this->height(), QColor(115,115,115));
    }
    if (m_nImageTop > 0)
    {
        painter.fillRect(0, 0, this->width(), m_nImageTop, QColor(115,115,115));
        painter.fillRect(0, m_nImageTop + cyBitmap, this->width(), m_nImageTop + 1, QColor(115,115,115));
    }

    //showKeyboardMapping(painter);
}

void QKeyboardView::showKeyboardMapping(QPainter& painter)
{
    painter.setPen(QPen(QColor::fromRgb(255, 32, 32)));
    for (int i = 0; i < m_nKeyboardKeysCount; i++)
    {
        QRect rcKey;
        rcKey.setLeft(m_nImageLeft + m_arrKeyboardKeys[i * 5]);
        rcKey.setTop(m_nImageTop + m_arrKeyboardKeys[i * 5 + 1]);
        rcKey.setRight(rcKey.left() + m_arrKeyboardKeys[i * 5 + 2]);
        rcKey.setBottom(rcKey.top() + m_arrKeyboardKeys[i * 5 + 3]);

        //painter.drawRect(rcKey);

        unsigned short scan = m_arrKeyboardKeys[i * 5 + 4];
        if (scan != 0)
        {
            rcKey.adjust(0, 0, -2, -2);
            TCHAR buffer[10];
            _sntprintf(buffer, 10, _T("%03o"), scan);
            painter.drawText(rcKey, Qt::AlignBottom | Qt::AlignRight, buffer);
        }
    }
}

void QKeyboardView::mousePressEvent(QMouseEvent *event)
{
    unsigned char keyscan = GetKeyByPoint(event->x(), event->y(), event->modifiers() & Qt::ShiftModifier);
    if (keyscan == 0) return;

    Emulator_KeyEvent(keyscan, TRUE, event->modifiers() & Qt::ControlModifier);
}

void QKeyboardView::mouseReleaseEvent(QMouseEvent *event)
{
    unsigned char keyscan = GetKeyByPoint(event->x(), event->y(), event->modifiers() & Qt::ShiftModifier);
    if (keyscan == 0) return;

    Emulator_KeyEvent(keyscan, FALSE, event->modifiers() & Qt::ControlModifier);
}

unsigned char QKeyboardView::GetKeyByPoint(int x, int y, BOOL okShift)
{
    for (int i = 0; i < m_nKeyboardKeysCount; i++)
    {
        QRect rcKey;
        rcKey.setLeft(m_nImageLeft + m_arrKeyboardKeys[i * 5]);
        rcKey.setTop(m_nImageTop + m_arrKeyboardKeys[i * 5 + 1]);
        rcKey.setRight(rcKey.left() + m_arrKeyboardKeys[i * 5 + 2]);
        rcKey.setBottom(rcKey.top() + m_arrKeyboardKeys[i * 5 + 3]);

        if (x >= rcKey.left() && x < rcKey.right() && y >= rcKey.top() && y < rcKey.bottom())
        {
            unsigned char bkscan = (unsigned char) m_arrKeyboardKeys[i * 5 + 4];
            if (bkscan == 0) return 0;

            if (okShift && bkscan >= 0100 && bkscan <= 0137)
                bkscan += 040;
            else if (okShift && bkscan >= 0060 && bkscan <= 0077)
                bkscan -= 020;

            return bkscan;
        }
    }

    return 0;
}
