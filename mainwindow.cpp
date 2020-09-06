#include "stdafx.h"
#include <QAction>
#include <QClipboard>
#include <QDateTime>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>
#include <QVBoxLayout>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdialogs.h"
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

    this->setWindowTitle("BK Back to Life");

    // Assign signals
    QObject::connect(ui->actionSaveStateImage, SIGNAL(triggered()), this, SLOT(saveStateImage()));
    QObject::connect(ui->actionLoadStateImage, SIGNAL(triggered()), this, SLOT(loadStateImage()));
    QObject::connect(ui->actionFileLoadBin, SIGNAL(triggered()), this, SLOT(fileLoadBin()));
    QObject::connect(ui->actionFileScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
    QObject::connect(ui->actionFileScreenshotAs, SIGNAL(triggered()), this, SLOT(saveScreenshotAs()));
    QObject::connect(ui->actionFileScreenshotToClipboard, SIGNAL(triggered()), this, SLOT(screenshotToClipboard()));
    QObject::connect(ui->actionScriptRun, SIGNAL(triggered()), this, SLOT(scriptRun()));
    QObject::connect(ui->actionFileExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionEmulatorRun, SIGNAL(triggered()), this, SLOT(emulatorRun()));
    QObject::connect(ui->actionEmulatorReset, SIGNAL(triggered()), this, SLOT(emulatorReset()));
    QObject::connect(ui->actionEmulatorColorScreen, SIGNAL(triggered()), this, SLOT(emulatorColorScreen()));
    QObject::connect(ui->actionConfBK10Basic, SIGNAL(triggered()), this, SLOT(configurationBK0010Basic()));
    QObject::connect(ui->actionConfBK10Focal, SIGNAL(triggered()), this, SLOT(configurationBK0010Focal()));
    QObject::connect(ui->actionConfBK10Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0010Fdd()));
    QObject::connect(ui->actionConfBK11, SIGNAL(triggered()), this, SLOT(configurationBK0011()));
    QObject::connect(ui->actionConfBK11Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0011Fdd()));
    QObject::connect(ui->actionDebugConsoleView, SIGNAL(triggered()), this, SLOT(debugConsoleView()));
    QObject::connect(ui->actionDebugDebugView, SIGNAL(triggered()), this, SLOT(debugDebugView()));
    QObject::connect(ui->actionDebugDisasmView, SIGNAL(triggered()), this, SLOT(debugDisasmView()));
    QObject::connect(ui->actionDebugMemoryView, SIGNAL(triggered()), this, SLOT(debugMemoryView()));
    QObject::connect(ui->actionDebugTeletypeView, SIGNAL(triggered()), this, SLOT(debugTeletypeView()));
    QObject::connect(ui->actionDebugStepInto, SIGNAL(triggered()), this, SLOT(debugStepInto()));
    QObject::connect(ui->actionDebugStepOver, SIGNAL(triggered()), this, SLOT(debugStepOver()));
    QObject::connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    QObject::connect(ui->actionViewKeyboard, SIGNAL(triggered()), this, SLOT(viewKeyboard()));
    QObject::connect(ui->actionSoundEnabled, SIGNAL(triggered()), this, SLOT(soundEnabled()));

    QSignalMapper* mapScreenMode = new QSignalMapper(this);
    mapScreenMode->setMapping(ui->actionEmulatorScreen0, 0);
    mapScreenMode->setMapping(ui->actionEmulatorScreen1, 1);
    mapScreenMode->setMapping(ui->actionEmulatorScreen2, 2);
    mapScreenMode->setMapping(ui->actionEmulatorScreen3, 3);
    mapScreenMode->setMapping(ui->actionEmulatorScreen4, 4);
    mapScreenMode->setMapping(ui->actionEmulatorScreen5, 5);
    mapScreenMode->setMapping(ui->actionEmulatorScreen6, 6);
    mapScreenMode->setMapping(ui->actionEmulatorScreen7, 7);
    QObject::connect(mapScreenMode, SIGNAL(mapped(int)), this, SLOT(emulatorScreenMode(int)));
    QObject::connect(ui->actionEmulatorScreen0, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen1, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen2, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen3, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen4, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen5, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen6, SIGNAL(triggered()), mapScreenMode, SLOT(map()));
    QObject::connect(ui->actionEmulatorScreen7, SIGNAL(triggered()), mapScreenMode, SLOT(map()));

    QSignalMapper* mapFloppy = new QSignalMapper(this);
    mapFloppy->setMapping(ui->actionDrivesFloppy0, 0);
    mapFloppy->setMapping(ui->actionDrivesFloppy1, 1);
    mapFloppy->setMapping(ui->actionDrivesFloppy2, 2);
    mapFloppy->setMapping(ui->actionDrivesFloppy3, 3);
    QObject::connect(mapFloppy, SIGNAL(mapped(int)), this, SLOT(emulatorFloppy(int)));
    QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), mapFloppy, SLOT(map()));
    QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), mapFloppy, SLOT(map()));
    QObject::connect(ui->actionDrivesFloppy2, SIGNAL(triggered()), mapFloppy, SLOT(map()));
    QObject::connect(ui->actionDrivesFloppy3, SIGNAL(triggered()), mapFloppy, SLOT(map()));

    // Views
    m_screen = new QEmulatorScreen();
    m_keyboard = new QKeyboardView();
    m_console = new QConsoleView();
    m_debug = new QDebugView(this);
    m_disasm = new QDisasmView();
    m_memory = new QMemoryView();
    m_teletype = new QTeletypeView();
    m_tape = new QTapeView();

    // Prepare the layout
    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(0);
    vboxlayout->addWidget(m_screen);
    vboxlayout->addWidget(m_keyboard);
    ui->centralWidget->setLayout(vboxlayout);
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    int maxwid = m_screen->maximumWidth() > m_keyboard->maximumWidth() ? m_screen->maximumWidth() : m_keyboard->maximumWidth();
    ui->centralWidget->setMaximumWidth(maxwid);

    m_dockDebug = new QDockWidget("Processor");
    m_dockDebug->setObjectName("dockDebug");
    m_dockDebug->setWidget(m_debug);
    m_dockDisasm = new QDockWidget("Disassemble");
    m_dockDisasm->setObjectName("dockDisasm");
    m_dockDisasm->setWidget(m_disasm);
    m_dockMemory = new QDockWidget("Memory");
    m_dockMemory->setObjectName("dockMemory");
    m_dockMemory->setWidget(m_memory);
    m_dockConsole = new QDockWidget("Debug Console");
    m_dockConsole->setObjectName("dockConsole");
    m_dockConsole->setWidget(m_console);
    m_dockTeletype = new QDockWidget("Teletype");
    m_dockTeletype->setObjectName("dockTeletype");
    m_dockTeletype->setWidget(m_teletype);
    m_dockTape = new QDockWidget("Tape");
    m_dockTape->setObjectName("dockTape");
    m_dockTape->setWidget(m_tape);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDebug, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDisasm, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockMemory, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockTape, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockConsole, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockTeletype);

    m_statusLabelInfo = new QLabel(this);
    m_statusLabelFrames = new QLabel(this);
    m_statusLabelUptime = new QLabel(this);
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
    delete m_dockTape;
    delete m_statusLabelInfo;
    delete m_statusLabelFrames;
    delete m_statusLabelUptime;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    saveSettings(Global_getSettings());
}

