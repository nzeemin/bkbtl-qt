#include "stdafx.h"
#include <QApplication>
#include <QCloseEvent>
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"
#include "qscripting.h"
#include "emubase/Processor.h"


//////////////////////////////////////////////////////////////////////
// QEmulator

QEmulator::QEmulator(QScriptWindow * window) :
    m_window(window),
    m_cpu(window->getEngine(), g_pBoard->GetCPU())
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


//////////////////////////////////////////////////////////////////////
// QEmulatorProcessor

QEmulatorProcessor::QEmulatorProcessor(QScriptEngine *engine, CProcessor *processor)
    : m_engine(engine), m_processor(processor)
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

    m_emulator = new QEmulator(this);
    m_engine.globalObject().setProperty("emulator", m_engine.newQObject(m_emulator));
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
