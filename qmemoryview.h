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

public slots:
    void changeWordByteMode();
    void gotoAddress();
    void scrollBy(uint16_t delta);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void wheelEvent(QWheelEvent *);

    void updateScrollPos();

protected slots:
    void scrollValueChanged();

private:
    bool m_ByteMode;  // false - word mode, true - byte mode
    unsigned short m_wBaseAddress;
    int m_cyLineMemory;  // Line height in pixels
    int m_nPageSize;  // Page size in lines
    int m_cyLine;
    QScrollBar *m_scrollbar;
};


#endif // QMEMORYVIEW_H
