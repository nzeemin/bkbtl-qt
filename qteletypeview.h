#ifndef QTELETYPEVIEW_H
#define QTELETYPEVIEW_H

#include <QWidget>

class QTextEdit;

class QTeletypeView : public QWidget
{
    Q_OBJECT
public:
    QTeletypeView();
    ~QTeletypeView();

    void print(const QString &);

private:
    QTextEdit *m_log;
};

#endif // QTELETYPEVIEW_H
