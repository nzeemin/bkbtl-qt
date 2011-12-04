#include "stdafx.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QSettings>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QLabel>
#include <QScriptEngine>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscreen.h"
#include "qkeyboardview.h"
#include "qconsoleview.h"
#include "qdebugview.h"
#include "qdisasmview.h"
#include "qmemoryview.h"
#include "qteletypeview.h"
#include "qtapeview.h"
#include "qscripting.h"
#include "Emulator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(_T("BK Back to Life"));

    // Assign signals
    QObject::connect(ui->actionFileLoadBin, SIGNAL(triggered()), this, SLOT(fileLoadBin()));
    QObject::connect(ui->actionFileScreenshot, SIGNAL(triggered()), this, SLOT(fileScreenshot()));
    QObject::connect(ui->actionFileExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionEmulatorRun, SIGNAL(triggered()), this, SLOT(emulatorRun()));
    QObject::connect(ui->actionEmulatorReset, SIGNAL(triggered()), this, SLOT(emulatorReset()));
    QObject::connect(ui->actionEmulatorColorScreen, SIGNAL(triggered()), this, SLOT(emulatorColorScreen()));
    QObject::connect(ui->actionEmulatorScreen0, SIGNAL(triggered()), this, SLOT(emulatorScreen0()));
    QObject::connect(ui->actionEmulatorScreen1, SIGNAL(triggered()), this, SLOT(emulatorScreen1()));
    QObject::connect(ui->actionEmulatorScreen2, SIGNAL(triggered()), this, SLOT(emulatorScreen2()));
    QObject::connect(ui->actionEmulatorScreen3, SIGNAL(triggered()), this, SLOT(emulatorScreen3()));
    QObject::connect(ui->actionConfBK10Basic, SIGNAL(triggered()), this, SLOT(configurationBK0010Basic()));
    QObject::connect(ui->actionConfBK10Focal, SIGNAL(triggered()), this, SLOT(configurationBK0010Focal()));
    QObject::connect(ui->actionConfBK10Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0010Fdd()));
    QObject::connect(ui->actionConfBK11, SIGNAL(triggered()), this, SLOT(configurationBK0011()));
    QObject::connect(ui->actionConfBK11Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0011Fdd()));
    QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), this, SLOT(emulatorFloppy0()));
    QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), this, SLOT(emulatorFloppy1()));
    QObject::connect(ui->actionDrivesFloppy2, SIGNAL(triggered()), this, SLOT(emulatorFloppy2()));
    QObject::connect(ui->actionDrivesFloppy3, SIGNAL(triggered()), this, SLOT(emulatorFloppy3()));
    QObject::connect(ui->actionDebugConsoleView, SIGNAL(triggered()), this, SLOT(debugConsoleView()));
    QObject::connect(ui->actionDebugDebugView, SIGNAL(triggered()), this, SLOT(debugDebugView()));
    QObject::connect(ui->actionDebugDisasmView, SIGNAL(triggered()), this, SLOT(debugDisasmView()));
    QObject::connect(ui->actionDebugMemoryView, SIGNAL(triggered()), this, SLOT(debugMemoryView()));
    QObject::connect(ui->actionDebugTeletypeView, SIGNAL(triggered()), this, SLOT(debugTeletypeView()));
    QObject::connect(ui->actionDebugStepInto, SIGNAL(triggered()), this, SLOT(debugStepInto()));
    QObject::connect(ui->actionDebugStepOver, SIGNAL(triggered()), this, SLOT(debugStepOver()));
    QObject::connect(ui->actionScriptRun, SIGNAL(triggered()), this, SLOT(scriptRun()));
    QObject::connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    QObject::connect(ui->actionHelpAboutQt, SIGNAL(triggered()), this, SLOT(helpAboutQt()));

    // Views
    m_screen = new QScreen();
    m_keyboard = new QKeyboardView();
    m_console = new QConsoleView();
    m_debug = new QDebugView();
    m_disasm = new QDisasmView();
    m_memory = new QMemoryView();
    m_teletype = new QTeletypeView();
    m_tape = new QTapeView();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(0);
    vboxlayout->addWidget(m_screen);
    vboxlayout->addWidget(m_keyboard);
    ui->centralWidget->setLayout(vboxlayout);
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    int maxwid = m_screen->maximumWidth() > m_keyboard->maximumWidth() ? m_screen->maximumWidth() : m_keyboard->maximumWidth();
    ui->centralWidget->setMaximumWidth(maxwid);

    m_dockDebug = new QDockWidget(_T("Processor"));
    m_dockDebug->setObjectName(_T("dockDebug"));
    m_dockDebug->setWidget(m_debug);
    m_dockDisasm = new QDockWidget(_T("Disassemble"));
    m_dockDisasm->setObjectName(_T("dockDisasm"));
    m_dockDisasm->setWidget(m_disasm);
    m_dockMemory = new QDockWidget(_T("Memory"));
    m_dockMemory->setObjectName(_T("dockMemory"));
    m_dockMemory->setWidget(m_memory);
    m_dockConsole = new QDockWidget(_T("Debug Console"));
    m_dockConsole->setObjectName(_T("dockConsole"));
    m_dockConsole->setWidget(m_console);
    m_dockTeletype = new QDockWidget(_T("Teletype"));
    m_dockTeletype->setObjectName(_T("dockTeletype"));
    m_dockTeletype->setWidget(m_teletype);
    m_dockTape = new QDockWidget(_T("Tape"));
    m_dockTape->setObjectName(_T("dockTape"));
    m_dockTape->setWidget(m_tape);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDebug, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDisasm, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockMemory, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockTape, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockConsole, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockTeletype);

    m_statusLabelInfo = new QLabel();
    m_statusLabelFrames = new QLabel();
    m_statusLabelUptime = new QLabel();
    statusBar()->addWidget(m_statusLabelInfo, 600);
    statusBar()->addPermanentWidget(m_statusLabelFrames, 150);
    statusBar()->addPermanentWidget(m_statusLabelUptime, 150);

    this->setFocusProxy(m_screen);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_screen;
    delete m_keyboard;
    delete m_console;
    delete m_debug;
    delete m_disasm;
    delete m_memory;
    delete m_teletype;
    delete m_dockConsole;
    delete m_dockDebug;
    delete m_dockDisasm;
    delete m_dockMemory;
    delete m_dockTeletype;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    Global_getSettings()->setValue("MainWindow/Geometry", saveGeometry());
    Global_getSettings()->setValue("MainWindow/WindowState", saveState());

    Global_getSettings()->setValue("MainWindow/ConsoleView", m_dockConsole->isVisible());
    Global_getSettings()->setValue("MainWindow/DebugView", m_dockDebug->isVisible());
    Global_getSettings()->setValue("MainWindow/DisasmView", m_dockDisasm->isVisible());
    Global_getSettings()->setValue("MainWindow/MemoryView", m_dockMemory->isVisible());
    Global_getSettings()->setValue("MainWindow/TeletypeView", m_dockTeletype->isVisible());
}

