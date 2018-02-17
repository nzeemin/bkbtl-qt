#include "stdafx.h"
#include <QtGui>
#include "qscreen.h"
#include "Emulator.h"

QEmulatorScreen::QEmulatorScreen(QWidget *parent) :
    QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    m_image = 0;
    m_mode = 0;

    createDisplay();
}

QEmulatorScreen::~QEmulatorScreen()
{
    delete m_image;
}

void QEmulatorScreen::saveScreenshot(QString strFileName)
{
    m_image->save(strFileName, "PNG", -1);
}

void QEmulatorScreen::setMode(int mode)
{
    if (m_mode == mode) return;

    m_mode = mode;

    createDisplay();

    this->repaint();
}

void QEmulatorScreen::createDisplay()
{
    if (m_image != 0)
    {
        delete m_image;
        m_image = 0;
    }

    int cxScreenWidth, cyScreenHeight;
    Emulator_GetScreenSize(m_mode, &cxScreenWidth, &cyScreenHeight);

    m_image = new QImage(cxScreenWidth, cyScreenHeight, QImage::Format_RGB32);

    setMinimumSize(cxScreenWidth + 8, cyScreenHeight + 8);
    setMaximumSize(cxScreenWidth + 100, cyScreenHeight + 20);
}

void QEmulatorScreen::paintEvent(QPaintEvent * /*event*/)
{
    Emulator_PrepareScreenRGB32(m_image->bits(), m_mode);

    // Center image
    int cxBitmap = m_image->width();
    int cyBitmap = m_image->height();
    m_nImageLeft = (this->width() - cxBitmap) / 2;
    m_nImageTop = (this->height() - cyBitmap) / 2;

    QPainter painter(this);
    painter.drawImage(m_nImageLeft, m_nImageTop, *m_image);

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
}

void QEmulatorScreen::keyPressEvent(QKeyEvent *event)
{
    if (! g_okEmulatorRunning) return;
    if (event->isAutoRepeat()) return;

    unsigned char bkscan = TranslateQtKeyToBkKey(event->key(), event->modifiers() & Qt::ShiftModifier);
    if (bkscan == 0) return;

    Emulator_KeyEvent(bkscan, true, event->modifiers() & Qt::ControlModifier);
    event->accept();
}

void QEmulatorScreen::keyReleaseEvent(QKeyEvent *event)
{
    unsigned char bkscan = TranslateQtKeyToBkKey(event->key(), event->modifiers() & Qt::ShiftModifier);
    if (bkscan == 0) return;

    Emulator_KeyEvent(bkscan, false, event->modifiers() & Qt::ControlModifier);
    event->accept();
}

const unsigned char arrPcscan2BkscanLat[256] = {  // ËÀÒ
/*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
/*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*2*/    0040, 0000, 0000, 0016, 0017, 0010, 0032, 0031, 0033, 0000, 0000, 0000, 0000, 0023, 0000, 0000,
/*3*/    0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067, 0070, 0071, 0000, 0000, 0000, 0000, 0000, 0000,
/*4*/    0000, 0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
/*5*/    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127, 0130, 0131, 0132, 0133, 0000, 0135, 0000, 0000,
/*6*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*7*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*c*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
};
const unsigned char arrPcscan2BkscanRus[256] = {  // ÐÓÑ
/*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
/*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*2*/    0040, 0000, 0000, 0016, 0017, 0010, 0032, 0031, 0033, 0000, 0000, 0000, 0000, 0023, 0000, 0000,
/*3*/    0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067, 0070, 0071, 0000, 0000, 0000, 0000, 0000, 0000,
/*4*/    0000, 0106, 0111, 0123, 0127, 0125, 0101, 0120, 0122, 0133, 0117, 0114, 0104, 0120, 0124, 0135,
/*5*/    0132, 0112, 0113, 0131, 0105, 0107, 0115, 0103, 0136, 0116, 0121, 0110, 0000, 0137, 0000, 0000,
/*6*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*7*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*c*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
};


unsigned char QEmulatorScreen::TranslateQtKeyToBkKey(int qtkey, bool okShift)
{
    switch (qtkey)
    {
    case Qt::Key_Down:      return 0033;
    case Qt::Key_Up:        return 0032;
    case Qt::Key_Left:      return 0010;
    case Qt::Key_Right:     return 0031;
    case Qt::Key_Enter:     return 0012;
    case Qt::Key_Return:    return 0012;
    case Qt::Key_Tab:       return 0015;
    case Qt::Key_Space:     return 0040;
    case Qt::Key_Backspace: return 0030;
    case Qt::Key_Home:      return 0023;    // ÂÑ
    case Qt::Key_F1:        return 0201;    // ÏÎÂÒ
    case Qt::Key_F2:        return 0003;    // ÊÒ
    case Qt::Key_F3:        return 0231;    // =|=>
    case Qt::Key_F4:        return 0026;    // |<==
    case Qt::Key_F5:        return 0027;    // |==>
    case Qt::Key_F6:        return 0202;    // ÈÍÄ ÑÓ
    case Qt::Key_F7:        return 0204;    // ÁËÎÊ ÐÅÄ
    case Qt::Key_F8:        return 0220;    // ØÀÃ
    case Qt::Key_Delete:    return 0014;    // ÑÁÐ
    }

    if (qtkey >= 32 && qtkey <= 255)
    {
        // Âûáèðàåì òàáëèöó ìàïïèíãà â çàâèñèìîñòè îò ôëàãà ÐÓÑ/ËÀÒ â ÁÊ
        unsigned short bkregister = g_pBoard->GetKeyboardRegister();
        const unsigned char * pTable = ((bkregister & KEYB_LAT) == 0) ? arrPcscan2BkscanRus : arrPcscan2BkscanLat;
        unsigned char bkscan = pTable[qtkey];
        if (bkscan == 0) return 0;

        if (okShift && bkscan >= 0100 && bkscan <= 0137)
            bkscan += 040;
        else if (okShift && bkscan >= 0060 && bkscan <= 0077)
            bkscan -= 020;

        return bkscan;
    }

    return 0;
}
