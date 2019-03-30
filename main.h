#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include "Common.h"


//////////////////////////////////////////////////////////////////////

class QApplication;
class MainWindow;
class QSettings;


QApplication* Global_getApplication();
MainWindow* Global_getMainWindow();
QSettings* Global_getSettings();
void Global_UpdateAllViews();
void Global_UpdateMenu();
void Global_showUptime(int uptimeMillisec);
void Global_showFps(double framesPerSecond);


//////////////////////////////////////////////////////////////////////
// Settings

void Settings_SetConfiguration(int configuration);
int  Settings_GetConfiguration();
void Settings_SetFloppyFilePath(int slot, const QString &sFilePath);
QString Settings_GetFloppyFilePath(int slot);
void Settings_SetSound(bool flag);
bool Settings_GetSound();


//////////////////////////////////////////////////////////////////////
#endif // MAIN_H
