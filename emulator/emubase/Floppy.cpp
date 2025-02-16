﻿/*  This file is part of BKBTL.
    BKBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    BKBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
BKBTL. If not, see <http://www.gnu.org/licenses/>. */

// Floppy.cpp
// Floppy controller and drives emulation
// See defines in header file Emubase.h

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "Emubase.h"


//////////////////////////////////////////////////////////////////////

// Маска флагов, сохраняемых в m_flags
const uint16_t FLOPPY_CMD_MASKSTORED =
    0x000F |  // Floppy drive selection bits
    FLOPPY_CMD_ENGINESTART | FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_STEP | FLOPPY_CMD_READDATA |
    FLOPPY_CMD_WRITEMARKER | FLOPPY_CMD_PRECORRECTION;

static void EncodeTrackData(const uint8_t* pSrc, uint8_t* data, uint8_t* marker, uint16_t track, uint16_t side);
static bool DecodeTrackData(const uint8_t* pRaw, uint8_t* pDest);

//////////////////////////////////////////////////////////////////////


CFloppyDrive::CFloppyDrive()
{
    fpFile = nullptr;
    okReadOnly = false;
    datatrack = dataside = 0;
    dataptr = 0;
    memset(data, 0, sizeof(data));
    memset(marker, 0, sizeof(marker));
}

void CFloppyDrive::Reset()
{
    datatrack = dataside = 0;
    dataptr = 0;
}


//////////////////////////////////////////////////////////////////////


CFloppyController::CFloppyController()
{
    m_side = m_track = 0;
    m_drive = -1;
    m_pDrive = nullptr;
    m_datareg = m_writereg = m_shiftreg = 0;
    m_searchsynctrig = false;
    m_writing = m_searchsync = m_writemarker = m_crccalculus = false;
    m_writeflag = m_shiftflag = m_shiftmarker = false;
    m_trackchanged = false;
    m_status = FLOPPY_STATUS_TRACK0;
    m_flags = FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_WRITEMARKER;
    m_okTrace = false;
}

CFloppyController::~CFloppyController()
{
    for (int drive = 0; drive < 4; drive++)
        DetachImage(drive);
}

void CFloppyController::Reset()
{
    if (m_okTrace) DebugLog(_T("Floppy RESET\r\n"));

    FlushChanges();

    m_side = m_track = 0;
    m_drive = -1;
    m_pDrive = nullptr;
    m_datareg = m_writereg = m_shiftreg = 0;
    m_searchsynctrig = false;
    m_writing = m_searchsync = m_writemarker = m_crccalculus = false;
    m_writeflag = m_shiftflag = false;
    m_trackchanged = false;
    m_status = FLOPPY_STATUS_TRACK0;
    m_flags = FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_WRITEMARKER;

    PrepareTrack();
}

bool CFloppyController::AttachImage(int drive, LPCTSTR sFileName)
{
    ASSERT(sFileName != nullptr);

    // If image attached - detach one first
    if (m_drivedata[drive].fpFile != nullptr)
        DetachImage(drive);

    // Open file
    m_drivedata[drive].okReadOnly = false;
    m_drivedata[drive].fpFile = ::_tfopen(sFileName, _T("r+b"));
    if (m_drivedata[drive].fpFile == nullptr)
    {
        m_drivedata[drive].okReadOnly = true;
        m_drivedata[drive].fpFile = ::_tfopen(sFileName, _T("rb"));
    }
    if (m_drivedata[drive].fpFile == nullptr)
        return false;

    m_side = m_track = m_drivedata[drive].datatrack = m_drivedata[drive].dataside = 0;
    m_drivedata[drive].dataptr = 0;
    m_datareg = m_writereg = m_shiftreg = 0;
    m_writing = m_searchsync = m_writemarker = m_crccalculus = false;
    m_writeflag = m_shiftflag = false;
    m_trackchanged = false;
    m_status = (m_pDrive != nullptr && m_pDrive->okReadOnly) ? FLOPPY_STATUS_TRACK0 | FLOPPY_STATUS_WRITEPROTECT : FLOPPY_STATUS_TRACK0;
    m_flags = FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_WRITEMARKER;

    PrepareTrack();

    return true;
}

void CFloppyController::DetachImage(int drive)
{
    if (m_drivedata[drive].fpFile == nullptr) return;

    FlushChanges();

    ::fclose(m_drivedata[drive].fpFile);
    m_drivedata[drive].fpFile = nullptr;
    m_drivedata[drive].okReadOnly = false;
    m_drivedata[drive].Reset();
}

