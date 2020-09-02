#ifndef QTAPEVIEW_H
#define QTAPEVIEW_H

#include <QWidget>

class QLabel;
class QPushButton;
typedef void *HANDLE;

class QTapeView : public QWidget
{
    Q_OBJECT
public:
    QTapeView(QWidget *parent = nullptr);

    bool tapeReadCallback(unsigned int samples);
    void tapeWriteCallback(int value, unsigned int samples);

public slots:
    void doPlayStop();
    void doRewind();
    void doTapeOpen();
    void doTapeSave();

private:
    QLabel* m_labelFile;
    QLabel* m_labelTotal;
    QLabel* m_labelCurrent;
    QPushButton* m_buttonPlay;
    QPushButton* m_buttonRewind;
    QPushButton* m_buttonOpen;
    QPushButton* m_buttonSave;

    bool m_okTapeInserted;
    bool m_okTapePlaying;
    bool m_okTapeRecording;
    QString m_sTapeFile;
    HANDLE m_hTapeWavPcmFile;
    unsigned long m_dwTapePositionShown;

    void createTape(const QString &sFileName);
    void openTape(const QString &sFileName);
    void closeTape();

    void stopTape();
    void playTape();
    void updatePosition();
};

#endif // QTAPEVIEW_H
