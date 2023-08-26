﻿#include "stdafx.h"
#include "util/WavPcmFile.h"
#include "qtapeview.h"
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include "Emulator.h"

//////////////////////////////////////////////////////////////////////

QTapeView* g_tapeView = nullptr;

bool CALLBACK TapeView_TapeReadCallback(unsigned int samples)
{
    return g_tapeView->tapeReadCallback(samples);
}
void CALLBACK TapeView_TapeWriteCallback(int value, unsigned int samples)
{
    g_tapeView->tapeWriteCallback(value, samples);
}

//////////////////////////////////////////////////////////////////////

QTapeView::QTapeView(QWidget *parent) :
    QWidget(parent)
{
    g_tapeView = this;

    m_okTapeInserted = false;
    m_okTapePlaying = false;
    m_okTapeRecording = false;
    m_hTapeWavPcmFile = INVALID_HANDLE_VALUE;
    m_dwTapePositionShown = 0;

    setMinimumSize(570, 52);

    int buttonWidth = 85;
    int viewWidth = 570;
    int cxLabelFile = 450;

    m_labelFile = new QLabel(this);
    m_labelFile->move(8, 4);
    m_labelFile->resize(cxLabelFile, 18);
    m_labelTotal = new QLabel(this);
    m_labelTotal->move(cxLabelFile + 8 + 4, 4);
    m_labelTotal->resize(viewWidth - 8 * 2 - cxLabelFile - 4, 18);
    m_labelCurrent = new QLabel(this);
    m_labelCurrent->move(8, 26);
    m_labelCurrent->resize(100, 18);
    m_buttonPlay = new QPushButton(tr("Play"), this);
    m_buttonPlay->move(8 + 100 + 16, 24);
    m_buttonPlay->resize(buttonWidth, 22);
    m_buttonPlay->setEnabled(false);
    m_buttonRewind = new QPushButton(tr("<< Rewind"), this);
    m_buttonRewind->move(8 + 100 + 16 + 4 + buttonWidth, 24);
    m_buttonRewind->resize(buttonWidth, 22);
    m_buttonRewind->setEnabled(false);
    m_buttonOpen = new QPushButton(tr("Open WAV"), this);
    m_buttonOpen->move(viewWidth - buttonWidth - 4 - buttonWidth - 8, 24);
    m_buttonOpen->resize(buttonWidth, 22);
    m_buttonSave = new QPushButton(tr("Save WAV"), this);
    m_buttonSave->move(viewWidth - buttonWidth - 8, 24);
    m_buttonSave->resize(buttonWidth, 22);

    QObject::connect(m_buttonPlay, SIGNAL(clicked()), this, SLOT(doPlayStop()));
    QObject::connect(m_buttonRewind, SIGNAL(clicked()), this, SLOT(doRewind()));
    QObject::connect(m_buttonOpen, SIGNAL(clicked()), this, SLOT(doTapeOpen()));
    QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(doTapeSave()));
}

void QTapeView::doPlayStop()
{
    if (m_okTapePlaying)
        stopTape();
    else
        playTape();
}

void QTapeView::doRewind()
{
    if (!m_okTapeInserted) return;

    WavPcmFile_SetPosition((HWAVPCMFILE)m_hTapeWavPcmFile, 0);

    updatePosition();
}