//////////////////////////////////////////////////////////////////////


uint16_t CFloppyController::GetState()
{
    if (m_pDrive == nullptr)
        return 0;
    if (m_pDrive->fpFile == nullptr)
        return FLOPPY_STATUS_INDEXMARK | (m_track == 0 ? FLOPPY_STATUS_TRACK0 : 0);

    if (m_track == 0)
        m_status |= FLOPPY_STATUS_TRACK0;
    else
        m_status &= ~FLOPPY_STATUS_TRACK0;

    if (m_pDrive->dataptr < FLOPPY_INDEXLENGTH)
        m_status |= FLOPPY_STATUS_INDEXMARK;
    else
        m_status &= ~FLOPPY_STATUS_INDEXMARK;

    if (IsEngineOn())
        m_status |= FLOPPY_STATUS_RDY;
    else
        m_status &= ~(FLOPPY_STATUS_RDY | FLOPPY_STATUS_MOREDATA | FLOPPY_STATUS_CHECKSUMOK);

    return m_status;
}

void CFloppyController::SetCommand(uint16_t cmd)
{
//    if (m_okTrace)
//        DebugLogFormat(_T("Floppy COMMAND %06o\r\n"), cmd);

    // Copy new flags to m_flags
    m_flags &= ~FLOPPY_CMD_MASKSTORED;
    m_flags |= cmd & FLOPPY_CMD_MASKSTORED;

    bool okPrepareTrack = false;  // Нужно ли считывать дорожку в буфер

    // Проверить, не сменился ли текущий привод;
    // тут выбирается текущий привод, чем более младший бит, тем больше приоритет;
    // если установлено несколько битов - выбирается тот, что младше.
    int newdrive = (cmd & 1) ? 0 : (cmd & 2) ? 1 : (cmd & 4) ? 2 : (cmd & 8) ? 3 : -1;

    if (m_drive != newdrive)
    {
        FlushChanges();

        m_drive = newdrive;
        m_pDrive = (newdrive == -1) ? nullptr : m_drivedata + m_drive;
        okPrepareTrack = true;

        if (m_okTrace)
            DebugLogFormat(_T("Floppy CURRENT DRIVE %d\r\n"), newdrive);
    }
    if (m_drive == -1)
        return;

    // Проверяем, не сменилась ли сторона
    if (m_flags & FLOPPY_CMD_SIDEUP)  // Side selection: 0 - down, 1 - up
    {
        if (m_side == 0) { m_side = 1;  okPrepareTrack = true; }
    }
    else
    {
        if (m_side == 1) { m_side = 0;  okPrepareTrack = true; }
    }

    if (m_flags & FLOPPY_CMD_STEP)  // Move head for one track to center or from center
    {
        if (m_okTrace)
            DebugLogFormat(_T("Floppy STEP %d\r\n"), (m_flags & FLOPPY_CMD_DIR) ? 1 : 0);

        if (m_flags & FLOPPY_CMD_DIR)
        {
            if (m_track < 82) { m_track++;  okPrepareTrack = true; }
        }
        else
        {
            if (m_track >= 1) { m_track--;  okPrepareTrack = true; }
        }

        if (m_track == 0)
            m_status |= FLOPPY_STATUS_TRACK0;
        else
            m_status &= ~FLOPPY_STATUS_TRACK0;
    }
    if (okPrepareTrack)
        PrepareTrack();

    if (m_flags & FLOPPY_CMD_READDATA) // Search for marker
    {
        if (m_okTrace)
            DebugLog(_T("Floppy SEARCHSYNC\r\n"));

        m_searchsynctrig = true;
    }
    else if (m_searchsynctrig)
    {
        m_searchsynctrig = false;
        m_searchsync = true;
        m_crccalculus = true;
        m_status &= ~(FLOPPY_STATUS_CHECKSUMOK | FLOPPY_STATUS_MOREDATA);
    }

    if (m_writing && (cmd & FLOPPY_CMD_WRITEMARKER))  // Запись маркера
    {
        m_writemarker = true;
        m_status &= ~(FLOPPY_STATUS_CHECKSUMOK | FLOPPY_STATUS_MOREDATA);
    }
}

uint16_t CFloppyController::GetData(void)
{
    if (m_okTrace && m_pDrive != nullptr)
    {
        uint16_t offset = m_pDrive->dataptr;
        if (offset >= 102 && (offset - 102) % 610 == 0)
            DebugLogFormat(_T("Floppy READ\t\tTRACK %d SIDE %d SECTOR %d \r\n"), (int)m_track, (int)m_side, (offset - 102) / 610);
    }

    m_status &= ~FLOPPY_STATUS_MOREDATA;
    m_writing = m_searchsync = false;
    m_writeflag = m_shiftflag = false;

    if (m_pDrive == nullptr || m_pDrive->fpFile == nullptr)
        return 0;

    return m_datareg;
}

