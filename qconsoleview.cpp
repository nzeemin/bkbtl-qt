#include "stdafx.h"
#include "qconsoleview.h"
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "main.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


const char * MESSAGE_UNKNOWN_COMMAND = "  Unknown command.\r\n";
const char * MESSAGE_WRONG_VALUE = "  Wrong value.\r\n";


QConsoleView::QConsoleView()
{
    setMinimumSize(320, 120);

    m_log = new QTextEdit();
    m_prompt = new QLabel();
    m_edit = new QLineEdit();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_log);
    QHBoxLayout *hboxlayout = new QHBoxLayout;
    hboxlayout->addWidget(m_prompt);
    hboxlayout->addWidget(m_edit);
    vboxlayout->addLayout(hboxlayout);
    this->setLayout(vboxlayout);

    QFont font = Common_GetMonospacedFont();
    m_log->setReadOnly(true);
    m_log->setFont(font);
    m_edit->setFont(font);

    QObject::connect(m_edit, SIGNAL(returnPressed()), this, SLOT(execConsoleCommand()));

    this->print("Use 'h' command to show help.\r\n\r\n");
}

QConsoleView::~QConsoleView()
{
    delete m_log;
    delete m_prompt;
    delete m_edit;
}

void QConsoleView::clear()
{
    m_log->clear();
}


void QConsoleView::updatePrompt()
{
    CProcessor* pProc = g_pBoard->GetCPU();
    if (pProc == 0) return;
    char buffer[15];
    _snprintf(buffer, 15, " %06o> ", pProc->GetPC());
    m_prompt->setText(buffer);
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
    m_log->insertPlainText("\r\n");
    m_log->moveCursor(QTextCursor::End);
}

void QConsoleView::printConsolePrompt()
{
    CProcessor* pProc = g_pBoard->GetCPU();
    char buffer[14];
    _snprintf(buffer, 14, "%06o> ", pProc->GetPC());
    this->print(buffer);
}

void QConsoleView::printHelp()
{
    this->print("Console command list:\r\n"
            "  c          Clear console log\r\n"
            "  dXXXXXX    Disassemble from address XXXXXX\r\n"
            "  g          Go; free run\r\n"
            "  gXXXXXX    Go; run processor until breakpoint at address XXXXXX\r\n"
            "  m          Memory dump at current address\r\n"
            "  mXXXXXX    Memory dump at address XXXXXX\r\n"
            "  mrN        Memory dump at address from register N; N=0..7\r\n"
            "  r          Show register values\r\n"
            "  rN         Show value of register N; N=0..7,ps\r\n"
            "  rN XXXXXX  Set register N to value XXXXXX; N=0..7,ps\r\n"
            "  s          Step Into; executes one instruction\r\n"
            "  so         Step Over; executes and stops after the current instruction\r\n"
//            "  u          Save memory dump to file memdumpXPU.bin\r\n"
               );
}

int QConsoleView::printDisassemble(quint16 address, bool okOneInstr, bool okShort)
{
    CProcessor* pProc = g_pBoard->GetCPU();
    bool okHaltMode = pProc->IsHaltMode();

    const int nWindowSize = 30;
    quint16 memory[nWindowSize + 2];
    int addrtype;
    for (int i = 0; i < nWindowSize + 2; i++)
        memory[i] = g_pBoard->GetWordView(address + i * 2, okHaltMode, true, &addrtype);

    char bufaddr[7];
    char bufvalue[7];
    char buffer[64];

    int lastLength = 0;
    int length = 0;
    for (int index = 0; index < nWindowSize; index++)  // Рисуем строки
    {
        PrintOctalValue(bufaddr, address);
        quint16 value = memory[index];
        PrintOctalValue(bufvalue, value);

        if (length > 0)
        {
            if (!okShort)
            {
                _snprintf(buffer, 64, "  %s  %s\r\n", bufaddr, bufvalue);
                this->print(buffer);
            }
        }
        else
        {
            if (okOneInstr && index > 0)
                break;
            char instr[8];
            char args[32];
            length = DisassembleInstruction(memory + index, address, instr, args);
            lastLength = length;
            if (index + length > nWindowSize)
                break;
            if (okShort)
                _snprintf(buffer, 64, "  %s  %-7s %s\r\n", bufaddr, instr, args);
            else
                _snprintf(buffer, 64, "  %s  %s  %-7s %s\r\n", bufaddr, bufvalue, instr, args);
            this->print(buffer);
        }
        length--;
        address += 2;
    }

    return lastLength;
}

