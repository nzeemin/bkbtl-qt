// Emulator.cpp

#include "stdafx.h"
#include "main.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
//#include "SoundGen.h"


//////////////////////////////////////////////////////////////////////


CMotherboard* g_pBoard = NULL;
BKConfiguration g_nEmulatorConfiguration;  // Current configuration

BOOL g_okEmulatorInitialized = FALSE;
BOOL g_okEmulatorRunning = FALSE;

WORD m_wEmulatorCPUBreakpoint = 0177777;

BOOL m_okEmulatorSound = FALSE;

long m_nFrameCount = 0;
DWORD m_dwTickCount = 0;
DWORD m_dwEmulatorUptime = 0;  // BK uptime, seconds, from turn on or reset, increments every 25 frames
long m_nUptimeFrameCount = 0;

BYTE* g_pEmulatorRam;  // RAM values - for change tracking
BYTE* g_pEmulatorChangedRam;  // RAM change flags
WORD g_wEmulatorCpuPC = 0177777;      // Current PC value
WORD g_wEmulatorPrevCpuPC = 0177777;  // Previous PC value

const int KEYEVENT_QUEUE_SIZE = 32;
WORD m_EmulatorKeyQueue[KEYEVENT_QUEUE_SIZE];
int m_EmulatorKeyQueueTop = 0;
int m_EmulatorKeyQueueBottom = 0;
int m_EmulatorKeyQueueCount = 0;


//////////////////////////////////////////////////////////////////////
// Colors

const DWORD ScreenView_ColorPalette[4] = {
    0x000000, 0x0000FF, 0x00FF00, 0xFF0000
};

const DWORD ScreenView_ColorPalettes[16][4] = {
    //                                         Palette#     01           10          11
    { 0x000000, 0x0000FF, 0x00FF00, 0xFF0000 },  // 00    синий   |   зеленый  |  красный
    { 0x000000, 0xFFFF00, 0xFF00FF, 0xFF0000 },  // 01   желтый   |  сиреневый |  красный
    { 0x000000, 0x00FFFF, 0x0000FF, 0xFF00FF },  // 02   голубой  |    синий   | сиреневый
    { 0x000000, 0x00FF00, 0x00FFFF, 0xFFFF00 },  // 03   зеленый  |   голубой  |  желтый
    { 0x000000, 0xFF00FF, 0x00FFFF, 0xFFFFFF },  // 04  сиреневый |   голубой  |   белый
    { 0x000000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF },  // 05    белый   |    белый   |   белый
    { 0x000000, 0x7F0000, 0x7F0000, 0xFF0000 },  // 06  темн-красн| красн-корич|  красный
    { 0x000000, 0x00FF7F, 0x00FF7F, 0xFFFF00 },  // 07  салатовый | светл-зелен|  желтый
    { 0x000000, 0xFF00FF, 0x7F00FF, 0x7F007F },  // 08  фиолетовый| фиол-синий | сиреневый
    { 0x000000, 0x00FF7F, 0x7F00FF, 0x7F0000 },  // 09 светл-зелен| фиол-синий |красн-корич
    { 0x000000, 0x00FF7F, 0x7F007F, 0x7F0000 },  // 10  салатовый | фиолетовый |темн-красный
    { 0x000000, 0x00FFFF, 0xFFFF00, 0xFF0000 },  // 11   голубой  |   желтый   |  красный
    { 0x000000, 0xFF0000, 0x00FF00, 0x00FFFF },  // 12   красный  |   зеленый  |  голубой
    { 0x000000, 0x00FFFF, 0xFFFF00, 0xFFFFFF },  // 13   голубой  |   желтый   |   белый
    { 0x000000, 0xFFFF00, 0x00FF00, 0xFFFFFF },  // 14   желтый   |   зеленый  |   белый
    { 0x000000, 0x00FFFF, 0x00FF00, 0xFFFFFF },  // 15   голубой  |   зеленый  |   белый
};


//////////////////////////////////////////////////////////////////////