void CFloppyController::WriteData(uint16_t data)
{
//    if (m_okTrace)
//        DebugLogFormat(_T("Floppy WRITE\t\t%04x\r\n"), data);  //DEBUG

    m_writing = true;  // Switch to write mode if not yet
    m_searchsync = false;

    if (!m_writeflag && !m_shiftflag)  // Both registers are empty
    {
        m_shiftreg = data;
        m_shiftflag = true;
        m_status |= FLOPPY_STATUS_MOREDATA;
    }
    else if (!m_writeflag && m_shiftflag)  // Write register is empty
    {
        m_writereg = data;
        m_writeflag = true;
        m_status &= ~FLOPPY_STATUS_MOREDATA;
    }
    else if (m_writeflag && !m_shiftflag)  // Shift register is empty
    {
        m_shiftreg = m_writereg;
        m_shiftflag = m_writeflag;
        m_writereg = data;
        m_writeflag = true;
        m_status &= ~FLOPPY_STATUS_MOREDATA;
    }
    else  // Both registers are not empty
    {
        m_writereg = data;
    }
}

void CFloppyController::Periodic()
{
    if (!IsEngineOn()) return;  // Вращаем дискеты только если включен мотор

    // Вращаем дискеты во всех драйвах сразу
    for (int drive = 0; drive < 4; drive++)
    {
        m_drivedata[drive].dataptr += 2;
        if (m_drivedata[drive].dataptr >= FLOPPY_RAWTRACKSIZE)
            m_drivedata[drive].dataptr = 0;
    }

    if (m_okTrace && m_pDrive != nullptr && m_pDrive->dataptr == 0)
        DebugLogFormat(_T("Floppy Index\n"));

    // Далее обрабатываем чтение/запись на текущем драйве
    if (m_pDrive == nullptr) return;
    if (!IsAttached(m_drive)) return;

    if (!m_writing)  // Read mode
    {
        m_datareg = (m_pDrive->data[m_pDrive->dataptr] << 8) | m_pDrive->data[m_pDrive->dataptr + 1];
        if (m_status & FLOPPY_STATUS_MOREDATA)
        {
            if (m_crccalculus)  // Stop CRC calculation
            {
                m_crccalculus = false;
                //TODO: Compare calculated CRC to m_datareg
                m_status |= FLOPPY_STATUS_CHECKSUMOK;
            }
        }
        else
        {
            if (m_searchsync)  // Search for marker
            {
                if (m_pDrive->marker[m_pDrive->dataptr / 2])  // Marker found
                {
                    m_status |= FLOPPY_STATUS_MOREDATA;
                    m_searchsync = false;

                    if (m_okTrace)
                        DebugLogFormat(_T("Floppy Marker Found\n"));
                }
            }
            else  // Just read
                m_status |= FLOPPY_STATUS_MOREDATA;
        }
    }
    else  // Write mode
    {
        if (m_shiftflag)
        {
            m_pDrive->data[m_pDrive->dataptr] = (uint8_t)(m_shiftreg & 0xff); // LOBYTE
            m_pDrive->data[m_pDrive->dataptr + 1] = (uint8_t)((m_shiftreg >> 8) & 0xff); // HIBYTE
            m_shiftflag = false;
            m_trackchanged = true;

            if (m_shiftmarker)
            {
//                if (m_okTrace)
//                    DebugLogFormat(_T("Floppy WRITING %06o MARKER\r\n"), m_shiftreg);  //DEBUG

                m_pDrive->marker[m_pDrive->dataptr / 2] = true;
                m_shiftmarker = false;
                m_crccalculus = true;  // Start CRC calculation
            }
            else
            {
//                if (m_okTrace)
//                    DebugLogFormat(_T("Floppy WRITING %06o\r\n"), m_shiftreg);  //DEBUG

                m_pDrive->marker[m_pDrive->dataptr / 2] = false;
            }

            if (m_writeflag)
            {
                m_shiftreg = m_writereg;
                m_shiftflag = m_writeflag;  m_writeflag = false;
                m_shiftmarker = m_writemarker;  m_writemarker = false;
                m_status |= FLOPPY_STATUS_MOREDATA;
            }
            else
            {
                if (m_crccalculus)  // Stop CRC calclation
                {
                    m_shiftreg = 0x4444;  //STUB
                    m_shiftflag = true;
                    m_shiftmarker = false;
                    m_crccalculus = false;
                    m_status |= FLOPPY_STATUS_CHECKSUMOK;
                }
            }
        }
    }
}

