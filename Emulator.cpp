/*  This file is part of BKBTL.
    BKBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    BKBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
BKBTL. If not, see <http://www.gnu.org/licenses/>. */

// Emulator.cpp

#include "stdafx.h"
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
#include "qsoundout.h"
#include <QTime>
#include <QFile>


//////////////////////////////////////////////////////////////////////


CMotherboard* g_pBoard = nullptr;
static QSoundOut * g_sound = nullptr;
BKConfiguration g_nEmulatorConfiguration;  // Current configuration

static bool g_okEmulatorInitialized = false;
bool g_okEmulatorRunning = false;

static quint16 m_wEmulatorCPUBreakpoint = 0177777;

static bool m_okEmulatorSound = false;

static long m_nFrameCount = 0;
static QTime m_emulatorTime;
static int m_nTickCount = 0;
static quint32 m_dwEmulatorUptime = 0;  // BK uptime, seconds, from turn on or reset, increments every 25 frames
static long m_nUptimeFrameCount = 0;

quint8* g_pEmulatorRam;  // RAM values - for change tracking
quint8* g_pEmulatorChangedRam;  // RAM change flags
quint16 g_wEmulatorCpuPC = 0177777;      // Current PC value
quint16 g_wEmulatorPrevCpuPC = 0177777;  // Previous PC value

static const int KEYEVENT_QUEUE_SIZE = 32;
static quint16 m_EmulatorKeyQueue[KEYEVENT_QUEUE_SIZE];
static int m_EmulatorKeyQueueTop = 0;
static int m_EmulatorKeyQueueBottom = 0;
static int m_EmulatorKeyQueueCount = 0;

void CALLBACK Emulator_TeletypeCallback(quint8 symbol);

void CALLBACK Emulator_FeedDAC(unsigned short l, unsigned short r);

//Прототип функции преобразования экрана
// Input:
//   pVideoBuffer   Исходные данные, биты экрана БК
//   okSmallScreen  Признак "малого" экрана
//   pPalette       Палитра
//   scroll         Текущее значение скроллинга
//   pImageBits     Результат, 32-битный цвет, размер для каждой функции свой
typedef void (CALLBACK* PREPARE_SCREEN_CALLBACK)(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);

