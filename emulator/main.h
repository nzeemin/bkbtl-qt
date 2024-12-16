#ifndef MAIN_H
#define MAIN_H

#include <QtGlobal>
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
void Global_RedrawDebugView();
void Global_RedrawDisasmView();
void Global_showUptime(int uptimeMillisec);
void Global_showFps(double framesPerSecond);


//////////////////////////////////////////////////////////////////////
// Settings

void Settings_SetConfiguration(int configuration);
int  Settings_GetConfiguration();
void Settings_SetFloppyFilePath(int slot, const QString &sFilePath);
QString Settings_GetFloppyFilePath(int slot);
void Settings_SetAutostart(bool flag);
bool Settings_GetAutostart();
void Settings_SetSound(bool flag);
bool Settings_GetSound();
void Settings_SetDebugBreakpoint(int bpno, quint16 address);
quint16 Settings_GetDebugBreakpoint(int bpno);
void Settings_SetDebugMemoryAddress(quint16 address);
quint16 Settings_GetDebugMemoryAddress();
bool Settings_GetDebugMemoryByte();
void Settings_SetDebugMemoryByte(bool flag);
void Settings_SetDebugMemoryNumeral(quint16 mode);
quint16 Settings_GetDebugMemoryNumeral();


//////////////////////////////////////////////////////////////////////
// Options

extern bool Option_ShowHelp;


//////////////////////////////////////////////////////////////////////
#endif // MAIN_H