void MainWindow::restoreSettings()
{
    restoreGeometry(Global_getSettings()->value("MainWindow/Geometry").toByteArray());
    restoreState(Global_getSettings()->value("MainWindow/WindowState").toByteArray());

    m_dockConsole->setVisible(Global_getSettings()->value("MainWindow/ConsoleView", false).toBool());
    m_dockDebug->setVisible(Global_getSettings()->value("MainWindow/DebugView", false).toBool());
    m_dockDisasm->setVisible(Global_getSettings()->value("MainWindow/DisasmView", false).toBool());
    m_dockMemory->setVisible(Global_getSettings()->value("MainWindow/MemoryView", false).toBool());
    m_dockTeletype->setVisible(Global_getSettings()->value("MainWindow/TeletypeView", false).toBool());
}

void MainWindow::UpdateMenu()
{
    ui->actionEmulatorRun->setChecked(g_okEmulatorRunning);

    ui->actionEmulatorScreen0->setChecked(m_screen->mode() == 0);
    ui->actionEmulatorScreen1->setChecked(m_screen->mode() == 1);
    ui->actionEmulatorScreen2->setChecked(m_screen->mode() == 2);
    ui->actionEmulatorScreen3->setChecked(m_screen->mode() == 3);
    ui->actionEmulatorColorScreen->setIcon(QIcon(
            (m_screen->mode() & 1) ? _T(":/images/iconScreenColor.png") : _T(":/images/iconScreenBW.png") ));

    ui->actionConfBK10Basic->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_BASIC);
    ui->actionConfBK10Focal->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_FOCAL);
    ui->actionConfBK10Fdd->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_FDD);
    ui->actionConfBK11->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0011);
    ui->actionConfBK11Fdd->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0011_FDD);

    ui->actionDrivesFloppy0->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(0) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
    ui->actionDrivesFloppy1->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(1) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
    ui->actionDrivesFloppy2->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(2) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));
    ui->actionDrivesFloppy3->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(3) ? _T(":/images/iconFloppy.png") : _T(":/images/iconFloppySlot.png") ));

    ui->actionDebugConsoleView->setChecked(m_console->isVisible());
    ui->actionDebugDebugView->setChecked(m_dockDebug->isVisible());
    ui->actionDebugDisasmView->setChecked(m_dockDisasm->isVisible());
    ui->actionDebugMemoryView->setChecked(m_dockMemory->isVisible());
    ui->actionDebugTeletypeView->setChecked(m_dockTeletype->isVisible());
}