void CALLBACK Emulator_PrepareScreenBW512x256(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenColor512x256(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenBW512x384(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenColor512x384(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenBW896x512(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenColor896x512(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenBW1024x768(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);
void CALLBACK Emulator_PrepareScreenColor1024x768(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits);

struct ScreenModeStruct
{
    int width;
    int height;
    PREPARE_SCREEN_CALLBACK callback;
}
static ScreenModeReference[] =
{
    { 512, 256, Emulator_PrepareScreenBW512x256 },
    { 512, 256, Emulator_PrepareScreenColor512x256 },
    { 512, 384, Emulator_PrepareScreenBW512x384 },
    { 512, 384, Emulator_PrepareScreenColor512x384 },
    { 896, 512, Emulator_PrepareScreenBW896x512 },
    { 896, 512, Emulator_PrepareScreenColor896x512 },
    { 1024, 768, Emulator_PrepareScreenBW1024x768 },
    { 1024, 768, Emulator_PrepareScreenColor1024x768 },
};

//////////////////////////////////////////////////////////////////////


static const char * FILENAME_BKROM_MONIT10    = "monit10.rom";
static const char * FILENAME_BKROM_FOCAL      = "focal.rom";
static const char * FILENAME_BKROM_TESTS      = "tests.rom";
static const char * FILENAME_BKROM_BASIC10_1  = "basic10_1.rom";
static const char * FILENAME_BKROM_BASIC10_2  = "basic10_2.rom";
static const char * FILENAME_BKROM_BASIC10_3  = "basic10_3.rom";
static const char * FILENAME_BKROM_DISK_326   = "disk_326.rom";
static const char * FILENAME_BKROM_BK11M_BOS  = "b11m_bos.rom";
static const char * FILENAME_BKROM_BK11M_EXT  = "b11m_ext.rom";
static const char * FILENAME_BKROM_BASIC11M_0 = "basic11m_0.rom";
static const char * FILENAME_BKROM_BASIC11M_1 = "basic11m_1.rom";
static const char * FILENAME_BKROM_BK11M_MSTD = "b11m_mstd.rom";


//////////////////////////////////////////////////////////////////////
// Colors

const quint32 ScreenView_BWPalette[4] =
{
    0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF
};

const quint32 ScreenView_ColorPalette[4] =
{
    0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000
};

const quint32 ScreenView_ColorPalettes[16][4] =
{
    //                                         Palette#     01           10          11
    { 0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 },  // 00    синий   |   зеленый  |  красный
    { 0xFF000000, 0xFFFFFF00, 0xFFFF00FF, 0xFFFF0000 },  // 01   желтый   |  сиреневый |  красный
    { 0xFF000000, 0xFF00FFFF, 0xFF0000FF, 0xFFFF00FF },  // 02   голубой  |    синий   | сиреневый
    { 0xFF000000, 0xFF00FF00, 0xFF00FFFF, 0xFFFFFF00 },  // 03   зеленый  |   голубой  |  желтый
    { 0xFF000000, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF },  // 04  сиреневый |   голубой  |   белый
    { 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },  // 05    белый   |    белый   |   белый
    { 0xFF000000, 0xFFC00000, 0xFF8E0000, 0xFFFF0000 },  // 06  темн-красн| красн-корич|  красный
    { 0xFF000000, 0xFFC0FF00, 0xFF8EFF00, 0xFFFFFF00 },  // 07  салатовый | светл-зелен|  желтый
    { 0xFF000000, 0xFFC000FF, 0xFF8E00FF, 0xFFFF00FF },  // 08  фиолетовый| фиол-синий | сиреневый
    { 0xFF000000, 0xFF8EFF00, 0xFF8E00FF, 0xFF8E0000 },  // 09 светл-зелен| фиол-синий |красн-корич
    { 0xFF000000, 0xFFC0FF00, 0xFFC000FF, 0xFFC00000 },  // 10  салатовый | фиолетовый |темн-красный
    { 0xFF000000, 0xFF00FFFF, 0xFFFFFF00, 0xFFFF0000 },  // 11   голубой  |   желтый   |  красный
    { 0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFF00FFFF },  // 12   красный  |   зеленый  |  голубой
    { 0xFF000000, 0xFF00FFFF, 0xFFFFFF00, 0xFFFFFFFF },  // 13   голубой  |   желтый   |   белый
    { 0xFF000000, 0xFFFFFF00, 0xFF00FF00, 0xFFFFFFFF },  // 14   желтый   |   зеленый  |   белый
    { 0xFF000000, 0xFF00FFFF, 0xFF00FF00, 0xFFFFFFFF },  // 15   голубой  |   зеленый  |   белый
};


//////////////////////////////////////////////////////////////////////

bool Emulator_LoadRomFile(const char * strFileName, quint8* buffer, quint32 fileOffset, quint32 bytesToRead)
{
    FILE* fpRomFile = ::fopen(strFileName, "rb");
    if (fpRomFile == nullptr)
        return false;

    ASSERT(bytesToRead <= 8192);
    ::memset(buffer, 0, 8192);

    if (fileOffset > 0)
    {
        ::fseek(fpRomFile, fileOffset, SEEK_SET);
    }

    quint32 dwBytesRead = ::fread(buffer, 1, bytesToRead, fpRomFile);
    if (dwBytesRead != bytesToRead)
    {
        ::fclose(fpRomFile);
        return false;
    }

    ::fclose(fpRomFile);

    return true;
}

bool Emulator_Init()
{
    ASSERT(g_pBoard == nullptr);

    CProcessor::Init();

    g_pBoard = new CMotherboard();

    // Allocate memory for old RAM values
    g_pEmulatorRam = (quint8*) ::malloc(65536);  ::memset(g_pEmulatorRam, 0, 65536);
    g_pEmulatorChangedRam = (quint8*) ::malloc(65536);  ::memset(g_pEmulatorChangedRam, 0, 65536);

    g_pBoard->Reset();

    g_sound = new QSoundOut();
    if (m_okEmulatorSound)
    {
        g_pBoard->SetSoundGenCallback(Emulator_FeedDAC);
    }

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    g_pBoard->SetTeletypeCallback(Emulator_TeletypeCallback);

    Emulator_OnUpdate();

    g_okEmulatorInitialized = true;
    return true;
}

void Emulator_Done()
{
    ASSERT(g_pBoard != nullptr);

    CProcessor::Done();

    g_pBoard->SetSoundGenCallback(nullptr);
    if (g_sound)
    {
        delete g_sound;
        g_sound = nullptr;
    }

    delete g_pBoard;
    g_pBoard = nullptr;

    // Free memory used for old RAM values
    ::free(g_pEmulatorRam);
    ::free(g_pEmulatorChangedRam);

    g_okEmulatorInitialized = false;
}

bool Emulator_InitConfiguration(BKConfiguration configuration)
{
    g_pBoard->SetConfiguration(configuration);

    quint8 buffer[8192];

    if ((configuration & BK_COPT_BK0011) == 0)
    {
        // Load Monitor ROM file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_MONIT10, buffer, 0, 8192))
        {
            AlertWarning("Failed to load Monitor ROM file.");
            return false;
        }
        g_pBoard->LoadROM(0, buffer);
    }

    if (configuration & BK_COPT_ROM_BASIC)
    {
        // Load BASIC ROM 1 file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC10_1, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BASIC ROM 1 file.");
            return false;
        }
        g_pBoard->LoadROM(1, buffer);
        // Load BASIC ROM 2 file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC10_2, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BASIC ROM 2 file.");
            return false;
        }
        g_pBoard->LoadROM(2, buffer);
        // Load BASIC ROM 3 file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC10_3, buffer, 0, 8064))
        {
            AlertWarning("Failed to load BASIC ROM 3 file.");
            return false;
        }
        g_pBoard->LoadROM(3, buffer);
    }
    else if (configuration & BK_COPT_ROM_FOCAL)
    {
        // Load Focal ROM file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_FOCAL, buffer, 0, 8192))
        {
            AlertWarning("Failed to load Focal ROM file.");
            return false;
        }
        g_pBoard->LoadROM(1, buffer);
        // Unused 8KB
        ::memset(buffer, 0, 8192);
        g_pBoard->LoadROM(2, buffer);
        // Load Tests ROM file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_TESTS, buffer, 0, 8064))
        {
            AlertWarning("Failed to load Tests ROM file.");
            return false;
        }
        g_pBoard->LoadROM(3, buffer);
    }

    if (configuration & BK_COPT_BK0011)
    {
        // Load BK0011M BASIC 0, part 1
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC11M_0, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BK11M BASIC 0 ROM file.");
            return false;
        }
        g_pBoard->LoadROM(0, buffer);
        // Load BK0011M BASIC 0, part 2
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC11M_0, buffer, 8192, 8192))
        {
            AlertWarning("Failed to load BK11M BASIC 0 ROM file.");
            return false;
        }
        g_pBoard->LoadROM(1, buffer);
        // Load BK0011M BASIC 1
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC11M_1, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BK11M BASIC 1 ROM file.");
            return false;
        }
        g_pBoard->LoadROM(2, buffer);

        // Load BK0011M EXT
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BK11M_EXT, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BK11M EXT ROM file.");
            return false;
        }
        g_pBoard->LoadROM(3, buffer);
        // Load BK0011M BOS
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BK11M_BOS, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BK11M BOS ROM file.");
            return false;
        }
        g_pBoard->LoadROM(4, buffer);
    }

    if (configuration & BK_COPT_FDD)
    {
        // Load disk driver ROM file
        ::memset(buffer, 0, 8192);
        if (!Emulator_LoadRomFile(FILENAME_BKROM_DISK_326, buffer, 0, 4096))
        {
            AlertWarning("Failed to load DISK ROM file.");
            return false;
        }
        g_pBoard->LoadROM((configuration & BK_COPT_BK0011) ? 5 : 3, buffer);
    }

    if ((configuration & BK_COPT_BK0011) && (configuration & BK_COPT_FDD) == 0)
    {
        // Load BK0011M MSTD
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BK11M_MSTD, buffer, 0, 8192))
        {
            AlertWarning("Failed to load BK11M MSTD ROM file.");
            return false;
        }
        g_pBoard->LoadROM(5, buffer);
    }

    g_nEmulatorConfiguration = configuration;

    g_pBoard->Reset();

