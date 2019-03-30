#ifndef QSOUNDOUT_H
#define QSOUNDOUT_H

#include <QObject>
#include <QAudioOutput>
#include <QIODevice>
#include <QMutex>
#include <QTimer>

#define SAMPLERATE          22050
#define FRAMESAMPLES        (SAMPLERATE/25)
#define SAMPLESIZE          16
#define CHANNELS            2
#define FRAMEBYTES          ((FRAMESAMPLES)*(SAMPLESIZE/8)*CHANNELS)
#define BUFFERS             5

class QSoundOut : public QObject
{
    Q_OBJECT
public:
    explicit QSoundOut(QObject *parent = nullptr);
    ~QSoundOut();

signals:

public slots:
    void FeedDAC(unsigned short left, unsigned short right);

private slots:
    void OnNotify();

private:
    QAudioOutput * m_audio;
    QIODevice * m_dev;
    QMutex  m_lock;
    QTimer m_kick;
    unsigned char rbuf[BUFFERS][FRAMEBYTES];
    unsigned char fbuf[FRAMEBYTES];
    int rcnt;
    int rrd;
    int rwr;
    int fptr;
};

#endif // QSOUNDOUT_H
