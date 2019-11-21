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

private:
    unsigned char GetKeyByPoint(int x, int y, bool okShift);
    void showKeyboardMapping(QPainter& painter);
};

#endif // QKEYBOARDVIEW_H