#if 0  //DEBUG: CPU and memory tests
    //Emulator_LoadRomFile("791401", buffer, 8192);
    //g_pBoard->LoadRAM(0, buffer, 8192);
    //Emulator_LoadRomFile("791404", buffer, 6144);
    //g_pBoard->LoadRAM(0, buffer, 6144);
    Emulator_LoadRomFile("791323", buffer, 4096);
    g_pBoard->LoadRAM(0, buffer, 4096);

    g_pBoard->GetCPU()->SetPC(0200);  //DEBUG
    g_pBoard->GetCPU()->SetPSW(0000);  //DEBUG
#endif

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    return true;
}

void Emulator_Start()
{
    g_okEmulatorRunning = true;

    m_nFrameCount = 0;
    m_emulatorTime.restart();
    m_nTickCount = 0;
}
void Emulator_Stop()
{
    g_okEmulatorRunning = false;
    m_wEmulatorCPUBreakpoint = 0177777;

    // Reset FPS indicator
    Global_showFps(-1.0);

    Global_UpdateAllViews();
}

void Emulator_Reset()
{
    ASSERT(g_pBoard != nullptr);

    g_pBoard->Reset();

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;
    Global_showUptime(0);

    Global_UpdateAllViews();
}

void Emulator_SetCPUBreakpoint(quint16 address)
{
    m_wEmulatorCPUBreakpoint = address;
}