void MainWindow::UpdateAllViews()
{
    Emulator_OnUpdate();

    if (m_debug != NULL)
        m_debug->updateData();
    if (m_disasm != NULL)
        m_disasm->updateData();
    if (m_memory != NULL)
        m_memory->updateData();

    m_screen->repaint();
    if (m_debug != NULL)
        m_debug->repaint();
    if (m_disasm != NULL)
        m_disasm->repaint();
    if (m_memory != NULL)
        m_memory->repaint();

    UpdateMenu();
}

void MainWindow::showUptime(int uptimeMillisec)
{
    int seconds = (int) (uptimeMillisec % 60);
    int minutes = (int) (uptimeMillisec / 60 % 60);
    int hours   = (int) (uptimeMillisec / 3600 % 60);

    TCHAR buffer[20];
    _sntprintf(buffer, 20, _T("Uptime: %02d:%02d:%02d"), hours, minutes, seconds);

    m_statusLabelUptime->setText(buffer);
}
void MainWindow::showFps(double framesPerSecond)
{
    if (framesPerSecond <= 0)
    {
        m_statusLabelFrames->setText(_T(""));
    }
    else
    {
        TCHAR buffer[16];
        _sntprintf(buffer, 16, _T("FPS: %05.2f"), framesPerSecond);
        m_statusLabelFrames->setText(buffer);
    }
}