const LPCTSTR FILENAME_BKROM_MONIT10    = _T("monit10.rom");
const LPCTSTR FILENAME_BKROM_FOCAL      = _T("focal.rom");
const LPCTSTR FILENAME_BKROM_TESTS      = _T("tests.rom");
const LPCTSTR FILENAME_BKROM_BASIC10_1  = _T("basic10_1.rom");
const LPCTSTR FILENAME_BKROM_BASIC10_2  = _T("basic10_2.rom");
const LPCTSTR FILENAME_BKROM_BASIC10_3  = _T("basic10_3.rom");
const LPCTSTR FILENAME_BKROM_DISK_327   = _T("disk_327.rom");
const LPCTSTR FILENAME_BKROM_BK11M_BOS  = _T("b11m_bos.rom");
const LPCTSTR FILENAME_BKROM_BK11M_EXT  = _T("b11m_ext.rom");


BOOL Emulator_LoadRomFile(LPCTSTR strFileName, BYTE* buffer, DWORD bytesToRead)
{
    FILE* fpRomFile = ::_tfopen(strFileName, _T("rb"));
    if (fpRomFile == NULL)
        return FALSE;

    ASSERT(bytesToRead <= 8192);
    ::memset(buffer, 0, 8192);

    DWORD dwBytesRead = ::fread(buffer, 1, bytesToRead, fpRomFile);
    if (dwBytesRead != bytesToRead)
    {
        ::fclose(fpRomFile);
        return FALSE;
    }

    ::fclose(fpRomFile);

    return TRUE;
}


BOOL Emulator_Init()
{
    ASSERT(g_pBoard == NULL);

    CProcessor::Init();

    g_pBoard = new CMotherboard();

    // Allocate memory for old RAM values
    g_pEmulatorRam = (BYTE*) ::malloc(65536);  memset(g_pEmulatorRam, 0, 65536);
    g_pEmulatorChangedRam = (BYTE*) ::malloc(65536);  memset(g_pEmulatorChangedRam, 0, 65536);

    g_pBoard->Reset();

    //if (m_okEmulatorSound)
    //{
    //    SoundGen_Initialize();
    //    g_pBoard->SetSoundGenCallback(SoundGen_FeedDAC);
    //}

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    //g_pBoard->SetTeletypeCallback(Emulator_TeletypeCallback);

    g_okEmulatorInitialized = TRUE;
    return TRUE;
}

void Emulator_Done()
{
    ASSERT(g_pBoard != NULL);

    CProcessor::Done();

    g_pBoard->SetSoundGenCallback(NULL);
    //SoundGen_Finalize();

    delete g_pBoard;
    g_pBoard = NULL;

    // Free memory used for old RAM values
    ::free(g_pEmulatorRam);
    ::free(g_pEmulatorChangedRam);

    g_okEmulatorInitialized = FALSE;
}


BOOL Emulator_InitConfiguration(BKConfiguration configuration)
{
    g_pBoard->SetConfiguration(configuration);

    BYTE buffer[8192];

    // Load Monitor ROM file - in all configurations
    if (!Emulator_LoadRomFile(FILENAME_BKROM_MONIT10, buffer, 8192))
    {
        AlertWarning(_T("Failed to load Monitor ROM file."));
        return FALSE;
    }
    g_pBoard->LoadROM(0, buffer);

    if (configuration & BK_COPT_ROM_BASIC)
    {
        // Load BASIC ROM 1 file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC10_1, buffer, 8192))
        {
            AlertWarning(_T("Failed to load BASIC ROM 1 file."));
            return FALSE;
        }
        g_pBoard->LoadROM(1, buffer);
        // Load BASIC ROM 2 file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC10_2, buffer, 8192))
        {
            AlertWarning(_T("Failed to load BASIC ROM 2 file."));
            return FALSE;
        }
        g_pBoard->LoadROM(2, buffer);
        // Load BASIC ROM 3 file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BASIC10_3, buffer, 8064))
        {
            AlertWarning(_T("Failed to load BASIC ROM 3 file."));
            return FALSE;
        }
        g_pBoard->LoadROM(3, buffer);
    }
    else if (configuration & BK_COPT_ROM_FOCAL)
    {
        // Load Focal ROM file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_FOCAL, buffer, 8192))
        {
            AlertWarning(_T("Failed to load Focal ROM file."));
            return FALSE;
        }
        g_pBoard->LoadROM(1, buffer);
        // Unused 8KB
        ::memset(buffer, 0, 8192);
        g_pBoard->LoadROM(2, buffer);
        // Load Tests ROM file
        if (!Emulator_LoadRomFile(FILENAME_BKROM_TESTS, buffer, 8064))
        {
            AlertWarning(_T("Failed to load Tests ROM file."));
            return FALSE;
        }
        g_pBoard->LoadROM(3, buffer);
    }

    if (configuration & BK_COPT_BK0011)
    {
        // Load BK0011M EXT
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BK11M_EXT, buffer, 8192))
        {
            AlertWarning(_T("Failed to load BK11M EXT ROM file."));
            return FALSE;
        }
        g_pBoard->LoadROM(1, buffer);
        // Load BK0011M BOS
        if (!Emulator_LoadRomFile(FILENAME_BKROM_BK11M_BOS, buffer, 8192))
        {
            AlertWarning(_T("Failed to load BK11M BOS ROM file."));
            return FALSE;
        }
        g_pBoard->LoadROM(2, buffer);
    }

    if (configuration & BK_COPT_FDD)
    {
        // Load disk driver ROM file
        ::memset(buffer, 0, 8192);
        if (!Emulator_LoadRomFile(FILENAME_BKROM_DISK_327, buffer, 4096))
        {
            AlertWarning(_T("Failed to load DISK 327 ROM file."));
            return FALSE;
        }
        g_pBoard->LoadROM(3, buffer);
    }

    g_nEmulatorConfiguration = configuration;

    g_pBoard->Reset();

