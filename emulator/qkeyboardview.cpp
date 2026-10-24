#include "stdafx.h"
#include <QtGui>
#include <QPalette>
#include <cmath>
#include "qkeyboardview.h"
#include "Emulator.h"


//////////////////////////////////////////////////////////////////////


const int KEYBOARD_KEYS_ARRAY_WIDTH = 5;
// Keyboard key mapping to bitmap
const unsigned short m_arrKeyboardKeys[] =
{
    /*   x1,y1    w,h      code  AR2code  */
    3,  3, 51, 34,    BK_KEY_REPEAT, // ПОВТ
    56,  3, 51, 34,    0003, // КТ
    109,  3, 51, 34,    0231, // Arrow right    =|=>|
    162,  3, 51, 34,    0026, // Arrow left     |<===
    215,  3, 51, 34,    0027, // Arrow right    |===>
    268,  3, 51, 34,    0202, // ИНД СУ
    321,  3, 51, 34,    0204, // БЛОК РЕД
    374,  3, 50, 34,    0220, // STEP
    426,  3, 51, 34,    0014, // СБР
    478,  3, 70, 34,    BK_KEY_STOP, // STOP

    3, 38,  34, 34,    BK_KEY_BACKSHIFT, // Big Arrow Down
    38, 38,  34, 34,    0073, // ; +
    74, 38,  34, 34,    0061, // 1 !
    109, 38,  34, 34,    0062, // 2 "
    144, 38,  34, 34,    0063, // 3 #
    179, 38,  34, 34,    0064, // 4 $
    214, 38,  35, 34,    0065, // 5 %
    250, 38,  34, 34,    0066, // 6 &
    285, 38,  34, 34,    0067, // 7 '
    320, 38,  34, 34,    0070, // 8 (
    355, 38,  34, 34,    0071, // 9 )
    391, 38,  34, 34,    0060, // 0 {
    426, 38,  34, 34,    0055, // - =
    461, 38,  34, 34,    0057, // / ?
    498, 38,  50, 34,    0030, // Backspace

    4, 73,  50, 34,    0015, // TAB
    56, 73,  34, 34,    0112, // Й J
    91, 73,  34, 34,    0103, // Ц C
    126, 73,  35, 34,    0125, // У U
    162, 73,  34, 34,    0113, // К K
    197, 73,  34, 34,    0105, // Е E
    232, 73,  34, 34,    0116, // Н N
    267, 73,  34, 34,    0107, // Г G
    302, 73,  35, 34,    0133, // Ш [
    338, 73,  34, 34,    0135, // Щ ]
    373, 73,  34, 34,    0132, // З Z
    408, 73,  34, 34,    0110, // Х H
    443, 73,  35, 34,    0072, // : *
    479, 73,  34, 34,    0137, // Ъ }
    514, 73,  34, 34,    0023, // ВС

    12, 109, 50, 34,    0000, // СУ
    64, 109, 34, 34,    0106, // Ф F
    99, 109, 35, 34,    0131, // Ы Y
    135, 109, 34, 34,    0127, // В W
    170, 109, 34, 34,    0101, // А A
    205, 109, 34, 34,    0120, // П P
    240, 109, 34, 34,    0122, // Р R
    275, 109, 35, 34,    0117, // О O
    311, 109, 34, 34,    0114, // Л L
    346, 109, 34, 34,    0104, // Д D
    381, 109, 34, 34,    0126, // Ж V
    416, 109, 34, 34,    0134, // Э Backslash
    452, 109, 34, 34,    0076, // . >
    488, 109, 50, 34,    0012, // ENTER

    12, 144, 34, 34,    BK_KEY_LOWER, // СТР
    47, 144, 34, 34,    BK_KEY_UPPER, // ЗАГЛ
    82, 144, 34, 34,    0121, // Я Q
    117, 144, 34, 34,    0136, // Ч ^
    152, 144, 34, 34,    0123, // С S
    187, 144, 35, 34,    0115, // М M
    223, 144, 34, 34,    0111, // И I
    258, 144, 34, 34,    0124, // Т T
    293, 144, 34, 34,    0130, // Ь X
    328, 144, 34, 34,    0102, // Б B
    363, 144, 35, 34,    0100, // Ю @
    399, 144, 34, 34,    0074, // , <

    12, 179, 50, 34,    0016, // RUS
    64, 179, 35, 34,    BK_KEY_AR2, // AR2
    99, 179, 281, 34,    0040, // Space bar
    382, 179, 50, 34,    0017, // LAT

    434, 144, 34, 69,    0010, // Left
    469, 144, 35, 34,    0032, // Up
    469, 179, 35, 34,    0033, // Down
    505, 144, 34, 69,    0031, // Right
};
const int m_nKeyboardKeysCount = sizeof(m_arrKeyboardKeys) / sizeof(unsigned short) / KEYBOARD_KEYS_ARRAY_WIDTH;

