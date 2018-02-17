#include "stdafx.h"
#include <QApplication>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QSettings>
#include <QDesktopWidget>
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"


void RestoreSettings();


QApplication *g_Application;
MainWindow *g_MainWindow;
QSettings *g_Settings;


int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    g_Application = &application;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Back to Life", "BKBTL");
    g_Settings = &settings;

    MainWindow w;
    g_MainWindow = &w;

    if (!Emulator_Init())
        return 255;
    if (!Emulator_InitConfiguration((BKConfiguration)Global_getSettings()->value("Configuration", BK_CONF_BK0010_BASIC).toInt()))
        return 255;

    w.restoreSettings(Global_getSettings());

    w.show();

    RestoreSettings();
    w.UpdateMenu();
    w.UpdateAllViews();

    QTimer timerFrame;
    QObject::connect(&timerFrame, SIGNAL(timeout()), &w, SLOT(emulatorFrame()), Qt::AutoConnection);
    timerFrame.start(32);

    int result = application.exec();

    Emulator_Done();

    Global_getSettings()->setValue("Configuration", (int)g_nEmulatorConfiguration);
    settings.sync();

    Common_Cleanup();

    return result;
}

QApplication* Global_getApplication()
{
    return g_Application;
}
MainWindow* Global_getMainWindow()
{
    return g_MainWindow;
}
QSettings* Global_getSettings()
{
    return g_Settings;
}

void Global_UpdateAllViews()
{
    Global_getMainWindow()->UpdateAllViews();
}
void Global_UpdateMenu()
{
    Global_getMainWindow()->UpdateMenu();
}

void Global_showUptime(int uptimeMillisec)
{
    Global_getMainWindow()->showUptime(uptimeMillisec);
}
void Global_showFps(double framesPerSecond)
{
    Global_getMainWindow()->showFps(framesPerSecond);
}

void RestoreSettings()
{
    // Reattach floppy images
    for (int slot = 0; slot < 4; slot++)
    {
        //QString path = Settings_GetFloppyFilePath(slot);
        //if (path.length() > 0)
        //{
        //    if (! g_pBoard->AttachFloppyImage(slot, qPrintable(path)))
        //        Settings_SetFloppyFilePath(slot, NULL);
        //}
    }
}
