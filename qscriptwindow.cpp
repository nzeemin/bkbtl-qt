#include "stdafx.h"
#include "qscriptwindow.h"
#include "main.h"
#include "qemulator.h"
#include <QApplication>

QScriptWindow::QScriptWindow(QWidget * parent)
    : QDialog(parent, Qt::Dialog),
      m_aborted(false)
{
    setWindowTitle("Script Running");
    setMinimumSize(300, 125);
    setMaximumSize(360, 160);
    m_cancelButton.setText("Stop");
    m_vlayout.addWidget(&m_static, 0, 0);
    m_vlayout.addWidget(&m_cancelButton, 0, Qt::AlignHCenter);
    setLayout(&m_vlayout);

    QObject::connect(&m_cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPressed()));

    m_emulator = new QEmulator();
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
            message.append("Script was STOPPED.");
            break;
        }

        message.append("Script finished. The result is:\n\n")
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
        reject();  // Close the window
    }
}