void MainWindow::fileLoadBin()
{
    // Open file dialog
    QFileDialog dlg;
    dlg.setNameFilter(_T("BK binary images (*.bin)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    LPCTSTR sFileName = qPrintable(strFileName);

    // Load BIN header
    FILE* fpBin = ::_tfopen(sFileName, _T("rb"));
    if (fpBin == NULL)
    {
        AlertWarning(_T("Failed to open file."));
        return;
    }
    unsigned char bufHeader[4];
    size_t dwBytesRead = ::fread(bufHeader, 1, 4, fpBin);
    if (dwBytesRead != 4)
    {
        ::fclose(fpBin);
        AlertWarning(_T("Failed to read file."));
        return;
    }

    // Calculate baseAddress / dataSize
    WORD baseAddress = *((WORD*)bufHeader);
    WORD dataSize = *(((WORD*)bufHeader) + 1);

    // Ask user
    TCHAR bufMessage[100];
    _sntprintf(bufMessage, 100,
        _T("Loading BIN image from file.\n\nBase address: %06o\nData size: %06o\n\nProceed?"),
        baseAddress, dataSize);
    if (!AlertOkCancel(bufMessage))
    {
        ::fclose(fpBin);
        return;
    }

    // Load file data
    DWORD bytesToRead = dataSize;
    WORD memoryBytes = (dataSize + 1) & 0xfffe;
    void* pBuffer = ::malloc(memoryBytes);
    dwBytesRead = ::fread(pBuffer, 1, bytesToRead, fpBin);
    if (dwBytesRead != bytesToRead)
    {
        ::fclose(fpBin);
        AlertWarning(_T("Failed to read file."));
        return;
    }

    ::fclose(fpBin);

    // Copy data to BK memory
    WORD address = baseAddress;
    WORD* pData = (WORD*)pBuffer;
    while (address < baseAddress + memoryBytes)
    {
        WORD value = *pData++;
        g_pBoard->SetRAMWord(address, value);
        address += 2;
    }

    ::free(pBuffer);
}

void MainWindow::fileScreenshot()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(_T("PNG images (*.png)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    m_screen->saveScreenshot(strFileName);
}

void MainWindow::helpAbout()
{
    QMessageBox::about(this, _T("About"), _T(
        "QtBkBtl Version 1.0\n"
        "Copyright (C) 2009-2010\n\n"
        "http://code.google.com/p/bkbtl/\n\n"
        "Author:\n"
        "Nikita Zimin (nzeemin@gmail.com)\n\n"
        "Special thanks to:\n"
        "Alexey Kisly\n\n"
        "Build date:\t" __DATE__ __TIME__));
}
void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, _T("About Qt"));
}

void MainWindow::emulatorFrame()
{
    if (!g_okEmulatorRunning)
        return;

    if (Emulator_IsBreakpoint())
        Emulator_Stop();
    else if (Emulator_SystemFrame())
    {
        m_screen->repaint();
    }
}

void MainWindow::emulatorRun()
{
    if (g_okEmulatorRunning)
    {
        this->setWindowTitle(_T("BK Back to Life"));
        Emulator_Stop();
    }
    else
    {
        this->setWindowTitle(_T("BK Back to Life [run]"));
        Emulator_Start();
    }
}

void MainWindow::emulatorReset()
{
    Emulator_Reset();

    m_screen->repaint();
}

void MainWindow::emulatorColorScreen()
{
    int newMode = m_screen->mode() ^ 1;

    m_screen->setMode(newMode);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
}

void MainWindow::emulatorScreen0()
{
    m_screen->setMode(0);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
}
void MainWindow::emulatorScreen1()
{
    m_screen->setMode(1);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
}
void MainWindow::emulatorScreen2()
{
    m_screen->setMode(2);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
}
void MainWindow::emulatorScreen3()
{
    m_screen->setMode(3);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
}

void MainWindow::configurationBK0010Basic() { setConfiguration(BK_CONF_BK0010_BASIC); }
void MainWindow::configurationBK0010Focal() { setConfiguration(BK_CONF_BK0010_FOCAL); }
void MainWindow::configurationBK0010Fdd()   { setConfiguration(BK_CONF_BK0010_FDD); }
void MainWindow::configurationBK0011()      { setConfiguration(BK_CONF_BK0011); }
void MainWindow::configurationBK0011Fdd()   { setConfiguration(BK_CONF_BK0011_FDD); }
void MainWindow::setConfiguration(int configuration)
{
    // Check if configuration changed
    if (g_nEmulatorConfiguration == configuration)
        return;

    // Ask user -- we have to reset machine to change configuration
    if (!AlertOkCancel(_T("Reset required after configuration change.\nAre you agree?")))
        return;

    // Change configuration
    Emulator_InitConfiguration((BKConfiguration)configuration);

    //Settings_SetConfiguration(configuration);

    UpdateMenu();
    Global_UpdateAllViews();
}

void MainWindow::emulatorFloppy0() { emulatorFloppy(0); }
void MainWindow::emulatorFloppy1() { emulatorFloppy(1); }
void MainWindow::emulatorFloppy2() { emulatorFloppy(2); }
void MainWindow::emulatorFloppy3() { emulatorFloppy(3); }
void MainWindow::emulatorFloppy(int slot)
{
    if (g_pBoard->IsFloppyImageAttached(slot))
    {
        g_pBoard->DetachFloppyImage(slot);
        //Settings_SetFloppyFilePath(slot, NULL);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(_T("BK floppy images (*.img *.bkd)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);
        LPCTSTR sFileName = qPrintable(strFileName);

        if (! g_pBoard->AttachFloppyImage(slot, sFileName))
        {
            AlertWarning(_T("Failed to attach floppy image."));
            return;
        }

        //Settings_SetFloppyFilePath(slot, sFileName);
    }

    UpdateMenu();
}

void MainWindow::debugConsoleView()
{
    BOOL okShow = !m_dockConsole->isVisible();
    m_dockConsole->setVisible(okShow);
    m_dockDebug->setVisible(okShow);
    m_dockDisasm->setVisible(okShow);
    m_dockMemory->setVisible(okShow);

    if (!okShow)
    {
        this->adjustSize();
    }

    UpdateMenu();
}
void MainWindow::debugDebugView()
{
    m_dockDebug->setVisible(!m_dockDebug->isVisible());
    UpdateMenu();
}
void MainWindow::debugDisasmView()
{
    m_dockDisasm->setVisible(!m_dockDisasm->isVisible());
    UpdateMenu();
}
void MainWindow::debugMemoryView()
{
    m_dockMemory->setVisible(!m_dockMemory->isVisible());
    UpdateMenu();
}
void MainWindow::debugTeletypeView()
{
    m_dockTeletype->setVisible(!m_dockTeletype->isVisible());
    UpdateMenu();
}

void MainWindow::debugStepInto()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand(_T("s"));
}
void MainWindow::debugStepOver()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand(_T("so"));
}

void MainWindow::printToTeletype(const QString & message)
{
    if (m_teletype != NULL)
    {
        m_teletype->print(message);
    }
}

void MainWindow::scriptRun()
{
    if (g_okEmulatorRunning)
        emulatorRun();  // Stop the emulator

    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilter(_T("Script files (*.js)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    QFile file(strFileName);
    file.open(QIODevice::ReadOnly);
    QString strScript = file.readAll();

    QScriptWindow window(this);
    window.runScript(strScript);
}
