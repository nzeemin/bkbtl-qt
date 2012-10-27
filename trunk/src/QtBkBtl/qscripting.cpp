#include "stdafx.h"
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"
#include "qscripting.h"
#include "emubase/Emubase.h"
#include "emubase/Processor.h"


//////////////////////////////////////////////////////////////////////
// QEmulator

QEmulator::QEmulator(QScriptWindow * window, QScriptEngine *engine) :
    m_window(window),
    m_engine(engine),
    m_cpu(g_pBoard->GetCPU())
{
}

void QEmulator::reset()
{
    Emulator_Reset();

    Global_getMainWindow()->UpdateAllViews();
}

bool QEmulator::run(int frames)
{
    bool result = true;
    for (int i = 0; i < frames; i++)
    {
        int res = Emulator_SystemFrame();
        if (!res)
        {
            result = false;
            break;
        }

        if (i % 25 == 24)  // Update the screen every 25 frames
        {
            Global_getMainWindow()->UpdateAllViews();
            Global_getApplication()->processEvents();
            if (m_window->isAborted())
                return false;
        }
    }

    Global_getMainWindow()->UpdateAllViews();

    return result;
}

float QEmulator::getUptime()
{
    return Emulator_GetUptime();
}

void QEmulator::setBreakpoint(quint16 address)
{
    Emulator_SetCPUBreakpoint((WORD)address);
}
bool QEmulator::isBreakpoint()
{
    return Emulator_IsBreakpoint();
}

void QEmulator::saveScreenshot(const QString &filename)
{
    Global_getMainWindow()->saveScreenshot(filename);
}

ushort QEmulator::readWord(ushort addr)
{
    BOOL okValid;
    return g_pBoard->GetWordView(addr, g_pBoard->GetCPU()->IsHaltMode(), FALSE, &okValid);
}
uchar QEmulator::readByte(ushort addr)
{
    BOOL okValid;
    ushort word = g_pBoard->GetWordView(addr, g_pBoard->GetCPU()->IsHaltMode(), FALSE, &okValid);
    if (!okValid)
        return 0;
    return (addr & 1) ? word & 0xff : (word >> 8) & 0xff;
}

void QEmulator::keyScan(uchar bkscan, int timeout)
{
    g_pBoard->KeyboardEvent(bkscan, TRUE, FALSE);
    run(timeout);
    g_pBoard->KeyboardEvent(bkscan, FALSE, FALSE);
    run(3);
}

const uchar arrChar2BkScan[128] = {
/*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
/*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0030, 0000, 0012, 0000, 0000, 0000, 0000, 0000,
/*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
/*2*/    0040, 0041, 0042, 0043, 0044, 0045, 0046, 0047, 0050, 0051, 0052, 0053, 0054, 0055, 0056, 0057,
/*3*/    0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067, 0070, 0071, 0072, 0073, 0074, 0275, 0076, 0077,
/*4*/    0100, 0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
/*5*/    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127, 0130, 0131, 0132, 0133, 0134, 0135, 0136, 0137,
/*6*/    0140, 0141, 0142, 0143, 0144, 0145, 0146, 0147, 0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157,
/*7*/    0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167, 0170, 0171, 0172, 0173, 0174, 0175, 0176, 0000,
};

void QEmulator::keyChar(char ch, int timeout)
{
    if (ch < 0) return;  // ASCII only

    uchar scan = arrChar2BkScan[(uchar)ch];
    if (scan == 0)
        return;
    keyScan(scan, timeout);
}

void QEmulator::keyString(const QString& str)
{
    for (int i = 0; i < str.length(); i++)
    {
        char ch = str[i].toAscii();
        keyChar(ch);
        if (m_window->isAborted())
            return;
    }
}

QScriptValue QEmulator::disassemble(ushort addr)
{
    WORD buffer[4];
    WORD current = addr;
    for (int i = 0; i < 4; i++)
    {
        BOOL okValid;
        buffer[i] = g_pBoard->GetWordView(current, g_pBoard->GetCPU()->IsHaltMode(), FALSE, &okValid);
        current += 2;
    }

    TCHAR instr[8], args[32];
    int instrlen = DisassembleInstruction(buffer, addr, instr, args);

    QScriptValue list = m_engine->newArray(4);
    list.setProperty(0, m_engine->newVariant(addr));
    list.setProperty(1, m_engine->newVariant(instr));
    list.setProperty(2, m_engine->newVariant(args));
    list.setProperty(3, m_engine->newVariant(instrlen));
    return list;
}

