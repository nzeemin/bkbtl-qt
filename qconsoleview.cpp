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

QString QConsoleView::MESSAGE_UNKNOWN_COMMAND;
QString QConsoleView::MESSAGE_WRONG_VALUE;
QString QConsoleView::MESSAGE_INVALID_REGNUM;

QConsoleView::QConsoleView()
{
    MESSAGE_UNKNOWN_COMMAND = QConsoleView::tr("  Unknown command.\r\n");
    MESSAGE_WRONG_VALUE = QConsoleView::tr("  Wrong value.\r\n");
    MESSAGE_INVALID_REGNUM = QConsoleView::tr("  Invalid register number, 0..7 expected.\r\n");

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

    this->print(tr("Use 'h' command to show help.\r\n\r\n"));
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

CProcessor* QConsoleView::getCurrentProcessor()
{
    return g_pBoard->GetCPU();
}

void QConsoleView::updatePrompt()
{
    CProcessor* pProc = getCurrentProcessor();
    if (pProc == nullptr) return;
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
    CProcessor* pProc = getCurrentProcessor();
    char buffer[14];
    _snprintf(buffer, 14, "%06o> ", pProc->GetPC());
    this->print(buffer);
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
    CProcessor* pProc = getCurrentProcessor();

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
//            if (ch1 < 32) wch1 = '·';
//            *pBuf = wch1;  pBuf++;
//            quint8 ch2 = HIBYTE(word);
//            char wch2 = Translate_BK_Unicode(ch2);
//            if (ch2 < 32) wch2 = '·';
//            *pBuf = wch2;  pBuf++;
//        }
        *pBuf++ = '\r';
        *pBuf++ = '\n';
        *pBuf = 0;

        this->print(buffer);

        address += 16;
    }
}