bool Emulator_IsBreakpoint()
{
    quint16 wCPUAddr = g_pBoard->GetCPU()->GetPC();
    if (wCPUAddr == m_wEmulatorCPUBreakpoint)
        return true;
    return false;
}

int Emulator_SystemFrame()
{
    g_pBoard->SetCPUBreakpoint(m_wEmulatorCPUBreakpoint);

    Emulator_ProcessKeyEvent();

    if (!g_pBoard->SystemFrame())
        return 0;

    // Calculate frames per second
    m_nFrameCount++;
    int nCurrentTicks = m_emulatorTime.elapsed();
    long nTicksElapsed = nCurrentTicks - m_nTickCount;
    if (nTicksElapsed >= 1200)
    {
        double dFramesPerSecond = m_nFrameCount * 1000.0 / nTicksElapsed;
        Global_showFps(dFramesPerSecond);

        m_nFrameCount = 0;
        m_nTickCount = nCurrentTicks;
    }

    // Calculate emulator uptime (25 frames per second)
    m_nUptimeFrameCount++;
    if (m_nUptimeFrameCount >= 25)
    {
        m_dwEmulatorUptime++;
        m_nUptimeFrameCount = 0;

        Global_showUptime(m_dwEmulatorUptime);
    }

    return 1;
}

float Emulator_GetUptime()
{
    return (float)m_dwEmulatorUptime + float(m_nUptimeFrameCount) / 25.0f;
}

// Update cached values after Run or Step
void Emulator_OnUpdate()
{
    // Update stored PC value
    g_wEmulatorPrevCpuPC = g_wEmulatorCpuPC;
    g_wEmulatorCpuPC = g_pBoard->GetCPU()->GetPC();

    // Update memory change flags
    {
        quint8* pOld = g_pEmulatorRam;
        quint8* pChanged = g_pEmulatorChangedRam;
        quint16 addr = 0;
        do
        {
            quint8 newvalue = g_pBoard->GetRAMByte(addr);
            quint8 oldvalue = *pOld;
            *pChanged = (newvalue != oldvalue) ? 255 : 0;
            *pOld = newvalue;
            addr++;
            pOld++;  pChanged++;
        }
        while (addr < 65535);
    }
}

// Get RAM change flag
//   addrtype - address mode - see ADDRTYPE_XXX constants
quint16 Emulator_GetChangeRamStatus(quint16 address)
{
    return *((quint16*)(g_pEmulatorChangedRam + address));
}

