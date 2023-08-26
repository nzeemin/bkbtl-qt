#include "stdafx.h"
#include <QSettings>
#include "main.h"


//////////////////////////////////////////////////////////////////////
// Options

bool Option_ShowHelp = false;


//////////////////////////////////////////////////////////////////////


void Settings_SetConfiguration(int configuration)
{
    Global_getSettings()->setValue("Configuration", configuration);
}
int Settings_GetConfiguration()
{
    QVariant value = Global_getSettings()->value("Configuration", 0);
    return value.toInt();
}

void Settings_SetFloppyFilePath(int slot, const QString & sFilePath)
{
    char bufValueName[] = "Floppy0";
    bufValueName[6] = slot + '0';
    Global_getSettings()->setValue(bufValueName, sFilePath);
}
QString Settings_GetFloppyFilePath(int slot)
{
    char bufValueName[] = "Floppy0";
    bufValueName[6] = slot + '0';
    QVariant value = Global_getSettings()->value(bufValueName, "");
    return value.toString();
}

void Settings_SetAutostart(bool flag)
{
    Global_getSettings()->setValue("Autostart", flag);
}
bool Settings_GetAutostart()
{
    QVariant value = Global_getSettings()->value("Autostart", false);
    return value.toBool();
}

void Settings_SetSound(bool flag)
{
    Global_getSettings()->setValue("Sound", flag);
}

bool Settings_GetSound()
{
    QVariant value = Global_getSettings()->value("Sound", false);
    return value.toBool();
}

void Settings_SetDebugMemoryAddress(quint16 mode)
{
    Global_getSettings()->setValue("DebugMemoryAddress", mode);
}
quint16 Settings_GetDebugMemoryAddress()
{
    QVariant value = Global_getSettings()->value("DebugMemoryAddress", 0);
    return (quint16)value.toUInt();
}

void Settings_SetDebugMemoryByte(bool flag)
{
    Global_getSettings()->setValue("DebugMemoryByte", flag);
}
bool Settings_GetDebugMemoryByte()
{
    QVariant value = Global_getSettings()->value("DebugMemoryByte", false);
    return value.toBool();
}


//////////////////////////////////////////////////////////////////////