void QConsoleView::printRegister(const char * strName, quint16 value)
{
    char buffer[31];
    char* p = buffer;
    *p++ = ' ';
    *p++ = ' ';
    strcpy(p, strName);  p += 2;
    *p++ = ' ';
    PrintOctalValue(p, value);  p += 6;
    *p++ = ' ';
    PrintBinaryValue(p, value);  p += 16;
    *p++ = '\r';
    *p++ = '\n';
    *p++ = 0;
    this->print(buffer);
}

void QConsoleView::printMemoryDump(quint16 address, int lines)
{
    CProcessor* pProc = g_pBoard->GetCPU();

    address &= ~1;  // Line up to even address

    bool okHaltMode = pProc->IsHaltMode();

    for (int line = 0; line < lines; line++)
    {
        quint16 dump[8];
        for (int i = 0; i < 8; i++)
            dump[i] = g_pBoard->GetWord(address + i * 2, okHaltMode);

        char buffer[2 + 6 + 2 + 7 * 8 + 1 + 16 + 1 + 2];
        char* pBuf = buffer;
        *pBuf = ' ';  pBuf++;
        *pBuf = ' ';  pBuf++;
        PrintOctalValue(pBuf, address);  pBuf += 6;
        *pBuf = ' ';  pBuf++;
        *pBuf = ' ';  pBuf++;
        for (int i = 0; i < 8; i++)
        {
            PrintOctalValue(pBuf, dump[i]);  pBuf += 6;
            *pBuf = ' ';  pBuf++;
        }
        *pBuf = ' ';  pBuf++;
//        for (int i = 0; i < 8; i++) {
//            quint16 word = dump[i];
//            quint8 ch1 = LOBYTE(word);
//            char wch1 = Translate_BK_Unicode(ch1);
//            if (ch1 < 32) wch1 = '·');
//            *pBuf = wch1;  pBuf++;
//            quint8 ch2 = HIBYTE(word);
//            char wch2 = Translate_BK_Unicode(ch2);
//            if (ch2 < 32) wch2 = '·');
//            *pBuf = wch2;  pBuf++;
//        }
        *pBuf++ = '\r';
        *pBuf++ = '\n';
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
    this->printConsolePrompt();
    this->printLine(command);

    bool okUpdateAllViews = false;  // Flag - need to update all debug views
    bool okUpdateMenu = false;  // Flag - need to update main menu
    CProcessor* pProc = g_pBoard->GetCPU();

    // Execute the command
    if (command == "h")
    {
        this->printHelp();
    }
    else if (command == "c")  // Clear log
    {
        this->clear();
    }
    else if (command.startsWith("r"))  // Register operations
    {
        if (command.length() == 1)  // Print all registers
        {
            for (int r = 0; r < 8; r++)
            {
                const char * name = REGISTER_NAME[r];
                quint16 value = pProc->GetReg(r);
                this->printRegister(name, value);
            }
        }
        else if (command[1].toLatin1() >= '0' && command[1].toLatin1() <= '7')  // "r0".."r7"
        {
            int r = command[1].toLatin1() - '0';
            const char * name = REGISTER_NAME[r];
            if (command.length() == 2)  // "rN" - show register N
            {
                quint16 value = pProc->GetReg(r);
                this->printRegister(name, value);
            }
            else if (command[2].toLatin1() == '=' || command[2].toLatin1() == ' ')  // "rN=XXXXXX" - set register N to value XXXXXX
            {
                quint16 value;
                if (! ParseOctalValue(command.mid(3), &value))
                    this->print(MESSAGE_WRONG_VALUE);
                else
                {
                    pProc->SetReg(r, value);
                    this->printRegister(name, value);
                    okUpdateAllViews = true;
                }
            }
            else
                this->print(MESSAGE_UNKNOWN_COMMAND);
        }
        else if (command.length() >= 2 && command[1].toLatin1() == 'p' && command[2].toLatin1() == 's')  // "rps"
        {
            if (command.length() == 2)  // "rps" - show PSW
            {
                quint16 value = pProc->GetPSW();
                this->printRegister("PS", value);
            }
            else if (command[3].toLatin1() == '=' || command[3].toLatin1() == ' ')  // "rps=XXXXXX" - set PSW to value XXXXXX
            {
                quint16 value;
                if (! ParseOctalValue(command.mid(4), &value))
                    this->print(MESSAGE_WRONG_VALUE);
                else
                {
                    pProc->SetPSW(value);
                    this->printRegister("PS", value);
                    okUpdateAllViews = true;
                }
            }
            else
                this->print(MESSAGE_UNKNOWN_COMMAND);
        }
        else
            this->print(MESSAGE_UNKNOWN_COMMAND);
    }
    else if (command == "s")  // "s" - Step Into, execute one instruction
    {
        this->printDisassemble(pProc->GetPC(), true, false);

        //pProc->Execute();
        g_pBoard->DebugTicks();

        okUpdateAllViews = true;
    }
    else if (command == "so")  // "so" - Step Over
    {
        int instrLength = this->printDisassemble(pProc->GetPC(), true, false);
        quint16 bpaddress = pProc->GetPC() + instrLength * 2;

        Emulator_SetCPUBreakpoint(bpaddress);
        Emulator_Start();

        okUpdateMenu = true;
    }
    else if (command.startsWith("d") ||  // Disassemble
            command.startsWith("D"))    // Disassemble, short format
    {
        bool okShort = (command[0] == 'D');
        if (command.length() == 1)  // "d" - disassemble at current address
            this->printDisassemble(pProc->GetPC(), false, okShort);
        else if (command[1].toLatin1() >= '0' && command[1].toLatin1() <= '7')  // "dXXXXXX" - disassemble at address XXXXXX
        {
            quint16 value;
            if (! ParseOctalValue(command.mid(1), &value))
                this->print(MESSAGE_WRONG_VALUE);
            else
                this->printDisassemble(value, false, okShort);
        }
        else
            this->print(MESSAGE_UNKNOWN_COMMAND);
    }
    else if (command.startsWith("m"))
    {
        if (command.length() == 1)  // "m" - dump memory at current address
        {
            this->printMemoryDump(pProc->GetPC(), 8);
        }
        else if (command[1].toLatin1() >= '0' && command[1].toLatin1() <= '7')  // "mXXXXXX" - dump memory at address XXXXXX
        {
            quint16 value;
            if (! ParseOctalValue(command.mid(1), &value))
                this->print(MESSAGE_WRONG_VALUE);
            else
                this->printMemoryDump(value, 8);
        }
        else if (command[1].toLatin1() == 'r' && command.length() >= 3 &&
                command[2].toLatin1() >= '0' && command[2].toLatin1() <= '7')  // "mrN" - dump memory at address from register N
        {
            int r = command[2].toLatin1() - '0';
            quint16 address = pProc->GetReg(r);
            this->printMemoryDump(address, 8);
        }
        else
            this->print(MESSAGE_UNKNOWN_COMMAND);
        //TODO: "mXXXXXX YYYYYY" - set memory cell at XXXXXX to value YYYYYY
        //TODO: "mrN YYYYYY" - set memory cell at address from rN to value YYYYYY
    }
    else if (command == "g")  // Go
    {
        Emulator_Start();
        okUpdateAllViews = true;
    }
    else if (command.startsWith("g"))  // Go
    {
        quint16 value;
        if (! ParseOctalValue(command.mid(1), &value))
            this->print(MESSAGE_WRONG_VALUE);
        else
        {
            Emulator_SetCPUBreakpoint(value);
            Emulator_Start();

            okUpdateMenu = true;
        }
    }
    else
    {
        this->print(MESSAGE_UNKNOWN_COMMAND);
    }

    if (okUpdateAllViews)
        Global_UpdateAllViews();
    else if (okUpdateMenu)
        Global_UpdateMenu();

}