void Emulator_GetScreenSize(int scrmode, int* pwid, int* phei)
{
    if (scrmode < 0 || scrmode >= int(sizeof(ScreenModeReference) / sizeof(ScreenModeStruct)))
        return;
    ScreenModeStruct* pinfo = ScreenModeReference + scrmode;
    *pwid = pinfo->width;
    *phei = pinfo->height;
}

void Emulator_PrepareScreenRGB32(void* pImageBits, int screenMode)
{
    if (pImageBits == nullptr) return;
    if (!g_okEmulatorInitialized) return;

    // Get scroll value
    quint16 scroll = g_pBoard->GetPortView(0177664);
    bool okSmallScreen = ((scroll & 01000) == 0);
    scroll &= 0377;
    scroll = (scroll >= 0330) ? scroll - 0330 : 050 + scroll;

    // Get palette
    quint32* pPalette;
    if ((g_nEmulatorConfiguration & BK_COPT_BK0011) == 0)
        pPalette = (quint32*)ScreenView_ColorPalette;
    else
        pPalette = (quint32*)ScreenView_ColorPalettes[g_pBoard->GetPalette()];

    const quint8* pVideoBuffer = g_pBoard->GetVideoBuffer();
    ASSERT(pVideoBuffer != nullptr);

    // Render to bitmap
    PREPARE_SCREEN_CALLBACK callback = ScreenModeReference[screenMode].callback;
    callback(pVideoBuffer, okSmallScreen, pPalette, scroll, pImageBits);
}

#define AVERAGERGB(a, b)  ( (((a) & 0xfefefeffUL) + ((b) & 0xfefefeffUL)) >> 1 )

void CALLBACK Emulator_PrepareScreenBW512x256(const quint8* pVideoBuffer, int okSmallScreen, const quint32* /*pPalette*/, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        quint32* pBits = (quint32*)pImageBits + y * 512;
        for (int x = 0; x < 512 / 16; x++)
        {
            quint16 src = *pVideo;

            for (int bit = 0; bit < 16; bit++)
            {
                quint32 color = (src & 1) ? 0xffffffff : 0xff000000;
                *pBits = color;
                pBits++;
                src = src >> 1;
            }

            pVideo++;
        }
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (256 - 64) * 512 * sizeof(quint32));
    }
}

void CALLBACK Emulator_PrepareScreenColor512x256(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        quint32* pBits = (quint32*)pImageBits + y * 512;
        for (int x = 0; x < 512 / 16; x++)
        {
            quint16 src = *pVideo;

            for (int bit = 0; bit < 16; bit += 2)
            {
                quint32 color = pPalette[src & 3];
                *pBits = color;
                pBits++;
                *pBits = color;
                pBits++;
                src = src >> 2;
            }

            pVideo++;
        }
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (256 - 64) * 512 * sizeof(quint32));
    }
}

void CALLBACK Emulator_PrepareScreenBW512x384(const quint8* pVideoBuffer, int okSmallScreen, const quint32* /*pPalette*/, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    int bky = 0;
    for (int y = 0; y < 384; y++)
    {
        quint32* pBits = (quint32*)pImageBits + y * 512;
        if (y % 3 == 1)
            continue;  // Skip, fill later

        int yy = (bky + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        for (int x = 0; x < 512 / 16; x++)
        {
            quint16 src = *pVideo;

            for (int bit = 0; bit < 16; bit++)
            {
                quint32 color = (src & 1) ? 0xffffffff : 0xff000000;
                *pBits = color;
                pBits++;
                src = src >> 1;
            }

            pVideo++;
        }

        if (y % 3 == 2)  // Fill skipped line
        {
            quint8* pBits2 = (quint8*)((quint32*)pImageBits + (y - 0) * 512);
            quint8* pBits1 = (quint8*)((quint32*)pImageBits + (y - 1) * 512);
            quint8* pBits0 = (quint8*)((quint32*)pImageBits + (y - 2) * 512);
            for (int x = 0; x < 512 * 4; x++)
            {
                *pBits1 = (quint8)((((quint16) * pBits0) + ((quint16) * pBits2)) / 2);
                pBits2++;  pBits1++;  pBits0++;
            }
        }

        bky++;
        if (bky >= linesToShow) break;
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (384 - 86) * 512 * sizeof(quint32));  //TODO
    }
}