void MainWindow::saveSettings(QSettings * settings)
{
    Global_getSettings()->setValue("MainWindow/ScreenMode", m_screen->mode());

    settings->setValue("MainWindow/Geometry", saveGeometry());
    settings->setValue("MainWindow/WindowState", saveState());

    settings->setValue("MainWindow/ConsoleView", m_dockConsole->isVisible());
    settings->setValue("MainWindow/DebugView", m_dockDebug->isVisible());
    settings->setValue("MainWindow/DisasmView", m_dockDisasm->isVisible());
    settings->setValue("MainWindow/MemoryView", m_dockMemory->isVisible());
    settings->setValue("MainWindow/TeletypeView", m_dockTeletype->isVisible());
    settings->setValue("MainWindow/TapeView", m_dockTape->isVisible());
}
void MainWindow::restoreSettings(QSettings * settings)
{
    int scrViewMode = Global_getSettings()->value("MainWindow/ScreenMode").toInt();
    m_screen->setMode(scrViewMode);

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());

    restoreGeometry(settings->value("MainWindow/Geometry").toByteArray());
    restoreState(settings->value("MainWindow/WindowState").toByteArray());

    m_dockConsole->setVisible(settings->value("MainWindow/ConsoleView", false).toBool());
    m_dockDebug->setVisible(settings->value("MainWindow/DebugView", false).toBool());
    m_dockDisasm->setVisible(settings->value("MainWindow/DisasmView", false).toBool());
    m_dockMemory->setVisible(settings->value("MainWindow/MemoryView", false).toBool());
    m_dockTeletype->setVisible(settings->value("MainWindow/TeletypeView", false).toBool());
    m_dockTape->setVisible(settings->value("MainWindow/TapeView", false).toBool());

    ui->actionSoundEnabled->setChecked(Settings_GetSound());
}

