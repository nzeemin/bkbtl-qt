#include "stdafx.h"
#include <QApplication>
#include "qemulator.h"
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"

void QEmulator::reset()
{
    Emulator_Reset();

    Global_getMainWindow()->UpdateAllViews();
}

bool QEmulator::run(int frames)
{
    bool result = true;
    for (int i = 0; i < frames; i++)
    {
        int res = Emulator_SystemFrame();
        if (!res)
        {
            result = false;
            break;
        }

        if (i % 25 == 24)  // Update the screen every 25 frames
        {
            Global_getMainWindow()->UpdateAllViews();
            Global_getApplication()->processEvents();
        }
    }

    Global_getMainWindow()->UpdateAllViews();

    return result;
}
