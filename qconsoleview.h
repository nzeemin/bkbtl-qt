#ifndef QCONSOLEVIEW_H
#define QCONSOLEVIEW_H

#include <QWidget>
#include "Common.h"

class QTextEdit;
class QLineEdit;

class QConsoleView : public QWidget
{
    Q_OBJECT
public:
    QConsoleView();
    ~QConsoleView();

    void print(const QString &message);
    void printLine(const QString &message);
    void clear();

public slots:
    void execConsoleCommand();
    void execConsoleCommand(const QString &command);

private:
    QTextEdit *m_log;
    QLineEdit *m_edit;

    void printConsolePrompt();
    void printHelp();
    int printDisassemble(quint16 address, bool okOneInstr, bool okShort);
    void printRegister(const char * strName, quint16 value);
    void printMemoryDump(quint16 address, int lines);
};

#endif // QCONSOLEVIEW_H
