#ifndef QKEYBOARDVIEW_H
#define QKEYBOARDVIEW_H

#include <QWidget>

class QKeyboardView : public QWidget
{
    Q_OBJECT
public:
    QKeyboardView(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    int m_nImageTop;
    int m_nImageLeft;

private:
    unsigned char GetKeyByPoint(int x, int y);
};

#endif // QKEYBOARDVIEW_H
