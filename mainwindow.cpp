#include "stdafx.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QVBoxLayout>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscreen.h"
#include "qkeyboardview.h"
#include "qdebugview.h"
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
    QObject::connect(ui->actionConfBK10Basic, SIGNAL(triggered()), this, SLOT(configurationBK0010Basic()));
    QObject::connect(ui->actionConfBK10Focal, SIGNAL(triggered()), this, SLOT(configurationBK0010Focal()));
    QObject::connect(ui->actionConfBK10Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0010Fdd()));
    QObject::connect(ui->actionConfBK11, SIGNAL(triggered()), this, SLOT(configurationBK0011()));
    QObject::connect(ui->actionConfBK11Fdd, SIGNAL(triggered()), this, SLOT(configurationBK0011Fdd()));
    //QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), this, SLOT(emulatorFloppy0()));
    //QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), this, SLOT(emulatorFloppy1()));
    //QObject::connect(ui->actionDrivesFloppy2, SIGNAL(triggered()), this, SLOT(emulatorFloppy2()));
    //QObject::connect(ui->actionDrivesFloppy3, SIGNAL(triggered()), this, SLOT(emulatorFloppy3()));
    QObject::connect(ui->actionHelpAboutQt, SIGNAL(triggered()), this, SLOT(helpAboutQt()));

    // Screen and keyboard
    m_screen = new QScreen();
    m_keyboard = new QKeyboardView();
    m_debug = new QDebugView();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_screen);
    layout->addWidget(m_keyboard);
    layout->addWidget(m_debug);
    ui->centralWidget->setLayout(layout);

    this->adjustSize();
    this->setFocusProxy(m_screen);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_screen;
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

void MainWindow::UpdateMenu()
{
    ui->actionEmulatorColorScreen->setIcon(QIcon(
            m_screen->mode() == ColorScreen ? _T(":/images/iconScreenColor.png") : _T(":/images/iconScreenBW.png") ));

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

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, _T("About Qt"));
}

void MainWindow::emulatorFrame()
{
    if (g_okEmulatorRunning)
    {
        if (Emulator_SystemFrame())
        {
            m_screen->repaint();
        }
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
    ScreenViewMode newMode = (m_screen->mode() == ColorScreen) ? BlackWhiteScreen : ColorScreen;
    m_screen->setMode(newMode);

    UpdateMenu();
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
    //MainWindow_UpdateAllViews();
}

void MainWindow::emulatorFloppy0() { emulatorFloppy(0); }
void MainWindow::emulatorFloppy1() { emulatorFloppy(1); }
void MainWindow::emulatorFloppy2() { emulatorFloppy(2); }
void MainWindow::emulatorFloppy3() { emulatorFloppy(3); }
void MainWindow::emulatorFloppy(int slot)
{
    //if (g_pBoard->IsFloppyImageAttached(slot))
    //{
    //    g_pBoard->DetachFloppyImage(slot);
    //    Settings_SetFloppyFilePath(slot, NULL);
    //}
    //else
    //{
    //    QFileDialog dlg;
    //    dlg.setNameFilter(_T("BK floppy images (*.dsk *.rtd)"));
    //    if (dlg.exec() == QDialog::Rejected)
    //        return;

    //    QString strFileName = dlg.selectedFiles().at(0);
    //    LPCTSTR sFileName = qPrintable(strFileName);

    //    if (! g_pBoard->AttachFloppyImage(slot, sFileName))
    //    {
    //        AlertWarning(_T("Failed to attach floppy image."));
    //        return;
    //    }

    //    Settings_SetFloppyFilePath(slot, sFileName);
    //}

    //UpdateMenu();
}