// Print disassembled instructions
// Return value: number of words disassembled
int QConsoleView::printDisassemble(quint16 address, bool okOneInstr, bool okShort)
{
    CProcessor* pProc = getCurrentProcessor();
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
    for (int index = 0; index < nWindowSize; index++)  // Draw strings
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

void QConsoleView::cmdShowHelp(const ConsoleCommandParams& /*params*/)
{
    this->print(tr("Console command list:\r\n"
            "  c          Clear console log\r\n"
            "  d          Disassemble from PC; use D for short format\r\n"
            "  dXXXXXX    Disassemble from address XXXXXX\r\n"
            "  g          Go; free run\r\n"
            "  gXXXXXX    Go; run processor until breakpoint at address XXXXXX\r\n"
            "  m          Memory dump at current address\r\n"
            "  mXXXXXX    Memory dump at address XXXXXX\r\n"
            "  mrN        Memory dump at address from register N; N=0..7\r\n"
            "  mXXXXXX YYYYYY  Set memory value at address XXXXXX\r\n"
            "  r          Show register values\r\n"
            "  rN         Show value of register N; N=0..7,ps\r\n"
            "  rN XXXXXX  Set register N to value XXXXXX; N=0..7,ps\r\n"
            "  s          Step Into; executes one instruction\r\n"
            "  so         Step Over; executes and stops after the current instruction\r\n"
            "  b          List breakpoints set\r\n"
            "  bXXXXXX    Set breakpoint at address XXXXXX\r\n"
            "  bcXXXXXX   Remove breakpoint at address XXXXXX\r\n"
            "  bc         Remove all breakpoints\r\n"
//            "  u          Save memory dump to file memdumpXPU.bin\r\n"
                  ));
}

void QConsoleView::cmdClearConsoleLog(const ConsoleCommandParams& /*params*/)
{
    this->clear();
}

void QConsoleView::cmdSetRegisterValue(const ConsoleCommandParams & params)
{
    int r = params.paramReg1;
    quint16 value = params.paramOct1;

    CProcessor* pProc = getCurrentProcessor();

    pProc->SetReg(r, value);
    LPCTSTR name = REGISTER_NAME[r];
    this->printRegister(name, value);

    Global_UpdateAllViews();
}

void QConsoleView::cmdPrintRegister(const ConsoleCommandParams & params)
{
    int r = params.paramReg1;

    const CProcessor* pProc = getCurrentProcessor();

    quint16 value = pProc->GetReg(r);
    LPCTSTR name = REGISTER_NAME[r];
    this->printRegister(name, value);
}

void QConsoleView::cmdPrintAllRegisters(const ConsoleCommandParams &)
{
    const CProcessor* pProc = getCurrentProcessor();

    for (int r = 0; r < 8; r++)
    {
        LPCTSTR name = REGISTER_NAME[r];
        quint16 value = pProc->GetReg(r);
        this->printRegister(name, value);
    }
}

void QConsoleView::cmdSetRegisterPSW(const ConsoleCommandParams & params)
{
    quint16 value = params.paramOct1;

    CProcessor* pProc = getCurrentProcessor();

    pProc->SetPSW(value);
    this->printRegister("PS", value);
    Global_UpdateAllViews();
}

void QConsoleView::cmdPrintRegisterPSW(const ConsoleCommandParams &)
{
    const CProcessor* pProc = getCurrentProcessor();

    quint16 value = pProc->GetPSW();
    this->printRegister("PS", value);
}

void QConsoleView::cmdStepInto(const ConsoleCommandParams &)
{
    CProcessor* pProc = getCurrentProcessor();

    this->printDisassemble(pProc->GetPC(), true, false);

    //pProc->Execute();
    g_pBoard->DebugTicks();

    Global_UpdateAllViews();
}

void QConsoleView::cmdStepOver(const ConsoleCommandParams &)
{
    CProcessor* pProc = getCurrentProcessor();

    int instrLength = this->printDisassemble(pProc->GetPC(), true, false);
    quint16 bpaddress = pProc->GetPC() + instrLength * 2;

    Emulator_SetTempCPUBreakpoint(bpaddress);
    Emulator_Start();

    Global_UpdateMenu();
}

void QConsoleView::cmdPrintDisassembleAtAddress(const ConsoleCommandParams & params)
{
    quint16 address = params.paramOct1;
    bool okShort = (params.commandText[0] == _T('D'));

    printDisassemble(address, false, okShort);
}

void QConsoleView::cmdPrintDisassembleAtPC(const ConsoleCommandParams & params)
{
    CProcessor* pProc = getCurrentProcessor();

    uint16_t address = pProc->GetPC();
    bool okShort = (params.commandText[0] == _T('D'));

    printDisassemble(address, false, okShort);
}

//void QConsoleView::cmdSetMemoryAtAddress(const ConsoleCommandParams & params)

void QConsoleView::cmdPrintMemoryDumpAtAddress(const ConsoleCommandParams & params)
{
    quint16 address = params.paramOct1;

    printMemoryDump(address);
}

void QConsoleView::cmdPrintMemoryDumpAtRegister(const ConsoleCommandParams & params)
{
    int r = params.paramReg1;

    CProcessor* pProc = getCurrentProcessor();
    quint16 address = pProc->GetReg(r);

    printMemoryDump(address);
}

void QConsoleView::cmdPrintMemoryDumpAtPC(const ConsoleCommandParams &)
{
    CProcessor* pProc = getCurrentProcessor();
    quint16 address = pProc->GetPC();

    printMemoryDump(address);
}

void QConsoleView::cmdRunToAddress(const ConsoleCommandParams & params)
{
    quint16 value = params.paramOct1;

    Emulator_SetTempCPUBreakpoint(value);
    Emulator_Start();

    Global_UpdateMenu();
}

void QConsoleView::cmdRun(const ConsoleCommandParams &)
{
    Emulator_Start();
    Global_UpdateAllViews();
}

void QConsoleView::cmdSetBreakpointAtAddress(const ConsoleCommandParams & params)
{
    quint16 value = params.paramOct1;

    bool result = Emulator_AddCPUBreakpoint(value);
    if (!result)
        this->print(tr("  Failed to add breakpoint.\r\n"));
    Global_RedrawDebugView();
    Global_RedrawDisasmView();
}

void QConsoleView::cmdPrintAllBreakpoints(const ConsoleCommandParams &)
{
    const quint16* pbps = Emulator_GetCPUBreakpointList();
    if (pbps == nullptr || *pbps == 0177777)
    {
        this->print(tr("  No breakpoints.\r\n"));
        return;
    }

    while (*pbps != 0177777)
    {
        QString line;  line.sprintf("  %06ho\r\n", *pbps);
        this->print(line);
        pbps++;
    }
}

void QConsoleView::cmdRemoveBreakpointAtAddress(const ConsoleCommandParams & params)
{
    quint16 value = params.paramOct1;

    bool result = Emulator_RemoveCPUBreakpoint(value);
    if (!result)
        this->print("  Failed to remove breakpoint.\r\n");
    Global_RedrawDebugView();
    Global_RedrawDisasmView();
}

void QConsoleView::cmdRemoveAllBreakpoints(const ConsoleCommandParams &)
{
    Emulator_RemoveAllBreakpoints();
    Global_RedrawDebugView();
    Global_RedrawDisasmView();
}


enum ConsoleCommandArgInfo
{
    ARGINFO_NONE,     // No parameters
    ARGINFO_REG,      // Register number 0..7
    ARGINFO_OCT,      // Octal value
    ARGINFO_REG_OCT,  // Register number, octal value
    ARGINFO_OCT_OCT,  // Octal value, octal value
};

typedef void(QConsoleView::* CONSOLE_COMMAND_CALLBACK)(const ConsoleCommandParams& params);

struct ConsoleCommandStruct
{
    LPCTSTR pattern;
    ConsoleCommandArgInfo arginfo;
    CONSOLE_COMMAND_CALLBACK callback;
}
static ConsoleCommands[] =
{
    // IMPORTANT! First list more complex forms with more arguments, then less complex forms
    { _T("h"), ARGINFO_NONE, &QConsoleView::cmdShowHelp },
    { _T("c"), ARGINFO_NONE, &QConsoleView::cmdClearConsoleLog },
    { _T("r%d=%ho"), ARGINFO_REG_OCT, &QConsoleView::cmdSetRegisterValue },
    { _T("r%d %ho"), ARGINFO_REG_OCT, &QConsoleView::cmdSetRegisterValue },
    { _T("r%d"), ARGINFO_REG, &QConsoleView::cmdPrintRegister },
    { _T("r"), ARGINFO_NONE, &QConsoleView::cmdPrintAllRegisters },
    { _T("rps=%ho"), ARGINFO_OCT, &QConsoleView::cmdSetRegisterPSW },
    { _T("rps %ho"), ARGINFO_OCT, &QConsoleView::cmdSetRegisterPSW },
    { _T("rps"), ARGINFO_NONE, &QConsoleView::cmdPrintRegisterPSW },
    { _T("s"), ARGINFO_NONE, &QConsoleView::cmdStepInto },
    { _T("so"), ARGINFO_NONE, &QConsoleView::cmdStepOver },
    { _T("d%ho"), ARGINFO_OCT, &QConsoleView::cmdPrintDisassembleAtAddress },
    { _T("D%ho"), ARGINFO_OCT, &QConsoleView::cmdPrintDisassembleAtAddress },
    { _T("d"), ARGINFO_NONE, &QConsoleView::cmdPrintDisassembleAtPC },
    { _T("D"), ARGINFO_NONE, &QConsoleView::cmdPrintDisassembleAtPC },
    //    { _T("u"), ARGINFO_NONE, &QConsoleView::cmdSaveMemoryDump },
    //    { _T("m%ho %ho"), ARGINFO_OCT_OCT, &QConsoleView::cmdSetMemoryAtAddress },
    //    { _T("m%ho=%ho"), ARGINFO_OCT_OCT, &QConsoleView::cmdSetMemoryAtAddress },
    { _T("m%ho"), ARGINFO_OCT, &QConsoleView::cmdPrintMemoryDumpAtAddress },
    { _T("mr%d"), ARGINFO_REG, &QConsoleView::cmdPrintMemoryDumpAtRegister },
    { _T("m"), ARGINFO_NONE, &QConsoleView::cmdPrintMemoryDumpAtPC },
    { _T("g%ho"), ARGINFO_OCT, &QConsoleView::cmdRunToAddress },
    { _T("g"), ARGINFO_NONE, &QConsoleView::cmdRun },
    { _T("b%ho"), ARGINFO_OCT, &QConsoleView::cmdSetBreakpointAtAddress },
    { _T("b"), ARGINFO_NONE, &QConsoleView::cmdPrintAllBreakpoints },
    { _T("bc%ho"), ARGINFO_OCT, &QConsoleView::cmdRemoveBreakpointAtAddress },
    { _T("bc"), ARGINFO_NONE, &QConsoleView::cmdRemoveAllBreakpoints },
};
const size_t ConsoleCommandsCount = sizeof(ConsoleCommands) / sizeof(ConsoleCommands[0]);

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

    ConsoleCommandParams params;
    params.commandText = command;
    params.paramReg1 = -1;
    params.paramOct1 = 0;
    params.paramOct2 = 0;
    // Find matching console command from the list, parse and execute the command
    bool parsedOkay = false, parseError = false;
    for (size_t i = 0; i < ConsoleCommandsCount; i++)
    {
        ConsoleCommandStruct& cmd = ConsoleCommands[i];

        int paramsParsed = 0;
        switch (cmd.arginfo)
        {
        case ARGINFO_NONE:
            parsedOkay = (command.compare(cmd.pattern) == 0);
            break;
        case ARGINFO_REG:
            paramsParsed = sscanf(command.toLatin1(), cmd.pattern, &params.paramReg1);
            parsedOkay = (paramsParsed == 1);
            if (parsedOkay && (params.paramReg1 < 0 || params.paramReg1 > 7))
            {
                print(MESSAGE_INVALID_REGNUM);
                parseError = true;
            }
            break;
        case ARGINFO_OCT:
            paramsParsed = sscanf(command.toLatin1(), cmd.pattern, &params.paramOct1);
            parsedOkay = (paramsParsed == 1);
            break;
        case ARGINFO_REG_OCT:
            paramsParsed = sscanf(command.toLatin1(), cmd.pattern, &params.paramReg1, &params.paramOct1);
            parsedOkay = (paramsParsed == 2);
            if (parsedOkay && (params.paramReg1 < 0 || params.paramReg1 > 7))
            {
                print(MESSAGE_INVALID_REGNUM);
                parseError = true;
            }
            break;
        case ARGINFO_OCT_OCT:
            paramsParsed = sscanf(command.toLatin1(), cmd.pattern, &params.paramOct1, &params.paramOct2);
            parsedOkay = (paramsParsed == 2);
            break;
        }

        if (parseError)
            break;  // Validation detected error and printed the message already

        if (parsedOkay)
        {
            (this->*(cmd.callback))(params);  // Execute the command
            break;
        }
    }

    if (!parsedOkay && !parseError)
        print(MESSAGE_UNKNOWN_COMMAND);

    //printConsolePrompt();
}
