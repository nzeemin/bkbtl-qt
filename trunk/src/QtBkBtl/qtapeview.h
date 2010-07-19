#ifndef QTAPEVIEW_H
#define QTAPEVIEW_H

#include <QWidget>

class QLabel;
class QPushButton;

class QTapeView : public QWidget
{
    Q_OBJECT
public:
    QTapeView(QWidget *parent = 0);

public slots:
    void tapePlay();
    void tapeRewind();
    void tapeOpen();
    void tapeSave();

private:
    QLabel* m_labelFile;
    QLabel* m_labelTotal;
    QLabel* m_labelCurrent;
    QPushButton* m_buttonPlay;
    QPushButton* m_buttonRewind;
    QPushButton* m_buttonOpen;
    QPushButton* m_buttonSave;
};

#endif // QTAPEVIEW_H
