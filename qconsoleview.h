#ifndef QCONSOLEVIEW_H
#define QCONSOLEVIEW_H

#include <QWidget>
#include "Common.h"

class QTextEdit;
class QLabel;
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
    void updatePrompt();

public slots:
    void execConsoleCommand();
    void execConsoleCommand(const QString &command);

protected:
    static QString MESSAGE_UNKNOWN_COMMAND;
    static QString MESSAGE_WRONG_VALUE;

private:
    QTextEdit *m_log;
    QLabel *m_prompt;
    QLineEdit *m_edit;

    void printConsolePrompt();
    void printHelp();
    int printDisassemble(quint16 address, bool okOneInstr, bool okShort);
    void printRegister(const char * strName, quint16 value);
    void printMemoryDump(quint16 address, int lines);
};

#endif // QCONSOLEVIEW_H