void QTapeView::doTapeOpen()
{
    if (m_okTapeInserted)
    {
        this->closeTape();
        return;
    }

    // File Open dialog
    QFileDialog dlg;
    dlg.setNameFilter(tr("WAV files (*.wav)|All Files (*.*)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    this->openTape(strFileName);
}

void QTapeView::doTapeSave()
{
    if (m_okTapeInserted)
    {
        this->closeTape();
        return;
    }

    // File Save dialog
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("WAV files (*.wav)|All Files (*.*)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    this->createTape(strFileName);
}

void QTapeView::createTape(const QString &sFileName)
{
    const char * lpszFile = qPrintable(sFileName);
    m_hTapeWavPcmFile = WavPcmFile_Create(lpszFile, 44100);
    if (m_hTapeWavPcmFile == INVALID_HANDLE_VALUE)
        return;  //TODO: Report error

    m_sTapeFile = sFileName;
    m_okTapeInserted = true;
    m_okTapeRecording = true;

    m_buttonPlay->setEnabled(true);
    m_buttonPlay->setText(tr("Record"));
    m_buttonRewind->setEnabled(true);
    m_labelFile->setText(lpszFile);

    this->updatePosition();

    m_buttonSave->setText(tr("Close WAV"));
    m_buttonOpen->setEnabled(false);
}

void QTapeView::openTape(const QString &sFileName)
{
    const char * lpszFile = qPrintable(sFileName);
    m_hTapeWavPcmFile = WavPcmFile_Open(lpszFile);
    if (m_hTapeWavPcmFile == INVALID_HANDLE_VALUE)
        return;  //TODO: Report about a bad WAV file

    m_sTapeFile = sFileName;
    m_okTapeInserted = true;
    m_okTapeRecording = false;

    m_buttonPlay->setEnabled(true);
    m_buttonPlay->setText(tr("Play"));
    m_buttonRewind->setEnabled(true);
    m_labelFile->setText(lpszFile);

    this->updatePosition();

    quint32 wavLength = WavPcmFile_GetLength((HWAVPCMFILE)m_hTapeWavPcmFile);
    int wavFreq = WavPcmFile_GetFrequency((HWAVPCMFILE)m_hTapeWavPcmFile);
    double wavLengthSeconds = double(wavLength) / wavFreq;

    char buffer[64];
    _snprintf(buffer, 64, "%d:%02d.%02d, %d Hz",
            int(wavLengthSeconds) / 60, int(wavLengthSeconds) % 60, int(wavLengthSeconds * 100) % 100, wavFreq);
    m_labelTotal->setText(buffer);

    m_buttonOpen->setText(tr("Close WAV"));
    m_buttonSave->setEnabled(false);
}

void QTapeView::closeTape()
{
    // Stop tape playback
    this->stopTape();

    WavPcmFile_Close((HWAVPCMFILE)m_hTapeWavPcmFile);
    m_hTapeWavPcmFile = INVALID_HANDLE_VALUE;

    m_okTapeInserted = false;

    m_buttonPlay->setEnabled(false);
    m_buttonRewind->setEnabled(false);
    m_buttonOpen->setEnabled(true);
    m_buttonSave->setEnabled(true);
    m_labelFile->setText(nullptr);
    m_labelTotal->setText(nullptr);
    m_labelCurrent->setText(nullptr);
    m_buttonOpen->setText(tr("Open WAV"));
    m_buttonSave->setText(tr("Save WAV"));
}

void QTapeView::playTape()
{
    if (m_okTapePlaying) return;
    if (!m_okTapeInserted) return;

    int sampleRate = WavPcmFile_GetFrequency((HWAVPCMFILE)m_hTapeWavPcmFile);

    if (m_okTapeRecording)
        g_pBoard->SetTapeWriteCallback(TapeView_TapeWriteCallback, sampleRate);
    else
        g_pBoard->SetTapeReadCallback(TapeView_TapeReadCallback, sampleRate);

    m_okTapePlaying = true;

    m_buttonPlay->setText(tr("Stop"));
}

void QTapeView::stopTape()
{
    if (!m_okTapePlaying) return;

    if (m_okTapeRecording)
        g_pBoard->SetTapeWriteCallback(nullptr, 0);
    else
        g_pBoard->SetTapeReadCallback(nullptr, 0);

    m_okTapePlaying = false;

    m_buttonPlay->setText(m_okTapeRecording ? tr("Record") : tr("Play"));
}

void QTapeView::updatePosition()
{
    quint32 wavPos = WavPcmFile_GetPosition((HWAVPCMFILE)m_hTapeWavPcmFile);
    int wavFreq = WavPcmFile_GetFrequency((HWAVPCMFILE)m_hTapeWavPcmFile);
    double wavPosSeconds = double(wavPos) / wavFreq;
    char buffer[64];
    _snprintf(buffer, 64, "%d:%02d.%02d",
            int(wavPosSeconds) / 60, int(wavPosSeconds) % 60, int(wavPosSeconds * 100) % 100);
    m_labelCurrent->setText(buffer);

    m_dwTapePositionShown = wavPos;
}


bool QTapeView::tapeReadCallback(unsigned int samples)
{
    if (m_hTapeWavPcmFile == (HWAVPCMFILE)INVALID_HANDLE_VALUE) return 0;
    if (m_okTapeRecording) return 0;
    if (samples == 0) return 0;

    unsigned int value = 0;
    for (unsigned int i = 0; i < samples; i++)
    {
        value = WavPcmFile_ReadOne((HWAVPCMFILE)m_hTapeWavPcmFile);
    }
    bool result = (value > 0xffffffff / 2);

    quint32 wavLength = WavPcmFile_GetLength((HWAVPCMFILE)m_hTapeWavPcmFile);
    quint32 wavPos = WavPcmFile_GetPosition((HWAVPCMFILE)m_hTapeWavPcmFile);
    if (wavPos >= wavLength)  // End of tape
        this->stopTape();

    int wavFreq = WavPcmFile_GetFrequency((HWAVPCMFILE)m_hTapeWavPcmFile);
    if (wavPos - m_dwTapePositionShown > (quint32)(wavFreq / 6) || !m_okTapePlaying)
    {
        this->updatePosition();
    }

    return result;
}

void QTapeView::tapeWriteCallback(int value, unsigned int samples)
{
    if (m_hTapeWavPcmFile == (HWAVPCMFILE)INVALID_HANDLE_VALUE) return;
    if (!m_okTapeRecording) return;
    if (samples == 0) return;

    // Write samples to the file
    for (unsigned int i = 0; i < samples; i++)
        WavPcmFile_WriteOne((HWAVPCMFILE)m_hTapeWavPcmFile, value);

    quint32 wavPos = WavPcmFile_GetPosition((HWAVPCMFILE)m_hTapeWavPcmFile);
    int wavFreq = WavPcmFile_GetFrequency((HWAVPCMFILE)m_hTapeWavPcmFile);
    if (wavPos - m_dwTapePositionShown > (quint32)(wavFreq / 6) || !m_okTapePlaying)
    {
        this->updatePosition();
    }
}
