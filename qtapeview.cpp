#include "stdafx.h"
#include "qtapeview.h"
#include <QLabel>
#include <QPushButton>

QTapeView::QTapeView(QWidget *parent) :
        QWidget(parent)
{
    setMinimumSize(512, 52);

    int buttonWidth = 85;

    m_labelFile = new QLabel(this);
    m_labelFile->move(8, 4);
    m_labelFile->resize(500, 18);
    m_labelTotal = new QLabel(this);
    m_labelTotal->move(8, 26);
    m_labelTotal->resize(100, 18);
    m_labelCurrent = new QLabel(this);
    m_buttonPlay = new QPushButton(_T("Play"), this);
    m_buttonPlay->move(8 + 100 + 16, 24);
    m_buttonPlay->resize(buttonWidth, 22);
    m_buttonRewind = new QPushButton(_T("<< Rewind"), this);
    m_buttonRewind->move(8 + 100 + 16 + 4 + buttonWidth, 24);
    m_buttonRewind->resize(buttonWidth, 22);
    m_buttonOpen = new QPushButton(_T("Open WAV"), this);
    m_buttonOpen->move(512 - buttonWidth - 4 - buttonWidth - 8, 24);
    m_buttonOpen->resize(buttonWidth, 22);
    m_buttonSave = new QPushButton(_T("Save WAV"), this);
    m_buttonSave->move(512 - buttonWidth - 8, 24);
    m_buttonSave->resize(buttonWidth, 22);

    QObject::connect(m_buttonPlay, SIGNAL(clicked()), this, SLOT(tapePlay()));
    QObject::connect(m_buttonRewind, SIGNAL(clicked()), this, SLOT(tapeRewind()));
    QObject::connect(m_buttonOpen, SIGNAL(clicked()), this, SLOT(tapeOpen()));
    QObject::connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(tapeSave()));
}

void QTapeView::tapePlay()
{
    //TODO
}

void QTapeView::tapeRewind()
{
    //TODO
}

void QTapeView::tapeOpen()
{
    //TODO
}

void QTapeView::tapeSave()
{
    //TODO
}