void CALLBACK Emulator_PrepareScreenColor512x384(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    int bky = 0;
    for (int y = 0; y < 384; y++)
    {
        quint32* pBits = (quint32*)pImageBits + y * 512;
        if (y % 3 == 1)
            continue;  // Skip, fill later

        int yy = (bky + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        for (int x = 0; x < 512 / 16; x++)
        {
            quint16 src = *pVideo;

            for (int bit = 0; bit < 16; bit += 2)
            {
                quint32 color = pPalette[src & 3];
                *pBits = color;
                pBits++;
                *pBits = color;
                pBits++;
                src = src >> 2;
            }

            pVideo++;
        }

        if (y % 3 == 2)  // Fill skipped line
        {
            quint8* pBits2 = (quint8*)((quint32*)pImageBits + (y - 0) * 512);
            quint8* pBits1 = (quint8*)((quint32*)pImageBits + (y - 1) * 512);
            quint8* pBits0 = (quint8*)((quint32*)pImageBits + (y - 2) * 512);
            for (int x = 0; x < 512 * 4; x++)
            {
                *pBits1 = (quint8)((((quint16) * pBits0) + ((quint16) * pBits2)) / 2);
                pBits2++;  pBits1++;  pBits0++;
            }
        }

        bky++;
        if (bky >= linesToShow) break;
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (384 - 86) * 512 * sizeof(quint32));  //TODO
    }
}

void CALLBACK Emulator_PrepareScreenBW896x512(const quint8* pVideoBuffer, int okSmallScreen, const quint32* /*pPalette*/, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        quint32* pBits = (quint32*)pImageBits + y * 896 * 2;
        quint32* pBits2 = pBits + 896;
        for (int x = 0; x < 512 / 16; x++)
        {
            uint16_t src = *pVideo;

            for (int bit = 0; bit < 16; bit += 4)
            {
                quint32 c1 = (src & 1) ? 0x0ffffff : 0;
                quint32 c2 = (src & 2) ? 0x0ffffff : 0;
                quint32 c3 = (src & 4) ? 0x0ffffff : 0;
                quint32 c4 = (src & 8) ? 0x0ffffff : 0;
                *(pBits++) = *(pBits2++) = c1;
                *(pBits++) = *(pBits2++) = AVERAGERGB(c1, c2);
                *(pBits++) = *(pBits2++) = c2;
                *(pBits++) = *(pBits2++) = AVERAGERGB(c2, c3);
                *(pBits++) = *(pBits2++) = c3;
                *(pBits++) = *(pBits2++) = AVERAGERGB(c3, c4);
                *(pBits++) = *(pBits2++) = c4;
                src = src >> 4;
            }

            pVideo++;
        }
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (256 - 64) * 896 * 2 * sizeof(quint32));  //TODO
    }
}

void CALLBACK Emulator_PrepareScreenColor896x512(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        quint32* pBits = (quint32*)pImageBits + y * 896 * 2;
        quint32* pBits2 = pBits + 896;
        for (int x = 0; x < 512 / 16; x++)
        {
            uint16_t src = *pVideo;

            for (int bit = 0; bit < 16; bit += 8)
            {
                quint32 c1 = pPalette[src & 3];  src = src >> 2;
                quint32 c2 = pPalette[src & 3];  src = src >> 2;
                quint32 c3 = pPalette[src & 3];  src = src >> 2;
                quint32 c4 = pPalette[src & 3];  src = src >> 2;
                quint32 c12 = AVERAGERGB(c1, c2);
                quint32 c23 = AVERAGERGB(c2, c3);
                quint32 c34 = AVERAGERGB(c3, c4);
                *(pBits++) = *(pBits2++) = c1;
                *(pBits++) = *(pBits2++) = c1;
                *(pBits++) = *(pBits2++) = c12;
                *(pBits++) = *(pBits2++) = c12;
                *(pBits++) = *(pBits2++) = c2;
                *(pBits++) = *(pBits2++) = c2;
                *(pBits++) = *(pBits2++) = c23;
                *(pBits++) = *(pBits2++) = c23;
                *(pBits++) = *(pBits2++) = c3;
                *(pBits++) = *(pBits2++) = c3;
                *(pBits++) = *(pBits2++) = c34;
                *(pBits++) = *(pBits2++) = c34;
                *(pBits++) = *(pBits2++) = c4;
                *(pBits++) = *(pBits2++) = c4;
            }

            pVideo++;
        }
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (256 - 64) * 896 * 2 * sizeof(quint32));  //TODO
    }
}

