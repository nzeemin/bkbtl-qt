#ifndef QMEMORYVIEW_H
#define QMEMORYVIEW_H

#include <QWidget>

class QMemoryView : public QWidget
{
    Q_OBJECT
public:
    QMemoryView();

    void updateData();

protected:
    void paintEvent(QPaintEvent *event);

private:
    unsigned short m_wBaseAddress;
    int m_cyLineMemory;  // Line height in pixels
    int m_nPageSize;  // Page size in lines
};

#endif // QMEMORYVIEW_H
