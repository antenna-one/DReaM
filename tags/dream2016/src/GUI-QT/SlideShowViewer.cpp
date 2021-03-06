/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *   Julian Cable, David Flamand
 *
 * Description: SlideShow Viewer
 *
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "SlideShowViewer.h"
#include "../util-QT/Util.h"
#include "../datadecoding/DABMOT.h"
#include "../datadecoding/DataDecoder.h"
#include <QFileDialog>
#include "ThemeCustomizer.h"

SlideShowViewer::SlideShowViewer(CSettings& Settings, QWidget* parent):
    CWindow(parent, Settings, "SlideShow"),
    vecImages(), vecImageNames(), iCurImagePos(-1),
    bClearMOTCache(false),decoder(NULL)
{
    setupUi(this);

    string p = Settings.Get(
                   "Receiver", "datafilesdirectory", string(DEFAULT_DATA_FILES_DIRECTORY));

    strCurrentSavePath = QString::fromUtf8((p+PATH_SEPARATOR+"MOT").c_str());

    /* Update time for color LED */
    LEDStatus->SetUpdateTime(1000);

    /* Connect controls */
    connect(ButtonStepBack, SIGNAL(clicked()), this, SLOT(OnButtonStepBack()));
    connect(ButtonStepForward, SIGNAL(clicked()), this, SLOT(OnButtonStepForward()));
    connect(ButtonJumpBegin, SIGNAL(clicked()), this, SLOT(OnButtonJumpBegin()));
    connect(ButtonJumpEnd, SIGNAL(clicked()), this, SLOT(OnButtonJumpEnd()));
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(close()));
    connect(actionClear_All, SIGNAL(triggered()), SLOT(OnClearAll()));
    connect(actionSave, SIGNAL(triggered()), SLOT(OnSave()));
    connect(actionSave_All, SIGNAL(triggered()), SLOT(OnSaveAll()));
    connect(actionClose, SIGNAL(triggered()), SLOT(close()));
    connect(&Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));

    OnClearAll();

    APPLY_CUSTOM_THEME();
}

SlideShowViewer::~SlideShowViewer()
{
}

void SlideShowViewer::OnTimer()
{
    if(decoder==NULL)
        return;

    if (bClearMOTCache)
    {
        bClearMOTCache = false;
        CMOTObject  NewObj;
        while (decoder->GetMOTObject(NewObj, CDataDecoder::AT_MOTSLIDESHOW))
        {
        }
    }

    CMOTObject NewObj;

    /* Poll the data decoder module for new object */
    if (decoder->GetMOTObject(NewObj, CDataDecoder::AT_MOTSLIDESHOW))
    {
        /* Store received picture */
        int iCurNumPict = vecImageNames.size();
        CVector<_BYTE>& imagedata = NewObj.Body.vecData;

        /* Load picture in QT format */
        QPixmap pic;
        if (pic.loadFromData(&imagedata[0], imagedata.size()))
        {
            /* Set new picture in source factory */
            vecImages.push_back(pic);
            vecImageNames.push_back(NewObj.strName.c_str());
        }

        /* If the last received picture was selected, automatically show
           new picture */
        if (iCurImagePos == iCurNumPict - 1)
            SetImage(iCurNumPict);
        else
            UpdateButtons();
    }
}

void SlideShowViewer::OnButtonStepBack()
{
    SetImage(iCurImagePos-1);
}

void SlideShowViewer::OnButtonStepForward()
{
    SetImage(iCurImagePos+1);
}

void SlideShowViewer::OnButtonJumpBegin()
{
    SetImage(0);
}

void SlideShowViewer::OnButtonJumpEnd()
{
    SetImage(vecImages.size()-1);
}

void SlideShowViewer::OnSave()
{
    /* Create directory for storing the file (if not exists) */
    CreateDirectories(strCurrentSavePath);

    QString strFilename = strCurrentSavePath + VerifyFilename(vecImageNames[iCurImagePos]);
    strFilename = QFileDialog::getSaveFileName(this,
                  tr("Save File"), strFilename, tr("Images (*.png *.jpg)"));

    /* Check if user not hit the cancel button */
    if (!strFilename.isEmpty())
    {
        vecImages[iCurImagePos].save(strFilename);

        strCurrentSavePath = QFileInfo(strFilename).path() + PATH_SEPARATOR;
    }
}