#if 0  //DEBUG: CPU and memory tests
    //Emulator_LoadRomFile(_T("791401"), buffer, 8192);
    //g_pBoard->LoadRAM(0, buffer, 8192);
    //Emulator_LoadRomFile(_T("791404"), buffer, 6144);
    //g_pBoard->LoadRAM(0, buffer, 6144);
    Emulator_LoadRomFile(_T("791323"), buffer, 4096);
    g_pBoard->LoadRAM(0, buffer, 4096);

    g_pBoard->GetCPU()->SetPC(0200);  //DEBUG
    g_pBoard->GetCPU()->SetPSW(0000);  //DEBUG
#endif

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    return TRUE;
}

void Emulator_Start()
{
    g_okEmulatorRunning = TRUE;

    m_nFrameCount = 0;
    //m_dwTickCount = GetTickCount();
}
void Emulator_Stop()
{
    g_okEmulatorRunning = FALSE;
    m_wEmulatorCPUBreakpoint = 0177777;

    // Reset FPS indicator
    //MainWindow_SetStatusbarText(StatusbarPartFPS, _T(""));

    //MainWindow_UpdateAllViews();
}

void Emulator_Reset()
{
    ASSERT(g_pBoard != NULL);

    g_pBoard->Reset();

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    //MainWindow_UpdateAllViews();
}

int Emulator_SystemFrame()
{
    g_pBoard->SetCPUBreakpoint(m_wEmulatorCPUBreakpoint);

    //ScreenView_ScanKeyboard();
    Emulator_ProcessKeyEvent();
    
	if (!g_pBoard->SystemFrame())
        return 0;

    //// Calculate frames per second
    //m_nFrameCount++;
    //DWORD dwCurrentTicks = GetTickCount();
    //long nTicksElapsed = dwCurrentTicks - m_dwTickCount;
    //if (nTicksElapsed >= 1200)
    //{
    //	double dFramesPerSecond = m_nFrameCount * 1000.0 / nTicksElapsed;
    //	TCHAR buffer[16];
    //	_stprintf(buffer, _T("FPS: %05.2f"), dFramesPerSecond);
    //    //MainWindow_SetStatusbarText(StatusbarPartFPS, buffer);

    //	m_nFrameCount = 0;
    //	m_dwTickCount = dwCurrentTicks;
    //}

    //// Calculate emulator uptime (25 frames per second)
    //m_nUptimeFrameCount++;
    //if (m_nUptimeFrameCount >= 25)
    //{
    //	m_dwEmulatorUptime++;
    //	m_nUptimeFrameCount = 0;

    //	int seconds = (int) (m_dwEmulatorUptime % 60);
    //	int minutes = (int) (m_dwEmulatorUptime / 60 % 60);
    //	int hours   = (int) (m_dwEmulatorUptime / 3600 % 60);

    //	TCHAR buffer[20];
    //	_stprintf(buffer, _T("Uptime: %02d:%02d:%02d"), hours, minutes, seconds);
    //    //MainWindow_SetStatusbarText(StatusbarPartUptime, buffer);
    //}

    return 1;
}

