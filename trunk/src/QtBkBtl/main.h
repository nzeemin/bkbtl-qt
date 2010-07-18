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

enum ScreenViewMode {
    ColorScreen = 1,
    BlackWhiteScreen = 2,
};


//////////////////////////////////////////////////////////////////////
// Settings

void Settings_SetFloppyFilePath(int slot, LPCTSTR sFilePath);
QString Settings_GetFloppyFilePath(int slot);
void Settings_SetCartridgeFilePath(int slot, LPCTSTR sFilePath);
QString Settings_GetCartridgeFilePath(int slot);
void Settings_SetHardFilePath(int slot, LPCTSTR sFilePath);
QString Settings_GetHardFilePath(int slot);


//////////////////////////////////////////////////////////////////////
#endif // MAIN_H
