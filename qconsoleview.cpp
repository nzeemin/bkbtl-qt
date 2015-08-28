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
    setMinimumSize(320, 120);

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
    m_log->moveCursor(QTextCursor::End);
    m_log->insertPlainText(message);
    m_log->moveCursor(QTextCursor::End);
}
void QConsoleView::printLine(const QString &message)
{
    m_log->moveCursor(QTextCursor::End);
    m_log->insertPlainText(message);
    m_log->insertPlainText(_T("\r\n"));
    m_log->moveCursor(QTextCursor::End);
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
            _T("  dXXXXXX    Disassemble from address XXXXXX\r\n")
            _T("  g          Go; free run\r\n")
            _T("  gXXXXXX    Go; run processor until breakpoint at address XXXXXX\r\n")
            _T("  m          Memory dump at current address\r\n")
            _T("  mXXXXXX    Memory dump at address XXXXXX\r\n")
            _T("  mrN        Memory dump at address from register N; N=0..7\r\n")
            _T("  r          Show register values\r\n")
            _T("  rN         Show value of register N; N=0..7,ps\r\n")
            _T("  rN XXXXXX  Set register N to value XXXXXX; N=0..7,ps\r\n")
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

void QConsoleView::printRegister(LPCTSTR strName, WORD value)
{
    TCHAR buffer[31];
    TCHAR* p = buffer;
    *p++ = _T(' ');
    *p++ = _T(' ');
    _tcscpy(p, strName);  p += 2;
    *p++ = _T(' ');
    PrintOctalValue(p, value);  p += 6;
    *p++ = _T(' ');
    PrintBinaryValue(p, value);  p += 16;
    *p++ = _T('\r');
    *p++ = _T('\n');
    *p++ = 0;
    this->print(buffer);
}

void QConsoleView::printMemoryDump(WORD address, int lines)
{
    CProcessor* pProc = g_pBoard->GetCPU();

    address &= ~1;  // Line up to even address

    BOOL okHaltMode = pProc->IsHaltMode();

    for (int line = 0; line < lines; line++)
    {
        WORD dump[8];
        for (int i = 0; i < 8; i++)
            dump[i] = g_pBoard->GetWord(address + i*2, okHaltMode);

        TCHAR buffer[2+6+2 + 7*8 + 1 + 16 + 1 + 2];
        TCHAR* pBuf = buffer;
        *pBuf = _T(' ');  pBuf++;
        *pBuf = _T(' ');  pBuf++;
        PrintOctalValue(pBuf, address);  pBuf += 6;
        *pBuf = _T(' ');  pBuf++;
        *pBuf = _T(' ');  pBuf++;
        for (int i = 0; i < 8; i++) {
            PrintOctalValue(pBuf, dump[i]);  pBuf += 6;
            *pBuf = _T(' ');  pBuf++;
        }
        *pBuf = _T(' ');  pBuf++;
//        for (int i = 0; i < 8; i++) {
//            WORD word = dump[i];
//            BYTE ch1 = LOBYTE(word);
//            TCHAR wch1 = Translate_BK_Unicode(ch1);
//            if (ch1 < 32) wch1 = _T('·');
//            *pBuf = wch1;  pBuf++;
//            BYTE ch2 = HIBYTE(word);
//            TCHAR wch2 = Translate_BK_Unicode(ch2);
//            if (ch2 < 32) wch2 = _T('·');
//            *pBuf = wch2;  pBuf++;
//        }
        *pBuf++ = _T('\r');
        *pBuf++ = _T('\n');
        *pBuf = 0;

        this->print(buffer);

        address += 16;
    }
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
    if (g_okEmulatorRunning) return;

    // Echo command to the log
    this->printLine(command);

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
    else if (command.startsWith(_T("r")))  // Register operations
    {
        if (command.length() == 1)  // Print all registers
        {
            for (int r = 0; r < 8; r++)
            {
                LPCTSTR name = REGISTER_NAME[r];
                WORD value = pProc->GetReg(r);
                this->printRegister(name, value);
            }
        }
        else if (command[1].toLatin1() >= '0' && command[1].toLatin1() <= '7')  // "r0".."r7"
        {
            int r = command[1].toLatin1() - '0';
            LPCTSTR name = REGISTER_NAME[r];
            if (command.length() == 2)  // "rN" - show register N
            {
                WORD value = pProc->GetReg(r);
                this->printRegister(name, value);
            }
            else if (command[2].toLatin1() == '=' || command[2].toLatin1() == ' ')  // "rN=XXXXXX" - set register N to value XXXXXX
            {
                WORD value;
                if (! ParseOctalValue(command.mid(3), &value))
                    this->print(MESSAGE_WRONG_VALUE);
                else
                {
                    pProc->SetReg(r, value);
                    this->printRegister(name, value);
                    okUpdateAllViews = TRUE;
                }
            }
            else
                this->print(MESSAGE_UNKNOWN_COMMAND);
        }
        else if (command.length() >= 2 && command[1].toLatin1() == 'p' && command[2].toLatin1() == 's')  // "rps"
        {
            if (command.length() == 2)  // "rps" - show PSW
            {
                WORD value = pProc->GetPSW();
                this->printRegister(_T("PS"), value);
            }
            else if (command[3].toLatin1() == '=' || command[3].toLatin1() == ' ')  // "rps=XXXXXX" - set PSW to value XXXXXX
            {
                WORD value;
                if (! ParseOctalValue(command.mid(4), &value))
                    this->print(MESSAGE_WRONG_VALUE);
                else
                {
                    pProc->SetPSW(value);
                    this->printRegister(_T("PS"), value);
                    okUpdateAllViews = TRUE;
                }
            }
            else
                this->print(MESSAGE_UNKNOWN_COMMAND);
        }
        else
            this->print(MESSAGE_UNKNOWN_COMMAND);
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
    else if (command.startsWith(_T("d")) ||  // Disassemble
             command.startsWith(_T("D")))    // Disassemble, short format
    {
        BOOL okShort = (command[0] == _T('D'));
        if (command.length() == 1)  // "d" - disassemble at current address
            this->printDisassemble(pProc->GetPC(), FALSE, okShort);
        else if (command[1].toLatin1() >= '0' && command[1].toLatin1() <= '7')  // "dXXXXXX" - disassemble at address XXXXXX
        {
            WORD value;
            if (! ParseOctalValue(command.mid(1), &value))
                this->print(MESSAGE_WRONG_VALUE);
            else
                this->printDisassemble(value, FALSE, okShort);
        }
        else
            this->print(MESSAGE_UNKNOWN_COMMAND);
    }
    else if (command.startsWith(_T("m")))
    {
        if (command.length() == 1)  // "m" - dump memory at current address
        {
            this->printMemoryDump(pProc->GetPC(), 8);
        }
        else if (command[1].toLatin1() >= '0' && command[1].toLatin1() <= '7')  // "mXXXXXX" - dump memory at address XXXXXX
        {
            WORD value;
            if (! ParseOctalValue(command.mid(1), &value))
                this->print(MESSAGE_WRONG_VALUE);
            else
                this->printMemoryDump(value, 8);
        }
        else if (command[1].toLatin1() == 'r' && command.length() >= 3 &&
                command[2].toLatin1() >= '0' && command[2].toLatin1() <= '7')  // "mrN" - dump memory at address from register N
        {
            int r = command[2].toLatin1() - '0';
            WORD address = pProc->GetReg(r);
            this->printMemoryDump(address, 8);
        }
        else
            this->print(MESSAGE_UNKNOWN_COMMAND);
        //TODO: "mXXXXXX YYYYYY" - set memory cell at XXXXXX to value YYYYYY
        //TODO: "mrN YYYYYY" - set memory cell at address from rN to value YYYYYY
    }
    else if (command == _T("g"))  // Go
    {
        Emulator_Start();
        okUpdateAllViews = TRUE;
    }
    else if (command.startsWith(_T("g")))  // Go
    {
        WORD value;
        if (! ParseOctalValue(command.mid(1), &value))
            this->print(MESSAGE_WRONG_VALUE);
        else
        {
            Emulator_SetCPUBreakpoint(value);
            Emulator_Start();

            okUpdateMenu = TRUE;
        }
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