// Update cached values after Run or Step
void Emulator_OnUpdate()
{
    // Update stored PC value
    g_wEmulatorPrevCpuPC = g_wEmulatorCpuPC;
    g_wEmulatorCpuPC = g_pBoard->GetCPU()->GetPC();

    // Update memory change flags
    {
        BYTE* pOld = g_pEmulatorRam;
        BYTE* pChanged = g_pEmulatorChangedRam;
        WORD addr = 0;
        do
        {
            BYTE newvalue = g_pBoard->GetRAMByte(addr);
            BYTE oldvalue = *pOld;
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
WORD Emulator_GetChangeRamStatus(WORD address)
{
    return *((WORD*)(g_pEmulatorChangedRam + address));
}

void Emulator_PrepareScreenRGB32(void* pImageBits, ScreenViewMode screenMode)
{
    if (pImageBits == NULL) return;
    if (!g_okEmulatorInitialized) return;

    // Get scroll value
    WORD scroll = g_pBoard->GetPortView(0177664);
    BOOL okSmallScreen = ((scroll & 01000) == 0);
    scroll &= 0377;
    scroll = (scroll >= 0330) ? scroll - 0330 : 050 + scroll;

    // Get palette
    DWORD* pPalette;
    if ((g_nEmulatorConfiguration & BK_COPT_BK0011) == 0)
        pPalette = (DWORD*)ScreenView_ColorPalette;
    else
        pPalette = (DWORD*)ScreenView_ColorPalettes[g_pBoard->GetPalette()];

    const BYTE* pVideoBuffer = g_pBoard->GetVideoBuffer();
    ASSERT(pVideoBuffer != NULL);

    // Render to bitmap
    int linesToShow = okSmallScreen ? 64 : 256;
    for (int y = 0; y < linesToShow; y++)
    {
        int yy = (y + scroll) & 0377;
        const WORD* pVideo = (WORD*)(pVideoBuffer + yy * 0100);
        DWORD* pBits = (DWORD*)pImageBits + y * 512;
        for (int x = 0; x < 512 / 16; x++)
        {
            WORD src = *pVideo;

            if (screenMode == BlackWhiteScreen)  // Black and white mode 512 x 256
            {
                for (int bit = 0; bit < 16; bit++)
                {
                    DWORD color = (src & 1) ? 0x0ffffff : 0;
                    *pBits = color;
                    pBits++;
                    src = src >> 1;
                }
            }
            else  // Color mode 256 x 256
            {
                for (int bit = 0; bit < 16; bit += 2)
                {
                    DWORD color = pPalette[src & 3];
                    *pBits = color;
                    pBits++;
                    *pBits = color;
                    pBits++;
                    src = src >> 2;
                }
            }

            pVideo++;
        }
    }

    if (okSmallScreen)
    {
        memset((DWORD*)pImageBits, 64 * 512, (256 - 64) * 512 * sizeof(DWORD));
    }
}

void Emulator_KeyEvent(BYTE keyscan, BOOL pressed)
{
    if (m_EmulatorKeyQueueCount == KEYEVENT_QUEUE_SIZE) return;  // Full queue

    WORD keyevent = MAKEWORD(keyscan, pressed ? 128 : 0);

    m_EmulatorKeyQueue[m_EmulatorKeyQueueTop] = keyevent;
    m_EmulatorKeyQueueTop++;
    if (m_EmulatorKeyQueueTop >= KEYEVENT_QUEUE_SIZE)
        m_EmulatorKeyQueueTop = 0;
    m_EmulatorKeyQueueCount++;
}

WORD Emulator_GetKeyEventFromQueue()
{
    if (m_EmulatorKeyQueueCount == 0) return 0;  // Empty queue

    WORD keyevent = m_EmulatorKeyQueue[m_EmulatorKeyQueueBottom];
    m_EmulatorKeyQueueBottom++;
    if (m_EmulatorKeyQueueBottom >= KEYEVENT_QUEUE_SIZE)
        m_EmulatorKeyQueueBottom = 0;
    m_EmulatorKeyQueueCount--;

    return keyevent;
}

void Emulator_ProcessKeyEvent()
{
    // Process next event in the keyboard queue
    WORD keyevent = Emulator_GetKeyEventFromQueue();
    if (keyevent != 0)
    {
        BOOL pressed = ((keyevent & 0x8000) != 0);
        BOOL ctrl = ((keyevent & 0x4000) != 0);
        BYTE bkscan = LOBYTE(keyevent);
        g_pBoard->KeyboardEvent(bkscan, pressed, ctrl);
    }
}


//////////////////////////////////////////////////////////////////////