// Read track data from file and fill m_data
void CFloppyController::PrepareTrack()
{
    FlushChanges();

    if (m_pDrive == nullptr) return;

    if (m_okTrace)
        DebugLogFormat(_T("Floppy Prepare Track\tTRACK %d SIDE %d\r\n"), m_track, m_side);

    uint32_t count;

    m_trackchanged = false;
    m_status |= FLOPPY_STATUS_MOREDATA;
    //NOTE: Not changing m_pDrive->dataptr
    m_pDrive->datatrack = m_track;
    m_pDrive->dataside = m_side;

    // Track has 10 sectors, 512 bytes each; offset of the file is === ((Track<<1)+SIDE)*5120
    long foffset = ((m_track * 2) + (m_side)) * 5120;

    uint8_t data[5120];
    memset(data, 0, 5120);

    if (m_pDrive->fpFile != nullptr)
    {
        ::fseek(m_pDrive->fpFile, foffset, SEEK_SET);
        count = (uint32_t) ::fread(data, 1, 5120, m_pDrive->fpFile);
        //TODO: Контроль ошибок чтения
    }

    // Fill m_data array and m_marker array with marked data
    EncodeTrackData(data, m_pDrive->data, m_pDrive->marker, m_track, m_side);

    ////DEBUG: Test DecodeTrackData()
    //uint8_t data2[5120];
    //bool parsed = DecodeTrackData(m_pDrive->data, data2);
    //ASSERT(parsed);
    //bool tested = true;
    //for (int i = 0; i < 5120; i++)
    //    if (data[i] != data2[i])
    //    {
    //        tested = false;
    //        break;
    //    }
    //ASSERT(tested);
}

void CFloppyController::FlushChanges()
{
    if (m_drive == -1) return;
    if (!IsAttached(m_drive)) return;
    if (!m_trackchanged) return;

//    DebugLog(_T("Floppy FLUSH\r\n"));  //DEBUG

    // Decode track data from m_data
    uint8_t data[5120];  memset(data, 0, 5120);
    bool decoded = DecodeTrackData(m_pDrive->data, data);

    if (decoded)  // Write to the file only if the track was correctly decoded from raw data
    {
        // Track has 10 sectors, 512 bytes each; offset of the file is === ((Track<<1)+SIDE)*5120
        long foffset = ((m_pDrive->datatrack * 2) + (m_pDrive->dataside)) * 5120;

        // Check file length
        ::fseek(m_pDrive->fpFile, 0, SEEK_END);
        size_t currentFileSize = ::ftell(m_pDrive->fpFile);
        while (currentFileSize < (size_t)(foffset + 5120))
        {
            uint8_t datafill[512];  ::memset(datafill, 0, 512);
            size_t bytesToWrite = ((size_t)(foffset + 5120) - currentFileSize) % 512;
            if (bytesToWrite == 0) bytesToWrite = 512;
            ::fwrite(datafill, 1, bytesToWrite, m_pDrive->fpFile);
            //TODO: Проверка на ошибки записи
            currentFileSize += bytesToWrite;
        }

        // Save data into the file
        ::fseek(m_pDrive->fpFile, foffset, SEEK_SET);
        uint32_t dwBytesWritten = (uint32_t) ::fwrite(data, 1, 5120, m_pDrive->fpFile);
        //TODO: Проверка на ошибки записи
    }
    else
    {
        DebugLog(_T("Floppy FLUSH FAILED\r\n"));
    }

    m_trackchanged = false;

    ////DEBUG: Save raw m_data/m_marker into rawdata.bin
    //HANDLE hRawFile = CreateFile(_T("rawdata.bin"),
    //            GENERIC_WRITE, FILE_SHARE_READ, NULL,
    //            CREATE_ALWAYS, 0, NULL);
}


//////////////////////////////////////////////////////////////////////


uint16_t getCrc(const uint8_t *ptr, size_t len)
{
    int crc = 0xffff;
    while (len--)
    {
        uint8_t val = *ptr++;
        crc ^= val << 8;
        for (int i = 0; i < 8; ++i)
        {
            if ((crc <<= 1) & 0x10000)
                crc ^= 0x1021;
        }
    }
    return (crc & 0xffff);
}