void SlideShowViewer::OnSaveAll()
{
    /* Create directory for storing the files (if not exists) */
    CreateDirectories(strCurrentSavePath);

    QString strDirectory = QFileDialog::getExistingDirectory(this,
                           tr("Open Directory"), strCurrentSavePath);

    /* Check if user not hit the cancel button */
    if (!strDirectory.isEmpty())
    {
        strCurrentSavePath = strDirectory + PATH_SEPARATOR;

        for(size_t i=0; i<vecImages.size(); i++)
            vecImages[i].save(strCurrentSavePath + VerifyFilename(vecImageNames[i]));
    }
}

void SlideShowViewer::OnClearAll()
{
    vecImages.clear();
    vecImageNames.clear();
    iCurImagePos = -1;
    UpdateButtons();
    LabelTitle->setText("");
    bClearMOTCache = true;
}

void SlideShowViewer::eventShow(QShowEvent*)
{
    /* Update window */
    OnTimer();

    /* Activate real-time timer when window is shown */
    Timer.start(GUI_CONTROL_UPDATE_TIME);
}

void SlideShowViewer::eventHide(QHideEvent*)
{
    /* Deactivate real-time timer so that it does not get new pictures */
    Timer.stop();
}

void SlideShowViewer::SetImage(int pos)
{
    if(vecImages.size()==0)
        return;
    if(pos<0)
        pos = 0;
    if(pos>int(vecImages.size()-1))
        pos = vecImages.size()-1;
    iCurImagePos = pos;
    Image->setPixmap(vecImages[pos]);
    QString imagename = vecImageNames[pos];
    Image->setToolTip(imagename);
    imagename =  "<b>" + imagename + "</b>";
    Linkify(imagename);
    LabelTitle->setText(imagename);
    UpdateButtons();
}

void SlideShowViewer::UpdateButtons()
{
    /* Set enable menu entry for saving a picture */
    if (iCurImagePos < 0)
    {
        actionClear_All->setEnabled(false);
        actionSave->setEnabled(false);
        actionSave_All->setEnabled(false);
    }
    else
    {
        actionClear_All->setEnabled(true);
        actionSave->setEnabled(true);
        actionSave_All->setEnabled(true);
    }

    if (iCurImagePos <= 0)
    {
        /* We are already at the beginning */
        ButtonStepBack->setEnabled(false);
        ButtonJumpBegin->setEnabled(false);
    }
    else
    {
        ButtonStepBack->setEnabled(true);
        ButtonJumpBegin->setEnabled(true);
    }

    if (iCurImagePos == int(vecImages.size()-1))
    {
        /* We are already at the end */
        ButtonStepForward->setEnabled(false);
        ButtonJumpEnd->setEnabled(false);
    }
    else
    {
        ButtonStepForward->setEnabled(true);
        ButtonJumpEnd->setEnabled(true);
    }

    QString strTotImages = QString().setNum(vecImages.size());
    QString strNumImage = QString().setNum(iCurImagePos + 1);

    QString strSep("");

    for (int i = 0; i < (strTotImages.length() - strNumImage.length()); i++)
        strSep += " ";

    LabelCurPicNum->setText(strSep + strNumImage + PATH_SEPARATOR + strTotImages);

    /* If no picture was received, show the following text */
    if (iCurImagePos < 0)
    {
        /* Init text browser window */
        Image->setText("<center>" + tr("MOT Slideshow Viewer") + "</center>");
        Image->setToolTip("");
    }
}

void SlideShowViewer::setStatus(int i, ETypeRxStatus s)
{
    if(i==short_id)
        SetStatus(LEDStatus, s);
}

void SlideShowViewer::setServiceInformation(int i, CService s)
{
    short_id = i;
    service = s;
    decoder = service.DataParam.pDecoder;
    QString strTitle("MOT Slide Show");
    QString strServiceID;
    QString strLabel = QString::fromUtf8(service.strLabel.c_str());
    if (service.iServiceID != 0)
    {
        if (strLabel != "")
            strLabel += " - ";

        /* Service ID (plot number in hexadecimal format) */
        strServiceID = "ID:" +
                       QString().setNum(service.iServiceID, 16).toUpper();
    }

    /* Add the description on the title of the dialog */
    if (strLabel != "" || strServiceID != "")
        strTitle += " [" + strLabel + strServiceID + "]";
    setWindowTitle(strTitle);
}
