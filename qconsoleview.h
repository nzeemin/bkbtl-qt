#ifndef QCONSOLEVIEW_H
#define QCONSOLEVIEW_H

#include <QWidget>

class QTextEdit;
class QLineEdit;

class QConsoleView : public QWidget
{
    Q_OBJECT
public:
    QConsoleView();

private:
    QTextEdit *m_log;
    QLineEdit *m_edit;
};

#endif // QCONSOLEVIEW_H