void MainWindow::updateMenu()
{
    ui->actionEmulatorRun->setChecked(g_okEmulatorRunning);

    ui->actionViewKeyboard->setChecked(m_keyboard->isVisible());

    ui->actionEmulatorScreen0->setChecked(m_screen->mode() == 0);
    ui->actionEmulatorScreen1->setChecked(m_screen->mode() == 1);
    ui->actionEmulatorScreen2->setChecked(m_screen->mode() == 2);
    ui->actionEmulatorScreen3->setChecked(m_screen->mode() == 3);
    ui->actionEmulatorScreen4->setChecked(m_screen->mode() == 4);
    ui->actionEmulatorScreen5->setChecked(m_screen->mode() == 5);
    ui->actionEmulatorScreen6->setChecked(m_screen->mode() == 6);
    ui->actionEmulatorScreen7->setChecked(m_screen->mode() == 7);
    ui->actionEmulatorColorScreen->setIcon(QIcon(
            (m_screen->mode() & 1) ? ":/images/iconScreenColor.png" : ":/images/iconScreenBW.png" ));

    ui->actionConfBK10Basic->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_BASIC);
    ui->actionConfBK10Focal->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_FOCAL);
    ui->actionConfBK10Fdd->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0010_FDD);
    ui->actionConfBK11->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0011);
    ui->actionConfBK11Fdd->setChecked(g_nEmulatorConfiguration == BK_CONF_BK0011_FDD);

    ui->actionDrivesFloppy0->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(0) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));
    ui->actionDrivesFloppy1->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(1) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));
    ui->actionDrivesFloppy2->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(2) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));
    ui->actionDrivesFloppy3->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(3) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));

    ui->actionDebugConsoleView->setChecked(m_console->isVisible());
    ui->actionDebugDebugView->setChecked(m_dockDebug->isVisible());
    ui->actionDebugDisasmView->setChecked(m_dockDisasm->isVisible());
    ui->actionDebugMemoryView->setChecked(m_dockMemory->isVisible());
    ui->actionDebugTeletypeView->setChecked(m_dockTeletype->isVisible());
}

void MainWindow::updateAllViews()
{
    Emulator_OnUpdate();

    if (m_debug != nullptr)
        m_debug->updateData();
    if (m_disasm != nullptr)
        m_disasm->updateData();
    if (m_memory != nullptr)
        m_memory->updateData();
    if (m_console != nullptr)
        m_console->updatePrompt();

    m_screen->repaint();
    if (m_debug != nullptr)
        m_debug->repaint();
    if (m_disasm != nullptr)
        m_disasm->repaint();
    if (m_memory != nullptr)
        m_memory->repaint();

    updateMenu();
}

