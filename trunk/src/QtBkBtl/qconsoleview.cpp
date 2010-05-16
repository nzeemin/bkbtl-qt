#include "stdafx.h"
#include "qconsoleview.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include "Emulator.h"
#include "emubase/Emubase.h"


const LPCTSTR MESSAGE_UNKNOWN_COMMAND = _T("  Unknown command.\r\n");
const LPCTSTR MESSAGE_WRONG_VALUE = _T("  Wrong value.\r\n");


QConsoleView::QConsoleView()
{
    setMinimumSize(320, 180);

    m_log = new QTextEdit();
    m_edit = new QLineEdit();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_log);
    vboxlayout->addWidget(m_edit);
    this->setLayout(vboxlayout);

    QFont font = Common_GetMonospacedFont();
    m_log->setReadOnly(true);
    m_log->setFont(font);
    m_edit->setFont(font);

    QObject::connect(m_edit, SIGNAL(returnPressed()), this, SLOT(execConsoleCommand()));

    this->print(_T("Use 'h' command to show help.\r\n\r\n"));
}

QConsoleView::~QConsoleView()
{
    delete m_log;
    delete m_edit;
}

void QConsoleView::clear()
{
    m_log->clear();
}

void QConsoleView::print(const QString &message)
{
    // Put selection to the end of text
    m_log->moveCursor(QTextCursor::End);
    // Insert the message
    m_log->insertPlainText(message);
    //TODO: Scroll to caret
}

void QConsoleView::printConsolePrompt()
{
    CProcessor* pProc = g_pBoard->GetCPU();
    TCHAR buffer[14];
    _sntprintf(buffer, 14, _T("%06o> "), pProc->GetPC());
    this->print(buffer);
}

void QConsoleView::printHelp()
{
    this->print(_T("Console command list:\r\n")
            _T("  c          Clear console log\r\n")
//            _T("  dXXXXXX    Disassemble from address XXXXXX\r\n")
//            _T("  g          Go; free run\r\n")
//            _T("  gXXXXXX    Go; run processor until breakpoint at address XXXXXX\r\n")
//            _T("  m          Memory dump at current address\r\n")
//            _T("  mXXXXXX    Memory dump at address XXXXXX\r\n")
//            _T("  mrN        Memory dump at address from register N; N=0..7\r\n")
//            _T("  r          Show register values\r\n")
//            _T("  rN         Show value of register N; N=0..7,ps\r\n")
//            _T("  rN XXXXXX  Set register N to value XXXXXX; N=0..7,ps\r\n")
            _T("  s          Step Into; executes one instruction\r\n")
            _T("  so         Step Over; executes and stops after the current instruction\r\n")
//            _T("  u          Save memory dump to file memdumpXPU.bin\r\n")
        );
}

int QConsoleView::printDisassemble(WORD address, BOOL okOneInstr, BOOL okShort)
{
    CProcessor* pProc = g_pBoard->GetCPU();
    BOOL okHaltMode = pProc->IsHaltMode();

    const int nWindowSize = 30;
    WORD memory[nWindowSize + 2];
    int addrtype;
    for (int i = 0; i < nWindowSize + 2; i++)
        memory[i] = g_pBoard->GetWordView(address + i*2, okHaltMode, TRUE, &addrtype);

    TCHAR bufaddr[7];
    TCHAR bufvalue[7];
    TCHAR buffer[64];

    int lastLength = 0;
    int length = 0;
    for (int index = 0; index < nWindowSize; index++) {  // Рисуем строки
        PrintOctalValue(bufaddr, address);
        WORD value = memory[index];
        PrintOctalValue(bufvalue, value);

        if (length > 0)
        {
            if (!okShort)
            {
                _sntprintf(buffer, 64, _T("  %s  %s\r\n"), bufaddr, bufvalue);
                this->print(buffer);
            }
        }
        else
        {
            if (okOneInstr && index > 0)
                break;
            TCHAR instr[8];
            TCHAR args[32];
            length = DisassembleInstruction(memory + index, address, instr, args);
            lastLength = length;
            if (index + length > nWindowSize)
                break;
            if (okShort)
                _sntprintf(buffer, 64, _T("  %s  %-7s %s\r\n"), bufaddr, instr, args);
            else
                _sntprintf(buffer, 64, _T("  %s  %s  %-7s %s\r\n"), bufaddr, bufvalue, instr, args);
            this->print(buffer);
        }
        length--;
        address += 2;
    }

    return lastLength;
}

void QConsoleView::execConsoleCommand()
{
    QString command = m_edit->text();
    m_edit->clear();
    this->execConsoleCommand(command);
}

void QConsoleView::execConsoleCommand(const QString &command)
{
    if (command.isNull() || command.isEmpty()) return;  // Nothing to do

    // Echo command to the log
    this->print(command);
    this->print(_T("\r\n"));

    BOOL okUpdateAllViews = FALSE;  // Flag - need to update all debug views
    BOOL okUpdateMenu = FALSE;  // Flag - need to update main menu
    CProcessor* pProc = g_pBoard->GetCPU();

    // Execute the command
    if (command == _T("h"))
    {
        this->printHelp();
    }
    else if (command == _T("c"))  // Clear log
    {
        this->clear();
    }
    else if (command == _T("s"))  // "s" - Step Into, execute one instruction
    {
        this->printDisassemble(pProc->GetPC(), TRUE, FALSE);

        //pProc->Execute();
        g_pBoard->DebugTicks();

        okUpdateAllViews = TRUE;
    }
    else if (command == _T("so"))  // "so" - Step Over
    {
        int instrLength = this->printDisassemble(pProc->GetPC(), TRUE, FALSE);
        WORD bpaddress = pProc->GetPC() + instrLength * 2;

        Emulator_SetCPUBreakpoint(bpaddress);
        Emulator_Start();

        okUpdateMenu = TRUE;
    }
    else
    {
        this->print(MESSAGE_UNKNOWN_COMMAND);
    }

    this->printConsolePrompt();

    if (okUpdateAllViews)
        Global_UpdateAllViews();
    else if (okUpdateMenu)
        Global_UpdateMenu();

}