void QEmulator::consolePrint(const QString &message)
{
    Global_getMainWindow()->consolePrint(message);
}

bool QEmulator::loadBin(const QString &binfilename)
{
    // Open file for reading
    QFile file(binfilename);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // Load header
    QByteArray dataHeader = file.read(4);
    if (file.error())
    {
        file.close();
        return false;
    }

    const WORD * pDataHeader = (const WORD *)dataHeader.constData();
    WORD baseAddress = pDataHeader[0];
    WORD dataSize = pDataHeader[1];
    WORD memoryBytes = (dataSize + 1) & 0xfffe;

    // Load file data
    QByteArray data = file.readAll();
    if (file.error())
    {
        file.close();
        return false;
    }
    file.close();
    if (data.length() != dataSize)
        return false;

    // Copy data to BK memory
    WORD address = baseAddress;
    const WORD * pData = (const WORD *)data.constData();
    while (address < baseAddress + memoryBytes)
    {
        WORD value = *pData++;
        g_pBoard->SetRAMWord(address, value);
        address += 2;
    }

    data.clear();

    return true;
}


//////////////////////////////////////////////////////////////////////
// QEmulatorProcessor

QEmulatorProcessor::QEmulatorProcessor(CProcessor *processor)
    : m_processor(processor)
{
}

ushort QEmulatorProcessor::getReg(int regno)
{
    if (regno < 0 || regno > 7) return 0;
    return m_processor->GetReg(regno);
}
ushort QEmulatorProcessor::getPSW()
{
    return m_processor->GetPSW();
}

void QEmulatorProcessor::setReg(int regno, ushort value)
{
    if (regno < 0 || regno > 7) return;
    m_processor->SetReg(regno, value);
    Global_getMainWindow()->UpdateAllViews();
}
void QEmulatorProcessor::setPSW(ushort value)
{
    m_processor->SetPSW(value);
    Global_getMainWindow()->UpdateAllViews();
}


//////////////////////////////////////////////////////////////////////
// QScriptWindow

QScriptWindow::QScriptWindow(QWidget * parent)
    : QDialog(parent, Qt::Dialog),
      m_aborted(false)
{
    setWindowTitle("Script Running");
    setMinimumSize(300, 125);
    setMaximumSize(400, 200);
    m_cancelButton.setText("Stop");
    m_vlayout.addWidget(&m_static, 0, 0);
    m_vlayout.addWidget(&m_cancelButton, 0, Qt::AlignHCenter);
    setLayout(&m_vlayout);
    setModal(true);

    QObject::connect(&m_cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPressed()));

    m_emulator = new QEmulator(this, &m_engine);
    m_engine.globalObject().setProperty("emulator", m_engine.newQObject(m_emulator));
    m_engine.globalObject().setProperty("emu", m_engine.newQObject(m_emulator));
}

QScriptWindow::~QScriptWindow()
{
    delete m_emulator;
}

void QScriptWindow::runScript(const QString & script)
{
    show();
    raise();
    activateWindow();

    QString message;
    for (;;)
    {
        m_static.setText("Syntax check...");
        Global_getApplication()->processEvents();

        QScriptSyntaxCheckResult checkResult = QScriptEngine::checkSyntax(script);
        if (checkResult.state() != QScriptSyntaxCheckResult::Valid)
        {
            message.append("Syntax check FAILED:\n\n%1\n\nat line %2 column %3.")
                .arg(checkResult.errorMessage())
                .arg(checkResult.errorLineNumber())
                .arg(checkResult.errorColumnNumber());
            break;
        }

        m_static.setText("Running script...");
        Global_getApplication()->processEvents();

        m_engine.setProcessEventsInterval(250);
        QScriptValue result = m_engine.evaluate(script);
        if (m_aborted)
        {
            message.append("The script was STOPPED.");
            break;
        }

        message.append("The script FINISHED. The result is:\n\n")
            .append(result.toString());
        break;
    }

    m_static.setText(message);
    m_cancelButton.setText("Close");
    exec();
}

void QScriptWindow::cancelButtonPressed()
{
    if (m_engine.isEvaluating())
    {
        m_aborted = true;
        m_engine.abortEvaluation();
    }
    else  // The script is not running
    {
        QDialog::reject();  // Close the window
    }
}

void QScriptWindow::reject()
{
    if (m_engine.isEvaluating())
    {
        m_aborted = true;
        m_engine.abortEvaluation();
    }
    else  // The script is not running
    {
        QDialog::reject();
    }
}