// Keyboard is drawn in these logical units and scaled to the widget size
const int KEYBOARD_WIDTH = 551;
const int KEYBOARD_HEIGHT = 216;

enum KeyStyle { KeyDark, KeyLight, KeyRed };

// Legend for every key above, in the same order. Text starting with '@' is a glyph name
struct KeyLegend
{
    const char* top;
    const char* bottom;
    KeyStyle style;
};
const KeyLegend m_arrKeyLegends[] =
{
    { "ПОВТ", "", KeyDark }, { "КТ", "", KeyDark }, { "@indent", "", KeyDark }, { "@home", "", KeyDark },
    { "@end", "", KeyDark }, { "ИНД", "СУ", KeyDark }, { "БЛОК", "РЕД", KeyDark }, { "ШАГ", "", KeyDark },
    { "СБР", "", KeyDark }, { "СТОП", "", KeyRed },

    { "@shiftdown", "", KeyLight }, { ";", "+", KeyLight }, { "1", "!", KeyLight }, { "2", "\"", KeyLight },
    { "3", "#", KeyLight }, { "4", "¤", KeyLight }, { "5", "%", KeyLight }, { "6", "&", KeyLight },
    { "7", "'", KeyLight }, { "8", "(", KeyLight }, { "9", ")", KeyLight }, { "0", "{", KeyLight },
    { "-", "=", KeyLight }, { "/", "?", KeyLight }, { "@backspace", "", KeyLight },

    { "ТАБ", "", KeyDark }, { "Й", "J", KeyLight }, { "Ц", "C", KeyLight }, { "У", "U", KeyLight },
    { "К", "K", KeyLight }, { "Е", "E", KeyLight }, { "Н", "N", KeyLight }, { "Г", "G", KeyLight },
    { "Ш", "[", KeyLight }, { "Щ", "]", KeyLight }, { "З", "Z", KeyLight }, { "Х", "H", KeyLight },
    { ":", "*", KeyLight }, { "Ъ", "}", KeyLight }, { "ВС", "", KeyDark },

    { "СУ", "", KeyDark }, { "Ф", "F", KeyLight }, { "Ы", "Y", KeyLight }, { "В", "W", KeyLight },
    { "А", "A", KeyLight }, { "П", "P", KeyLight }, { "Р", "R", KeyLight }, { "О", "O", KeyLight },
    { "Л", "L", KeyLight }, { "Д", "D", KeyLight }, { "Ж", "V", KeyLight }, { "Э", "\\", KeyLight },
    { ".", ">", KeyLight }, { "@enter", "", KeyLight },

    { "СТР", "", KeyDark }, { "ЗАГЛ", "", KeyDark }, { "Я", "Q", KeyLight }, { "Ч", "¬", KeyLight },
    { "С", "S", KeyLight }, { "М", "M", KeyLight }, { "И", "I", KeyLight }, { "Т", "T", KeyLight },
    { "Ь", "X", KeyLight }, { "Б", "B", KeyLight }, { "Ю", "@", KeyLight }, { ",", "<", KeyLight },

    { "РУС", "", KeyDark }, { "АР2", "", KeyDark }, { "", "", KeyLight }, { "ЛАТ", "", KeyDark },
    { "@left", "", KeyDark }, { "@up", "", KeyDark }, { "@down", "", KeyDark }, { "@right", "", KeyDark },
};
static_assert(sizeof(m_arrKeyLegends) / sizeof(KeyLegend) == sizeof(m_arrKeyboardKeys) / sizeof(unsigned short) / KEYBOARD_KEYS_ARRAY_WIDTH,
              "Every key needs a legend");

