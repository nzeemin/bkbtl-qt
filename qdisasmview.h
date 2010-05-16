#ifndef QDISASMVIEW_H
#define QDISASMVIEW_H

#include <QWidget>

class QPainter;
class CProcessor;

class QDisasmView : public QWidget
{
    Q_OBJECT
public:
    QDisasmView();

    void updateData();

protected:
    void paintEvent(QPaintEvent *event);

private:
    unsigned short m_wDisasmBaseAddr;
    unsigned short m_wDisasmNextBaseAddr;

    void DrawDisassemble(QPainter& painter, CProcessor* pProc, unsigned short base, unsigned short previous);
};

#endif // QDISASMVIEW_H
