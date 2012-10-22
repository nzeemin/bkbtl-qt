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
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

private:
    unsigned short m_wDisasmBaseAddr;
    unsigned short m_wDisasmNextBaseAddr;

    int DrawDisassemble(QPainter& painter, CProcessor* pProc, unsigned short base, unsigned short previous);
};

#endif // QDISASMVIEW_H