void CALLBACK Emulator_PrepareScreenBW1024x768(const quint8* pVideoBuffer, int okSmallScreen, const quint32* /*pPalette*/, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        quint32* pBits = (quint32*)pImageBits + y * 1024 * 3;
        quint32* pBits2 = pBits + 1024;
        quint32* pBits3 = pBits2 + 1024;
        for (int x = 0; x < 512 / 16; x++)
        {
            quint16 src = *pVideo;

            for (int bit = 0; bit < 16; bit++)
            {
                quint32 color = (src & 1) ? 0x0ffffff : 0;
                *pBits = *pBits2 = color;
                pBits++;  pBits2++;
                *pBits = *pBits2 = color;
                pBits++;  pBits2++;
                src = src >> 1;
            }

            pVideo++;
        }
        memset(pBits3, 0, 1024 * sizeof(quint32));
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (256 - 64) * 1024 * 3 * sizeof(quint32));  //TODO
    }
}

void CALLBACK Emulator_PrepareScreenColor1024x768(const quint8* pVideoBuffer, int okSmallScreen, const quint32* pPalette, int scroll, void* pImageBits)
{
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const quint16* pVideo = (quint16*)(pVideoBuffer + yy * 0100);
        quint32* pBits = (quint32*)pImageBits + y * 1024 * 3;
        quint32* pBits2 = pBits + 1024;
        quint32* pBits3 = pBits2 + 1024;
        for (int x = 0; x < 512 / 16; x++)
        {
            quint16 src = *pVideo;

            for (int bit = 0; bit < 16; bit += 2)
            {
                quint32 color = pPalette[src & 3];
                *pBits = *pBits2 = color;
                pBits++;  pBits2++;
                *pBits = *pBits2 = color;
                pBits++;  pBits2++;
                *pBits = *pBits2 = color;
                pBits++;  pBits2++;
                *pBits = *pBits2 = color;
                pBits++;  pBits2++;
                src = src >> 2;
            }

            pVideo++;
        }
        memset(pBits3, 0, 1024 * sizeof(quint32));
    }
    if (okSmallScreen)
    {
        memset((quint32*)pImageBits, 0, (256 - 64) * 1024 * 3 * sizeof(quint32));  //TODO
    }
}


void Emulator_KeyEvent(quint8 keyscan, bool pressed, bool ctrl)
{
    if (m_EmulatorKeyQueueCount == KEYEVENT_QUEUE_SIZE) return;  // Full queue

    quint16 keyflags = (pressed ? 128 : 0) | (ctrl ? 64 : 0);
    quint16 keyevent = ((quint16)keyscan) | (keyflags << 8);

    m_EmulatorKeyQueue[m_EmulatorKeyQueueTop] = keyevent;
    m_EmulatorKeyQueueTop++;
    if (m_EmulatorKeyQueueTop >= KEYEVENT_QUEUE_SIZE)
        m_EmulatorKeyQueueTop = 0;
    m_EmulatorKeyQueueCount++;
}

quint16 Emulator_GetKeyEventFromQueue()
{
    if (m_EmulatorKeyQueueCount == 0) return 0;  // Empty queue

    quint16 keyevent = m_EmulatorKeyQueue[m_EmulatorKeyQueueBottom];
    m_EmulatorKeyQueueBottom++;
    if (m_EmulatorKeyQueueBottom >= KEYEVENT_QUEUE_SIZE)
        m_EmulatorKeyQueueBottom = 0;
    m_EmulatorKeyQueueCount--;

    return keyevent;
}

void Emulator_ProcessKeyEvent()
{
    // Process next event in the keyboard queue
    quint16 keyevent = Emulator_GetKeyEventFromQueue();
    if (keyevent != 0)
    {
        bool pressed = ((keyevent & 0x8000) != 0);
        bool ctrl = ((keyevent & 0x4000) != 0);
        quint8 bkscan = (quint8)(keyevent & 0xff);
        g_pBoard->KeyboardEvent(bkscan, pressed, ctrl);
    }
}

