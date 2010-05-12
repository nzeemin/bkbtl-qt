#ifndef QSCREEN_H
#define QSCREEN_H

#include <QWidget>

class QScreen : public QWidget
{
    Q_OBJECT
public:
    QScreen(QWidget *parent = 0);
    ~QScreen();

public:
    void saveScreenshot(QString strFileName);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QImage* m_image;
    int m_nImageTop;
    int m_nImageLeft;

private:
    unsigned char TranslateQtKeyToBkKey(int qtkey);
};

#endif // QSCREEN_H
