#include "stdafx.h"
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
#include "emubase/Processor.h"
#include "qscripting.h"


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

    Global_getMainWindow()->updateAllViews();
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
            Global_getMainWindow()->updateAllViews();
            Global_getApplication()->processEvents();
            if (m_window->isAborted())
                return false;
        }
    }

    Global_getMainWindow()->updateAllViews();
    Global_getApplication()->processEvents();
    if (m_window->isAborted())
        return false;

    return result;
}

float QEmulator::getUptime()
{
    return Emulator_GetUptime();
}

bool QEmulator::addCPUBreakpoint(quint16 address)
{
    return Emulator_AddCPUBreakpoint((quint16)address);
}
bool QEmulator::removeCPUBreakpoint(quint16 address)
{
    return Emulator_RemoveCPUBreakpoint((quint16)address);
}
bool QEmulator::isBreakpoint()
{
    return Emulator_IsBreakpoint();
}
QScriptValue QEmulator::getCPUBreakpoints()
{
    const quint16* bps = Emulator_GetCPUBreakpointList();

    int count = 0;
    for (int i = 0; i < MAX_BREAKPOINTCOUNT; i++)
    {
        if (bps[i] == 0177777)
            break;
        count++;
    }

    QScriptEngine* engine = m_window->getEngine();
    QScriptValue list = engine->newArray(count);
    for (int i = 0; i < count; i++)
    {
        list.setProperty(i, engine->newVariant(bps[i]));
    }
    return list;
}

void QEmulator::saveScreenshot(const QString &filename)
{
    Global_getMainWindow()->saveScreenshot(filename);
}

ushort QEmulator::readWord(ushort addr)
{
    int addrType;
    return g_pBoard->GetWordView(addr, g_pBoard->GetCPU()->IsHaltMode(), false, &addrType);
}
uchar QEmulator::readByte(uint16_t addr)
{
    int addrType;
    uint16_t word = g_pBoard->GetWordView(addr, g_pBoard->GetCPU()->IsHaltMode(), false, &addrType);
    if (addrType == ADDRTYPE_DENY)
        return 0;
    return (addr & 1) ? word & 0xff : (word >> 8) & 0xff;
}

void QEmulator::keyScan(uchar bkscan, int timeout)
{
    g_pBoard->KeyboardEvent(bkscan, true, false);
    run(timeout);
    g_pBoard->KeyboardEvent(bkscan, false, false);
    run(3);
}

const uchar arrChar2BkScan[128] =
{
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
        char ch = str[i].toLatin1();
        keyChar(ch);
        if (m_window->isAborted())
            return;
    }
}

QScriptValue QEmulator::disassemble(ushort addr)
{
    quint16 buffer[4];
    quint16 current = addr;
    for (int i = 0; i < 4; i++)
    {
        int addrType;
        buffer[i] = g_pBoard->GetWordView(current, g_pBoard->GetCPU()->IsHaltMode(), false, &addrType);
        current += 2;
    }

    char instr[8], args[32];
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

    const quint16 * pDataHeader = (const quint16 *)dataHeader.constData();
    quint16 baseAddress = pDataHeader[0];
    quint16 dataSize = pDataHeader[1];
    quint16 memoryBytes = (dataSize + 1) & 0xfffe;

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
    quint16 address = baseAddress;
    const quint16 * pData = (const quint16 *)data.constData();
    while (address < baseAddress + memoryBytes)
    {
        quint16 value = *pData++;
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
    Global_getMainWindow()->updateAllViews();
}
void QEmulatorProcessor::setPSW(ushort value)
{
    m_processor->SetPSW(value);
    Global_getMainWindow()->updateAllViews();
}


//////////////////////////////////////////////////////////////////////
// QScriptWindow

QScriptWindow::QScriptWindow(QWidget * parent)
    : QDialog(parent, Qt::Dialog),
      m_aborted(false)
{
    setWindowTitle(tr("Script Running"));
    setMinimumSize(300, 125);
    setMaximumSize(400, 200);
    m_cancelButton.setText(tr("Stop"));
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
        m_static.setText(tr("Syntax check..."));
        Global_getApplication()->processEvents();

        QScriptSyntaxCheckResult checkResult = QScriptEngine::checkSyntax(script);
        if (checkResult.state() != QScriptSyntaxCheckResult::Valid)
        {
            message.append(tr("Syntax check FAILED:\n\n%1\n\nat line %2 column %3.")
                    .arg(checkResult.errorMessage())
                    .arg(checkResult.errorLineNumber())
                    .arg(checkResult.errorColumnNumber()));
            break;
        }

        m_static.setText(tr("Running script..."));
        Global_getApplication()->processEvents();

        m_engine.setProcessEventsInterval(250);
        QScriptValue result = m_engine.evaluate(script);
        if (m_aborted)
        {
            message.append(tr("The script was STOPPED."));
            break;
        }

        message.append(tr("The script FINISHED. The result is:\n\n"))
        .append(result.toString());
        break;
    }

    m_static.setText(message);
    m_cancelButton.setText(tr("Close"));
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


//////////////////////////////////////////////////////////////////////
