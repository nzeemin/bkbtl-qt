#ifndef QKEYBOARDVIEW_H
#define QKEYBOARDVIEW_H

#include <QWidget>
#include "Common.h"

class QKeyboardView : public QWidget
{
    Q_OBJECT
public:
    QKeyboardView(QWidget *parent = nullptr);
    QSize imageSize() const;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    int m_nImageTop;
    int m_nImageLeft;
    double m_scale;
    unsigned char m_nKeyPressedScan;
    unsigned char m_nKeyPressedCode;

private:
    void updateTransform();
    unsigned char getKeyByPoint(int x, int y);
    void showKeyboardMapping(QPainter& painter);
};

#endif // QKEYBOARDVIEW_H