// Fill data array and marker array with marked data
static void EncodeTrackData(const uint8_t* pSrc, uint8_t* data, uint8_t* marker, uint16_t track, uint16_t side)
{
    memset(data, 0, FLOPPY_RAWTRACKSIZE);
    memset(marker, 0, FLOPPY_RAWMARKERSIZE);
    uint32_t count;
    size_t ptr = 0;

    int gap = 42;  // GAP4a + GAP1 length
    for (uint8_t sect = 0; sect < 10; sect++)
    {
        // GAP
        for (count = 0; count < (uint32_t) gap; count++) data[ptr++] = 0x4e;
        // sector header
        for (count = 0; count < 12; count++) data[ptr++] = 0x00;
        // marker
        marker[ptr / 2] = true;  // ID marker; start CRC calculus
        uint8_t *pCrcPtr = data + ptr;  size_t nCrcPtr = ptr;
        data[ptr++] = 0xa1;  data[ptr++] = 0xa1;  data[ptr++] = 0xa1;
        data[ptr++] = 0xfe;

        data[ptr++] = (uint8_t) track;  data[ptr++] = (side == 0) ? 0 : 1;
        data[ptr++] = sect + 1;  data[ptr++] = 2; // Assume 512 bytes per sector;
        // crc
        uint16_t crc = getCrc(pCrcPtr, ptr - nCrcPtr);
        data[ptr++] = (uint8_t)(crc & 0xff);  data[ptr++] = (uint8_t)(crc >> 8);

        // sync
        for (count = 0; count < 22; count++) data[ptr++] = 0x4e;
        // data header
        for (count = 0; count < 12; count++) data[ptr++] = 0x00;
        // marker
        marker[ptr / 2] = true;  // Data marker; start CRC calculus
        pCrcPtr = data + ptr;  nCrcPtr = ptr;
        data[ptr++] = 0xa1;  data[ptr++] = 0xa1;  data[ptr++] = 0xa1;
        data[ptr++] = 0xfb;
        // data
        for (count = 0; count < 512; count++)
            data[ptr++] = pSrc[(sect * 512) + count];
        // crc
        crc = getCrc(pCrcPtr, ptr - nCrcPtr);
        data[ptr++] = (uint8_t)(crc & 0xff);  data[ptr++] = (uint8_t)(crc >> 8);

        gap = 36;  // GAP3 length
    }

    // fill GAP4B to the end of the track
    while (ptr < FLOPPY_RAWTRACKSIZE) data[ptr++] = 0x4e;
}

// Decode track data from raw data
// pRaw is array of FLOPPY_RAWTRACKSIZE bytes
// pDest is array of 5120 bytes
// Returns: true - decoded, false - parse error
static bool DecodeTrackData(const uint8_t* pRaw, uint8_t* pDest)
{
    uint16_t dataptr = 0;  // Offset in m_data array
    uint16_t destptr = 0;  // Offset in data array
    for (;;)
    {
        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x4e) dataptr++;  // Skip GAP1 or GAP3
        if (dataptr >= FLOPPY_RAWTRACKSIZE) break;  // End of track or error
        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x00) dataptr++;  // Skip sync
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong

        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        if (pRaw[dataptr++] != 0xfe) return false;  // Marker not found

        uint8_t sectcyl, secthd, sectsec, sectno = 0;
        if (dataptr < FLOPPY_RAWTRACKSIZE) sectcyl = pRaw[dataptr++];
        if (dataptr < FLOPPY_RAWTRACKSIZE) secthd  = pRaw[dataptr++];
        if (dataptr < FLOPPY_RAWTRACKSIZE) sectsec = pRaw[dataptr++];
        if (dataptr < FLOPPY_RAWTRACKSIZE) sectno  = pRaw[dataptr++];
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong

        int sectorsize;
        if (sectno == 1) sectorsize = 256;
        else if (sectno == 2) sectorsize = 512;
        else if (sectno == 3) sectorsize = 1024;
        else return false;  // Something wrong: unknown sector size
        // crc
        if (dataptr < FLOPPY_RAWTRACKSIZE) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE) dataptr++;

        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x4e) dataptr++;  // Skip GAP2
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x00) dataptr++;  // Skip sync
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong

        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        if (pRaw[dataptr++] != 0xfb) return false;  // Marker not found

        for (int count = 0; count < sectorsize; count++)  // Copy sector data
        {
            if (destptr >= 5120) break;  // End of track or error
            pDest[destptr++] = pRaw[dataptr++];
            if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        }
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        // crc
        if (dataptr < FLOPPY_RAWTRACKSIZE) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE) dataptr++;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