static void drawGlyph(QPainter& painter, const QString& name, const QRectF& rc)
{
    const QPointF c = rc.center();
    const qreal hw = rc.width() * 0.28, hh = rc.height() * 0.22;  // glyph half-extents
    const qreal head = qMin(hw, hh) * 0.9;

    // Arrow from 'from' to 'to' with the head at 'to'
    auto arrow = [&](QPointF from, QPointF to)
    {
        QPointF d = to - from;
        qreal len = std::hypot(d.x(), d.y());
        QPointF u = d / len, n(-u.y(), u.x());
        painter.drawLine(from, to);
        painter.drawLine(to, to - u * head + n * head * 0.7);
        painter.drawLine(to, to - u * head - n * head * 0.7);
    };

    painter.save();
    QPen pen = painter.pen();
    pen.setWidthF(1.6);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    if (name == "@right" || name == "@end") arrow(c - QPointF(hw, 0), c + QPointF(hw, 0));
    if (name == "@left" || name == "@home" || name == "@backspace") arrow(c + QPointF(hw, 0), c - QPointF(hw, 0));
    if (name == "@up") arrow(c + QPointF(0, hh), c - QPointF(0, hh));
    if (name == "@down") arrow(c - QPointF(0, hh), c + QPointF(0, hh));
    if (name == "@end") painter.drawLine(c + QPointF(hw, -hh), c + QPointF(hw, hh));
    if (name == "@home") painter.drawLine(c + QPointF(-hw, -hh), c + QPointF(-hw, hh));
    if (name == "@indent")
    {
        QPolygonF poly;
        poly << c + QPointF(-hw, -hh * 0.5) << c + QPointF(0, -hh * 0.5) << c + QPointF(0, -hh)
             << c + QPointF(hw, 0) << c + QPointF(0, hh) << c + QPointF(0, hh * 0.5) << c + QPointF(-hw, hh * 0.5);
        painter.drawPolygon(poly);
    }
    if (name == "@shiftdown")
    {
        QPolygonF poly;
        poly << c + QPointF(-hh * 0.5, -hh) << c + QPointF(hh * 0.5, -hh) << c + QPointF(hh * 0.5, 0)
             << c + QPointF(hh, 0) << c + QPointF(0, hh) << c + QPointF(-hh, 0) << c + QPointF(-hh * 0.5, 0);
        painter.drawPolygon(poly);
    }
    if (name == "@enter")
    {
        painter.drawLine(c + QPointF(hw, -hh), c + QPointF(hw, hh * 0.4));
        arrow(c + QPointF(hw, hh * 0.4), c + QPointF(-hw, hh * 0.4));
    }

    painter.restore();
}

QKeyboardView::QKeyboardView(QWidget *parent) :
    QWidget(parent)
{
    m_nKeyPressedScan = m_nKeyPressedCode = 0;

    m_nImageTop = m_nImageLeft = 0;
    m_scale = 1.0;

    setMinimumSize(KEYBOARD_WIDTH + 8, KEYBOARD_HEIGHT + 8);
}

QSize QKeyboardView::imageSize() const
{
    return QSize(KEYBOARD_WIDTH, KEYBOARD_HEIGHT);
}

// Scale the keyboard to the largest size that keeps its aspect ratio, and center it
void QKeyboardView::updateTransform()
{
    QSize scaled = imageSize().scaled(this->size(), Qt::KeepAspectRatio);
    m_scale = double(scaled.width()) / KEYBOARD_WIDTH;
    m_nImageLeft = (this->width() - scaled.width()) / 2;
    m_nImageTop = (this->height() - scaled.height()) / 2;
}

