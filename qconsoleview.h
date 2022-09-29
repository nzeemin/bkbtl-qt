#ifndef QCONSOLEVIEW_H
#define QCONSOLEVIEW_H

#include <QWidget>
#include "Common.h"

class QTextEdit;
class QLabel;
class QLineEdit;
class CProcessor;


struct ConsoleCommandParams
{
    QString     commandText;
    int         paramReg1;
    quint16     paramOct1, paramOct2;
};

class QConsoleView : public QWidget
{
    Q_OBJECT
public:
    QConsoleView();
    ~QConsoleView();

    CProcessor* getCurrentProcessor();

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
    static QString MESSAGE_INVALID_REGNUM;

private:
    QTextEdit *m_log;
    QLabel *m_prompt;
    QLineEdit *m_edit;

    void printConsolePrompt();
    int printDisassemble(quint16 address, bool okOneInstr, bool okShort);
    void printRegister(const char * strName, quint16 value);
    void printMemoryDump(quint16 address, int lines = 8);

public:
    void cmdShowHelp(const ConsoleCommandParams& params);
    void cmdClearConsoleLog(const ConsoleCommandParams& params);
    void cmdSetRegisterValue(const ConsoleCommandParams& params);
    void cmdPrintRegister(const ConsoleCommandParams & params);
    void cmdPrintAllRegisters(const ConsoleCommandParams & params);
    void cmdSetRegisterPSW(const ConsoleCommandParams& params);
    void cmdPrintRegisterPSW(const ConsoleCommandParams& params);
    void cmdStepInto(const ConsoleCommandParams& params);
    void cmdStepOver(const ConsoleCommandParams& params);
    void cmdPrintDisassembleAtAddress(const ConsoleCommandParams& params);
    void cmdPrintDisassembleAtPC(const ConsoleCommandParams& params);
    //void cmdSetMemoryAtAddress(const ConsoleCommandParams& params);
    void cmdPrintMemoryDumpAtAddress(const ConsoleCommandParams& params);
    void cmdPrintMemoryDumpAtRegister(const ConsoleCommandParams& params);
    void cmdPrintMemoryDumpAtPC(const ConsoleCommandParams& params);
    void cmdRunToAddress(const ConsoleCommandParams& params);
    void cmdRun(const ConsoleCommandParams& params);
    void cmdSetBreakpointAtAddress(const ConsoleCommandParams& params);
    void cmdPrintAllBreakpoints(const ConsoleCommandParams& params);
    void cmdRemoveBreakpointAtAddress(const ConsoleCommandParams& params);
    void cmdRemoveAllBreakpoints(const ConsoleCommandParams& params);
};

#endif // QCONSOLEVIEW_H