void MainWindow::redrawDisasmView()
{
    if (m_disasm != nullptr)
        m_disasm->repaint();
}

void MainWindow::updateWindowText()
{
    if (g_okEmulatorRunning)
        this->setWindowTitle(tr("UKNC Back to Life [run]"));
    else
        this->setWindowTitle(tr("UKNC Back to Life [stop]"));
}

void MainWindow::showUptime(int uptimeMillisec)
{
    int seconds = (int) (uptimeMillisec % 60);
    int minutes = (int) (uptimeMillisec / 60 % 60);
    int hours   = (int) (uptimeMillisec / 3600 % 60);

    char buffer[12];
    _snprintf(buffer, 20, "%02d:%02d:%02d", hours, minutes, seconds);
    m_statusLabelUptime->setText(tr("Uptime: %1").arg(buffer));
}
void MainWindow::showFps(double framesPerSecond)
{
    if (framesPerSecond <= 0)
    {
        m_statusLabelFrames->setText("");
    }
    else
    {
        double speed = framesPerSecond / 25.0 * 100.0;
        char buffer[16];
        _snprintf(buffer, 16, "%03.f%%", speed);
        m_statusLabelFrames->setText(buffer);
    }
}

void MainWindow::fileLoadBin()
{
    // Open file dialog
    QFileDialog dlg;
    dlg.setNameFilter("BK binary images (*.bin)");
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    const char * sFileName = qPrintable(strFileName);

    // Load BIN header
    FILE* fpBin = ::fopen(sFileName, "rb");
    if (fpBin == nullptr)
    {
        AlertWarning("Failed to open file.");
        return;
    }
    unsigned char bufHeader[4];
    size_t dwBytesRead = ::fread(bufHeader, 1, 4, fpBin);
    if (dwBytesRead != 4)
    {
        ::fclose(fpBin);
        AlertWarning("Failed to read file.");
        return;
    }

    // Calculate baseAddress / dataSize
    quint16 baseAddress = *((quint16*)bufHeader);
    quint16 dataSize = *(((quint16*)bufHeader) + 1);

    // Ask user
    char bufMessage[100];
    _snprintf(bufMessage, 100,
            "Loading BIN image from file.\n\nBase address: %06o\nData size: %06o\n\nProceed?",
            baseAddress, dataSize);
    if (!AlertOkCancel(bufMessage))
    {
        ::fclose(fpBin);
        return;
    }

    // Load file data
    quint32 bytesToRead = dataSize;
    quint16 memoryBytes = (dataSize + 1) & 0xfffe;
    void* pBuffer = ::malloc(memoryBytes);
    dwBytesRead = ::fread(pBuffer, 1, bytesToRead, fpBin);
    if (dwBytesRead != bytesToRead)
    {
        ::fclose(fpBin);
        AlertWarning("Failed to read file.");
        return;
    }

    ::fclose(fpBin);

    // Copy data to BK memory
    quint16 address = baseAddress;
    quint16* pData = (quint16*)pBuffer;
    while (address < baseAddress + memoryBytes)
    {
        quint16 value = *pData++;
        g_pBoard->SetRAMWord(address, value);
        address += 2;
    }

    ::free(pBuffer);
}

