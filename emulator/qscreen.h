﻿#ifndef QEMULATORSCREEN_H
#define QEMULATORSCREEN_H

#include <QWidget>
#include "main.h"


class QEmulatorScreen : public QWidget
{
    Q_OBJECT
public:
    QEmulatorScreen(QWidget *parent = nullptr);
    ~QEmulatorScreen();

public:
    QImage getScreenshot();
    int mode() const { return m_mode; }
    void setMode(int mode);

protected:
    void createDisplay();
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QImage* m_image;
    int m_nImageTop;
    int m_nImageLeft;
    int m_mode;

private:
    unsigned char TranslateQtKeyToBkKey(int qtkey, bool okShift);
};

#endif // QEMULATORSCREEN_H
