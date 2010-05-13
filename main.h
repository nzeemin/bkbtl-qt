#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include "Common.h"


//////////////////////////////////////////////////////////////////////

class QApplication;
class QMainWindow;
class QSettings;


QApplication* Global_getApplication();
QMainWindow* Global_getMainWindow();
QSettings* Global_getSettings();


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