void MainWindow::saveStateImage()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("BK state images (*.bkst)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveStateImage(strFileName);
}
void MainWindow::saveStateImage(const QString& strFileName)
{
    const char * sFileName = qPrintable(strFileName);
    Emulator_SaveImage(sFileName);
}
void MainWindow::loadStateImage()
{
    QFileDialog dlg;
    dlg.setNameFilter(tr("BK state images (*.bkst)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    loadStateImage(strFileName);
}
void MainWindow::loadStateImage(const QString& strFileName)
{
    const char * sFileName = qPrintable(strFileName);
    Emulator_LoadImage(sFileName);

    updateAllViews();
}

void MainWindow::saveScreenshot()
{
    QString strFileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshotAs()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter("PNG images (*.png)");
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshot(const QString& strFileName)
{
    QImage image = m_screen->getScreenshot();
    image.save(strFileName, "PNG", -1);
}

void MainWindow::screenshotToClipboard()
{
    QImage image = m_screen->getScreenshot();

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
    clipboard->setImage(image);
}

void MainWindow::helpAbout()
{
    QAboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::viewKeyboard()
{
    m_keyboard->setVisible(!m_keyboard->isVisible());
    updateMenu();
}

void MainWindow::emulatorFrame()
{
    if (!g_okEmulatorRunning)
        return;
    if (!isActiveWindow())
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
        this->setWindowTitle("BK Back to Life");
        Emulator_Stop();
    }
    else
    {
        this->setWindowTitle("BK Back to Life [run]");
        Emulator_Start();
    }
}

void MainWindow::emulatorReset()
{
    Emulator_Reset();

    m_screen->repaint();
}

void MainWindow::soundEnabled()
{
    bool sound = ui->actionSoundEnabled->isChecked();
    Emulator_SetSound(sound);
    Settings_SetSound(sound);
}

void MainWindow::emulatorColorScreen()
{
    int newMode = m_screen->mode() ^ 1;
    emulatorScreenMode(newMode);
}

void MainWindow::emulatorScreenMode(int mode)
{
    if (mode < 0 || mode > 7) return;

    m_screen->setMode(mode);
    updateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
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
    if (!AlertOkCancel("Reset required after configuration change.\nAre you agree?"))
        return;

    // Change configuration
    Emulator_InitConfiguration((BKConfiguration)configuration);

    Settings_SetConfiguration(configuration);

    updateMenu();
    Global_UpdateAllViews();
}

void MainWindow::emulatorFloppy(int slot)
{
    if (g_pBoard->IsFloppyImageAttached(slot))
    {
        g_pBoard->DetachFloppyImage(slot);
        Settings_SetFloppyFilePath(slot, nullptr);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter("BK floppy images (*.img *.bkd)");
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);
        const char * sFileName = qPrintable(strFileName);

        if (! g_pBoard->AttachFloppyImage(slot, sFileName))
        {
            AlertWarning("Failed to attach floppy image.");
            return;
        }

        Settings_SetFloppyFilePath(slot, sFileName);
    }

    updateMenu();
}

void MainWindow::debugConsoleView()
{
    bool okShow = !m_dockConsole->isVisible();
    m_dockConsole->setVisible(okShow);
    m_dockDebug->setVisible(okShow);
    m_dockDisasm->setVisible(okShow);
    m_dockMemory->setVisible(okShow);

    if (!okShow)
    {
        this->adjustSize();
    }

    updateMenu();
}
void MainWindow::debugDebugView()
{
    m_dockDebug->setVisible(!m_dockDebug->isVisible());
    updateMenu();
}
void MainWindow::debugDisasmView()
{
    m_dockDisasm->setVisible(!m_dockDisasm->isVisible());
    updateMenu();
}
void MainWindow::debugMemoryView()
{
    m_dockMemory->setVisible(!m_dockMemory->isVisible());
    updateMenu();
}
void MainWindow::debugTeletypeView()
{
    m_dockTeletype->setVisible(!m_dockTeletype->isVisible());
    updateMenu();
}

void MainWindow::debugStepInto()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("s");
}
void MainWindow::debugStepOver()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("so");
}

void MainWindow::printToTeletype(const QString & message)
{
    if (m_teletype != nullptr)
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
    dlg.setNameFilter("Script files (*.js)");
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    QFile file(strFileName);
    file.open(QIODevice::ReadOnly);
    QString strScript = file.readAll();

    QScriptWindow window(this);
    window.runScript(strScript);
}

void MainWindow::consolePrint(const QString &message)
{
    m_console->printLine(message);
}
