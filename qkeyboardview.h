#ifndef QKEYBOARDVIEW_H
#define QKEYBOARDVIEW_H

#include <QWidget>
#include "Common.h"

class QKeyboardView : public QWidget
{
    Q_OBJECT
public:
    QKeyboardView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    int m_nImageTop;
    int m_nImageLeft;
    unsigned char m_nKeyPressedScan;
    unsigned char m_nKeyPressedCode;

private:
    unsigned char getKeyByPoint(int x, int y);
    void showKeyboardMapping(QPainter& painter);
};

#endif // QKEYBOARDVIEW_H
