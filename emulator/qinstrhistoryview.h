#ifndef QINSTRHISTORYVIEW_H
#define QINSTRHISTORYVIEW_H

#include <QWidget>
#include <QVector>
#include "Emulator.h"

class QScrollBar;

class QInstrHistoryView : public QWidget
{
    Q_OBJECT
public:
    QInstrHistoryView();

    void updateData();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void scrollBarChanged(int value);

private:
    int rowCount() const;
    int maxIndex() const;
    void updateScrollBar();
    void setLastIndex(int index);

    QScrollBar *m_scrollBar;
    int m_lastIndex;  // History index drawn on the bottom row; 0 = the most recent instruction
    // Snapshot of the history taken on update after Stop or Step;
    // the view shows the snapshot, so it keeps the state of the previous stop while the emulator is running
    QVector<InstructionRecord> m_snapshot;  // Index 0 is the most recent instruction
};

#endif // QINSTRHISTORYVIEW_H