void QKeyboardView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(115, 115, 115));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    updateTransform();
    painter.translate(m_nImageLeft, m_nImageTop);
    painter.scale(m_scale, m_scale);
    painter.fillRect(0, 0, KEYBOARD_WIDTH, KEYBOARD_HEIGHT, QColor(200, 200, 200));

    QFont font = painter.font();
    font.setPixelSize(11);
    painter.setFont(font);

    for (int i = 0; i < m_nKeyboardKeysCount; i++)
    {
        const KeyLegend& legend = m_arrKeyLegends[i];
        QRectF rcKey(m_arrKeyboardKeys[i * 5], m_arrKeyboardKeys[i * 5 + 1],
                     m_arrKeyboardKeys[i * 5 + 2], m_arrKeyboardKeys[i * 5 + 3]);
        rcKey.adjust(0.5, 0.5, -0.5, -0.5);
        bool pressed = m_nKeyPressedScan != 0 && m_nKeyPressedScan == m_arrKeyboardKeys[i * 5 + 4];

        QColor colorTop, colorBottom, colorText;
        switch (legend.style)
        {
        case KeyLight: colorTop = QColor(255, 255, 255); colorBottom = QColor(225, 225, 225); colorText = QColor(30, 30, 30); break;
        case KeyRed:   colorTop = QColor(235, 70, 55);   colorBottom = QColor(190, 25, 15);   colorText = QColor(255, 255, 255); break;
        default:       colorTop = QColor(110, 110, 110); colorBottom = QColor(60, 60, 60);    colorText = QColor(240, 240, 240); break;
        }
        if (pressed)  // Pressed key is shown inverted
        {
            colorTop = QColor(255 - colorTop.red(), 255 - colorTop.green(), 255 - colorTop.blue());
            colorBottom = QColor(255 - colorBottom.red(), 255 - colorBottom.green(), 255 - colorBottom.blue());
            colorText = QColor(255 - colorText.red(), 255 - colorText.green(), 255 - colorText.blue());
        }

        QLinearGradient gradient(rcKey.topLeft(), rcKey.bottomLeft());
        gradient.setColorAt(0, colorTop);
        gradient.setColorAt(1, colorBottom);
        painter.setPen(QPen(QColor(40, 40, 40), 0.8));
        painter.setBrush(gradient);
        painter.drawRoundedRect(rcKey, 4, 4);

        painter.setPen(colorText);
        QString top = QString::fromUtf8(legend.top);
        QString bottom = QString::fromUtf8(legend.bottom);
        if (top.startsWith('@'))
            drawGlyph(painter, top, rcKey);
        else if (bottom.isEmpty())
            painter.drawText(rcKey, Qt::AlignCenter, top);
        else
        {
            QRectF half(rcKey.left(), rcKey.top(), rcKey.width(), rcKey.height() / 2);
            painter.drawText(half.translated(0, 2), Qt::AlignCenter, top);
            painter.drawText(half.translated(0, rcKey.height() / 2 - 2), Qt::AlignCenter, bottom);
        }
    }

    //showKeyboardMapping(painter);
}

void QKeyboardView::showKeyboardMapping(QPainter& painter)
{
    painter.setPen(QPen(QColor::fromRgb(255, 32, 32)));
    for (int i = 0; i < m_nKeyboardKeysCount; i++)
    {
        QRect rcKey;
        rcKey.setLeft(m_arrKeyboardKeys[i * 5]);
        rcKey.setTop(m_arrKeyboardKeys[i * 5 + 1]);
        rcKey.setRight(rcKey.left() + m_arrKeyboardKeys[i * 5 + 2]);
        rcKey.setBottom(rcKey.top() + m_arrKeyboardKeys[i * 5 + 3]);

        //painter.drawRect(rcKey);

        unsigned short scan = m_arrKeyboardKeys[i * 5 + 4];
        if (scan != 0)
        {
            rcKey.adjust(0, 0, -2, -2);
            char buffer[10];
            _snprintf(buffer, 10, "%03o", scan);
            painter.drawText(rcKey, Qt::AlignBottom | Qt::AlignRight, buffer);
        }
    }
}

void QKeyboardView::mousePressEvent(QMouseEvent *event)
{
    unsigned char keyscan = getKeyByPoint(event->x(), event->y());
    if (keyscan == 0) return;

    grabMouse();

    m_nKeyPressedScan = keyscan;

    bool okShift = (event->modifiers() & Qt::ShiftModifier) != 0;
    if (okShift && keyscan >= 0100 && keyscan <= 0137)
        keyscan += 040;
    else if (okShift && keyscan >= 0060 && keyscan <= 0077)
        keyscan -= 020;

    m_nKeyPressedCode = keyscan;

    Emulator_KeyEvent(keyscan, true, event->modifiers() & Qt::ControlModifier);

    repaint();
}

void QKeyboardView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_nKeyPressedScan == 0) return;

    releaseMouse();

    Emulator_KeyEvent(m_nKeyPressedCode, false, event->modifiers() & Qt::ControlModifier);

    m_nKeyPressedScan = m_nKeyPressedCode = 0;

    repaint();
}

unsigned char QKeyboardView::getKeyByPoint(int x, int y)
{
    // Widget point to logical keyboard coordinates
    updateTransform();
    QPointF pt((x - m_nImageLeft) / m_scale, (y - m_nImageTop) / m_scale);

    for (int i = 0; i < m_nKeyboardKeysCount; i++)
    {
        QRectF rcKey(m_arrKeyboardKeys[i * 5], m_arrKeyboardKeys[i * 5 + 1],
                     m_arrKeyboardKeys[i * 5 + 2], m_arrKeyboardKeys[i * 5 + 3]);

        if (rcKey.contains(pt))
        {
            unsigned char bkscan = (unsigned char) m_arrKeyboardKeys[i * 5 + 4];
            if (bkscan == 0) return 0;

            return bkscan;
        }
    }

    return 0;
}
