#ifndef QMEMORYVIEW_H
#define QMEMORYVIEW_H

#include <QWidget>
#include <QScrollBar>

class QMemoryView : public QWidget
{
    Q_OBJECT
public:
    QMemoryView();
    ~QMemoryView();

    void updateData();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

protected slots:
    void scrollValueChanged();

private:
    unsigned short m_wBaseAddress;
    int m_cyLineMemory;  // Line height in pixels
    int m_nPageSize;  // Page size in lines
    int m_cyLine;
    QScrollBar *m_scrollbar;
};


#endif // QMEMORYVIEW_H