void CALLBACK Emulator_FeedDAC(unsigned short l, unsigned short r)
{
    if (g_sound)
    {
        if (m_okEmulatorSound)
            g_sound->FeedDAC(l, r);
    }
}

void Emulator_SetSound(bool enable)
{
    m_okEmulatorSound = enable;
    if (g_pBoard != nullptr)
    {
        if (enable)
            g_pBoard->SetSoundGenCallback(Emulator_FeedDAC);
        else
            g_pBoard->SetSoundGenCallback(nullptr);
    }
}

void CALLBACK Emulator_TeletypeCallback(quint8 symbol)
{
    if (symbol >= 32 || symbol == 13 || symbol == 10)
    {
        Global_getMainWindow()->printToTeletype(QString((TCHAR)symbol));
    }
    else
    {
        char buffer[32];
        _snprintf(buffer, 32, "<%02x>", symbol);
        Global_getMainWindow()->printToTeletype(buffer);
    }
}


//////////////////////////////////////////////////////////////////////
//
// Emulator image format - see CMotherboard::SaveToImage()
// Image header format (32 bytes):
//   4 bytes        BK_IMAGE_HEADER1
//   4 bytes        BK_IMAGE_HEADER2
//   4 bytes        BK_IMAGE_VERSION
//   4 bytes        BK_IMAGE_SIZE
//   4 bytes        BK uptime
//   12 bytes       Not used

bool Emulator_SaveImage(const QString &sFilePath)
{
    QFile file(sFilePath);
    if (! file.open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to save image file."));
        return false;
    }

    // Allocate memory
    quint8* pImage = (quint8*) ::calloc(BKIMAGE_SIZE, 1);
    if (pImage == nullptr)
    {
        file.close();
        return false;
    }
    // Prepare header
    quint32* pHeader = (quint32*) pImage;
    *pHeader++ = BKIMAGE_HEADER1;
    *pHeader++ = BKIMAGE_HEADER2;
    *pHeader++ = BKIMAGE_VERSION;
    *pHeader++ = BKIMAGE_SIZE;
    // Store emulator state to the image
    g_pBoard->SaveToImage(pImage);
    *(quint32*)(pImage + 16) = m_dwEmulatorUptime;

    // Save image to the file
    qint64 bytesWritten = file.write((const char *)pImage, BKIMAGE_SIZE);
    if (bytesWritten != BKIMAGE_SIZE)
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to save image file data."));
        return false;
    }

    // Free memory, close file
    ::free(pImage);
    file.close();

    return true;
}

bool Emulator_LoadImage(const QString &sFilePath)
{
    Emulator_Stop();

    QFile file(sFilePath);
    if (! file.open(QIODevice::ReadOnly))
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to load image file."));
        return false;
    }

    // Read header
    quint32 bufHeader[BKIMAGE_HEADER_SIZE / sizeof(quint32)];
    qint64 bytesRead = file.read((char*)bufHeader, BKIMAGE_HEADER_SIZE);
    if (bytesRead != BKIMAGE_HEADER_SIZE)
    {
        file.close();
        return false;
    }

    //TODO: Check version and size

    // Allocate memory
    quint8* pImage = (quint8*) ::malloc(BKIMAGE_SIZE);
    if (pImage == nullptr)
    {
        file.close();
        return false;
    }

    // Read image
    file.seek(0);
    bytesRead = file.read((char*)pImage, BKIMAGE_SIZE);
    if (bytesRead != BKIMAGE_SIZE)
    {
        ::free(pImage);
        file.close();
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to load image file data."));
        return false;
    }

    // Restore emulator state from the image
    g_pBoard->LoadFromImage(pImage);

    m_dwEmulatorUptime = *(quint32*)(pImage + 16);
    g_wEmulatorCpuPC = g_pBoard->GetCPU()->GetPC();

    // Free memory, close file
    ::free(pImage);
    file.close();

    return true;
}


//////////////////////////////////////////////////////////////////////
