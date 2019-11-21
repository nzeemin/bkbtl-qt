#ifndef QDEBUGVIEW_H
#define QDEBUGVIEW_H

#include <QWidget>
#include "Common.h"

class CMotherboard;
class CProcessor;
class QToolBar;

class QDebugView : public QWidget
{
    Q_OBJECT
public:
    QDebugView(QWidget *parent = nullptr);

    void updateData();

protected:
    void paintEvent(QPaintEvent *event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

private:
    QToolBar* m_toolbar;
    unsigned short m_wDebugCpuR[9];  // Old register values - R0..R7, PSW
    bool m_okDebugCpuRChanged[9];  // Register change flags

private:
    void drawProcessor(QPainter &painter, const CProcessor *pProc, int x, int y, quint16 *arrR, bool *arrRChanged);
    void drawMemoryForRegister(QPainter &painter, int reg, CProcessor *pProc, int x, int y);
    void drawPorts(QPainter &painter, int x, int y);
};

#endif // QDEBUGVIEW_H
