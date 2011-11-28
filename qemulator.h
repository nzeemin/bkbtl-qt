#ifndef QEMULATOR_H
#define QEMULATOR_H

#include <QObject>

class QEmulator : public QObject
{
    Q_OBJECT
public:
    QEmulator() { }

public:


public slots:
    void reset();
    bool run(int frames);
    //TODO: void setBreakpoint(qword address);
    //TODO: bool isBreakpoint()
};

#endif // QEMULATOR_H
