#include "stdafx.h"
#include <QApplication>
#include <QCloseEvent>
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

void QEmulator::saveScreenshot(QString filename)
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
