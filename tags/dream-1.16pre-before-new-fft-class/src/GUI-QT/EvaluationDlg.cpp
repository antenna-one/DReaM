/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
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

#include "EvaluationDlg.h"
#include "DialogUtil.h"
#include "Rig.h"
#include <qmessagebox.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <QHideEvent>
#include <QShowEvent>

/* Implementation *************************************************************/
systemevalDlg::systemevalDlg(CDRMReceiver& NDRMR, CSettings& NSettings,
                             QWidget* parent, const char* name, bool modal, Qt::WFlags f) :
    systemevalDlgBase(parent, name, modal, f),
    DRMReceiver(NDRMR),
    Settings(NSettings),
    Timer(), TimerInterDigit(),
    eNewCharType(CDRMPlot::NONE_OLD)
{
    /* Enable minimize and maximize box for QDialog */
    setWindowFlags(Qt::Window);

    /* Get window geometry data and apply it */
    CWinGeom s;
    Settings.Get("System Evaluation Dialog", s);
    const QRect WinGeom(s.iXPos, s.iYPos, s.iWSize, s.iHSize);

    if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
        setGeometry(WinGeom);

    /* Set help text for the controls */
    AddWhatsThisHelp();

    MainPlot = new CDRMPlot(plot);

    /* Init controls -------------------------------------------------------- */
    /* Init main plot */
    int iPlotStyle = Settings.Get("System Evaluation Dialog", "plotstyle", 0);
    Settings.Put("System Evaluation Dialog", "plotstyle", iPlotStyle);
    MainPlot->SetRecObj(&DRMReceiver);
    MainPlot->SetPlotStyle(iPlotStyle);

    /* Init slider control */
    SliderNoOfIterations->setRange(0, 4);
    SliderNoOfIterations->
    setValue(DRMReceiver.GetMSCMLC()->GetInitNumIterations());
    TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
                                 QString().setNum(DRMReceiver.GetMSCMLC()->GetInitNumIterations()));

    /* Update times for colour LEDs */
    LEDFAC->SetUpdateTime(1500);
    LEDSDC->SetUpdateTime(1500);
    LEDMSC->SetUpdateTime(600);
    LEDFrameSync->SetUpdateTime(600);
    LEDTimeSync->SetUpdateTime(600);
    LEDIOInterface->SetUpdateTime(2000); /* extra long -> red light stays long */

    /* Init parameter for frequency edit for log file */
    EdtFrequency->setText(QString().setNum(DRMReceiver.GetFrequency()));

    /* Update controls */
    UpdateControls();

    /* Set the Char Type of each selectable item */
    QTreeWidgetItemIterator it(chartSelector, QTreeWidgetItemIterator::NoChildren);
    for (; *it; it++)
    {
        if ((*it)->text(0) == tr("SNR Spectrum"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::SNR_SPECTRUM);
        if ((*it)->text(0) == tr("Audio Spectrum"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::AUDIO_SPECTRUM);
        if ((*it)->text(0) == tr("Shifted PSD"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::POWER_SPEC_DENSITY);
        if ((*it)->text(0) == tr("Waterfall Input Spectrum"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::INP_SPEC_WATERF);
        if ((*it)->text(0) == tr("Input Spectrum"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::INPUTSPECTRUM_NO_AV);
        if ((*it)->text(0) == tr("Input PSD"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::INPUT_SIG_PSD);
        if ((*it)->text(0) == tr("MSC"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::MSC_CONSTELLATION);
        if ((*it)->text(0) == tr("SDC"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::SDC_CONSTELLATION);
        if ((*it)->text(0) == tr("FAC"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::FAC_CONSTELLATION);
        if ((*it)->text(0) == tr("FAC / SDC / MSC"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::ALL_CONSTELLATION);
        if ((*it)->text(0) == tr("Frequency / Sample Rate"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::FREQ_SAM_OFFS_HIST);
        if ((*it)->text(0) == tr("Delay / Doppler"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::DOPPLER_DELAY_HIST);
        if ((*it)->text(0) == tr("SNR / Audio"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::SNR_AUDIO_HIST);
        if ((*it)->text(0) == tr("Transfer Function"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::TRANSFERFUNCTION);
        if ((*it)->text(0) == tr("Impulse Response"))
            (*it)->setData(0,  Qt::UserRole, CDRMPlot::AVERAGED_IR);
    }

    /* Expand all items */
    chartSelector->expandAll();

    string  plotType = Settings.Get("System Evaluation Dialog", "sysevplottype", string("Audio Spectrum"));
    /* If MDI in is enabled, disable some of the controls and use different
       initialization for the chart and chart selector */
    if (DRMReceiver.GetRSIIn()->GetInEnabled() == TRUE)
    {
        SliderNoOfIterations->setEnabled(FALSE);

        ButtonGroupChanEstFreqInt->setEnabled(FALSE);
        ButtonGroupChanEstTimeInt->setEnabled(FALSE);
        ButtonGroupTimeSyncTrack->setEnabled(FALSE);
        CheckBoxFlipSpec->setEnabled(FALSE);
        EdtFrequency->setText("0");
        EdtFrequency->setEnabled(FALSE);
        GroupBoxInterfRej->setEnabled(FALSE);

        /* Only audio spectrum makes sence for MDI in */
        plotType = "Audio Spectrum";
    }
    QList<QTreeWidgetItem*> pl = chartSelector->findItems(plotType.c_str(), Qt::MatchRecursive);
    if(pl.size()>0)
    {
        QTreeWidgetItem* i = pl.first();
        chartSelector->setCurrentItem(i, 0);
        OnListSelChanged(i, NULL); // TODO why doesn't setCurrentItem send signal ?
    }

    /* Init context menu for tree widget */
    pTreeWidgetContextMenu = new QMenu(tr("Chart Selector context menu"), this);
    pTreeWidgetContextMenu->addAction(tr("&Open in separate window"),
            this, SLOT(OnTreeWidgetContMenu(bool)));

    /* Connect controls ----------------------------------------------------- */
    connect(SliderNoOfIterations, SIGNAL(valueChanged(int)),
            this, SLOT(OnSliderIterChange(int)));

    /* Radio buttons */
    connect(RadioButtonTiLinear, SIGNAL(clicked()),
            this, SLOT(OnRadioTimeLinear()));
    connect(RadioButtonTiWiener, SIGNAL(clicked()),
            this, SLOT(OnRadioTimeWiener()));
    connect(RadioButtonFreqLinear, SIGNAL(clicked()),
            this, SLOT(OnRadioFrequencyLinear()));
    connect(RadioButtonFreqDFT, SIGNAL(clicked()),
            this, SLOT(OnRadioFrequencyDft()));
    connect(RadioButtonFreqWiener, SIGNAL(clicked()),
            this, SLOT(OnRadioFrequencyWiener()));
    connect(RadioButtonTiSyncEnergy, SIGNAL(clicked()),
            this, SLOT(OnRadioTiSyncEnergy()));
    connect(RadioButtonTiSyncFirstPeak, SIGNAL(clicked()),
            this, SLOT(OnRadioTiSyncFirstPeak()));

    /* Char selector list view */
    connect(chartSelector, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(OnListSelChanged( QTreeWidgetItem *, QTreeWidgetItem *)));
	chartSelector->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(chartSelector, SIGNAL(customContextMenuRequested ( const QPoint&  )),
		this, SLOT(OnCustomContextMenuRequested(const QPoint&)));

    /* Buttons */
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(close()));

    /* Check boxes */
    connect(CheckBoxFlipSpec, SIGNAL(clicked()),
            this, SLOT(OnCheckFlipSpectrum()));
    connect(CheckBoxMuteAudio, SIGNAL(clicked()),
            this, SLOT(OnCheckBoxMuteAudio()));
    connect(CheckBoxWriteLog, SIGNAL(clicked()),
            this, SLOT(OnCheckWriteLog()));
    connect(CheckBoxSaveAudioWave, SIGNAL(clicked()),
            this, SLOT(OnCheckSaveAudioWAV()));
    connect(CheckBoxRecFilter, SIGNAL(clicked()),
            this, SLOT(OnCheckRecFilter()));
    connect(CheckBoxModiMetric, SIGNAL(clicked()),
            this, SLOT(OnCheckModiMetric()));
    connect(CheckBoxReverb, SIGNAL(clicked()),
            this, SLOT(OnCheckBoxReverb()));

    /* Timers */
    connect(&Timer, SIGNAL(timeout()),
            this, SLOT(OnTimer()));

    connect(&TimerInterDigit, SIGNAL(timeout()),
            this, SLOT(OnTimerInterDigit()));

    connect(EdtFrequency, SIGNAL(textChanged ( const QString&)),
            this, SLOT(OnFrequencyEdited ( const QString &)));

    /* Start log file flag */
    CheckBoxWriteLog->setChecked(Settings.Get("Logfile", "enablelog", FALSE));
}

systemevalDlg::~systemevalDlg()
{
    if(DRMReceiver.GetWriteData()->GetIsWriteWaveFile())
        DRMReceiver.GetWriteData()->StopWriteWaveFile();
    delete MainPlot;
}

void systemevalDlg::UpdateControls()
{
    /* Slider for MLC number of iterations */
    const int iNumIt = DRMReceiver.GetMSCMLC()->GetInitNumIterations();
    if (SliderNoOfIterations->value() != iNumIt)
    {
        /* Update slider and label */
        SliderNoOfIterations->setValue(iNumIt);
        TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
                                     QString().setNum(iNumIt));
    }

    /* Update for channel estimation and time sync switches */
    switch (DRMReceiver.GetTimeInt())
    {
    case CChannelEstimation::TLINEAR:
        if (!RadioButtonTiLinear->isChecked())
            RadioButtonTiLinear->setChecked(TRUE);
        break;

    case CChannelEstimation::TWIENER:
        if (!RadioButtonTiWiener->isChecked())
            RadioButtonTiWiener->setChecked(TRUE);
        break;
    }

    switch (DRMReceiver.GetFreqInt())
    {
    case CChannelEstimation::FLINEAR:
        if (!RadioButtonFreqLinear->isChecked())
            RadioButtonFreqLinear->setChecked(TRUE);
        break;

    case CChannelEstimation::FDFTFILTER:
        if (!RadioButtonFreqDFT->isChecked())
            RadioButtonFreqDFT->setChecked(TRUE);
        break;

    case CChannelEstimation::FWIENER:
        if (!RadioButtonFreqWiener->isChecked())
            RadioButtonFreqWiener->setChecked(TRUE);
        break;
    }

    switch (DRMReceiver.GetTiSyncTracType())
    {
    case CTimeSyncTrack::TSFIRSTPEAK:
        if (!RadioButtonTiSyncFirstPeak->isChecked())
            RadioButtonTiSyncFirstPeak->setChecked(TRUE);
        break;

    case CTimeSyncTrack::TSENERGY:
        if (!RadioButtonTiSyncEnergy->isChecked())
            RadioButtonTiSyncEnergy->setChecked(TRUE);
        break;
    }

    /* Update settings checkbuttons */
    CheckBoxReverb->setChecked(DRMReceiver.GetAudSorceDec()->GetReverbEffect());
    CheckBoxRecFilter->setChecked(DRMReceiver.GetFreqSyncAcq()->GetRecFilter());
    CheckBoxModiMetric->setChecked(DRMReceiver.GetIntCons());
    CheckBoxMuteAudio->setChecked(DRMReceiver.GetWriteData()->GetMuteAudio());
    CheckBoxFlipSpec->
    setChecked(DRMReceiver.GetReceiveData()->GetFlippedSpectrum());

    CheckBoxSaveAudioWave->
    setChecked(DRMReceiver.GetWriteData()->GetIsWriteWaveFile());


    /* Update frequency edit control (frequency could be changed by
       schedule dialog */
    int iFrequency = DRMReceiver.GetFrequency();
    int iCurFrequency = EdtFrequency->text().toInt();

    if (iFrequency != iCurFrequency)
    {
        EdtFrequency->setText(QString().setNum(iFrequency));
        iCurFrequency = iFrequency;
    }
}

void systemevalDlg::showEvent(QShowEvent* e)
{
	EVENT_FILTER(e);
    /* Restore chart windows */
    const size_t iNumChartWin = Settings.Get("System Evaluation Dialog", "numchartwin", 0);
    for (size_t i = 0; i < iNumChartWin; i++)
    {
        stringstream s;

        /* create the section key for this window */
        s << "Chart Window " << i;

        /* get the chart type */
        const CDRMPlot::ECharType eNewType = (CDRMPlot::ECharType) Settings.Get(s.str(), "type", 0);

        /* get window geometry data */
        CWinGeom c;
        Settings.Get(s.str(), c);
        const QRect WinGeom(c.iXPos, c.iYPos, c.iWSize, c.iHSize);

        /* Open the new chart window */
        CDRMPlot* pNewChartWin = OpenChartWin(eNewType);

        /* and restore its geometry */
        if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
            pNewChartWin->setGeometry(WinGeom);

        /* Add window pointer in vector (needed for closing the windows) */
        vecpDRMPlots.push_back(pNewChartWin);
    }

    /* Update controls */
    UpdateControls();

    /* Activate real-time timer */
    Timer.start(GUI_CONTROL_UPDATE_TIME);
//    setIconSize(QSize(16,16));

#if QT_VERSION >= 0x040000  
    /* Notify the MainPlot of showEvent */
    MainPlot->activate();
#endif
}

void systemevalDlg::hideEvent(QHideEvent* e)
{
	EVENT_FILTER(e);
#if QT_VERSION >= 0x040000  
    /* Notify the MainPlot of hideEvent */
    MainPlot->deactivate();
#endif

    /* Stop the real-time timer */
    Timer.stop();

    /* Store size and position of all additional chart windows */
    int iNumOpenCharts = 0;

    for (size_t i = 0; i < vecpDRMPlots.size(); i++)
    {
        /* Check, if window wasn't closed by the user */
        if (vecpDRMPlots[i]->isVisible())
        {
            stringstream s;
            CWinGeom c;
            const QRect CWGeom = vecpDRMPlots[i]->geometry();

            /* Set parameters */
            c.iXPos = CWGeom.x();
            c.iYPos = CWGeom.y();
            c.iHSize = CWGeom.height();
            c.iWSize = CWGeom.width();

            s << "Chart Window " << iNumOpenCharts;
            Settings.Put(s.str(), c);
            /* Convert plot type into an integer type. TODO: better solution */
            Settings.Put(s.str(), "type", (int) vecpDRMPlots[i]->GetChartType());

            iNumOpenCharts++;
        }
        /* Close window afterwards */
        vecpDRMPlots[i]->close();
    }
    Settings.Put("System Evaluation Dialog", "numchartwin", iNumOpenCharts);

    /* We do not need the pointers anymore, reset vector */
    vecpDRMPlots.clear();

    /* Set window geometry data in DRMReceiver module */
    CWinGeom s;
    QRect WinGeom = geometry();
    s.iXPos = WinGeom.x();
    s.iYPos = WinGeom.y();
    s.iHSize = WinGeom.height();
    s.iWSize = WinGeom.width();
    Settings.Put("System Evaluation Dialog", s);

    /* Store current plot type. Convert plot type into an integer type.  */
    QString ctext = chartSelector->currentItem()->text(0);
    Settings.Put("System Evaluation Dialog", "sysevplottype", ctext.toStdString());
}

void systemevalDlg::OnTimerInterDigit()
{
    TimerInterDigit.stop();
    DRMReceiver.SetFrequency(EdtFrequency->text().toInt());
}

void systemevalDlg::OnFrequencyEdited ( const QString & )
{
    TimerInterDigit.setInterval(100);
}

void systemevalDlg::UpdatePlotStyle(int iPlotStyle)
{
    /* Update chart windows */
    for (size_t i = 0; i < vecpDRMPlots.size(); i++)
        vecpDRMPlots[i]->SetPlotStyle(iPlotStyle);

    /* Update main plot window */
    MainPlot->SetPlotStyle(iPlotStyle);
}

void systemevalDlg::OnTreeWidgetContMenu(bool)
{
    if (eNewCharType != CDRMPlot::NONE_OLD)
    {
        /* Open the new chart */
        vecpDRMPlots.push_back(OpenChartWin(eNewCharType));
        eNewCharType = CDRMPlot::NONE_OLD;
    }
}

void systemevalDlg::OnCustomContextMenuRequested(const QPoint& p)
{
	QModelIndex index = chartSelector->indexAt(p);
    /* Make sure we have a non root item */
    if (index.parent() != QModelIndex())
    {
        /* Popup the context menu */
        eNewCharType = CDRMPlot::ECharType(index.data(Qt::UserRole).toInt());
        pTreeWidgetContextMenu->exec(QCursor::pos());
    }
}

CDRMPlot* systemevalDlg::OpenChartWin(CDRMPlot::ECharType eNewType)
{
    /* Create new chart window */
    CDRMPlot* pNewChartWin = new CDRMPlot(NULL);
    pNewChartWin->setCaption(tr("Chart Window"));

    /* Set plot style*/
    pNewChartWin->SetPlotStyle(Settings.Get("System Evaluation Dialog", "plotstyle", 0));

    /* Set correct icon (use the same as this dialog) */
    const QIcon& icon = this->windowIcon();
    pNewChartWin->setIcon(icon);

    /* Set receiver object and correct chart type */
    pNewChartWin->SetRecObj(&DRMReceiver);
    pNewChartWin->SetupChart(eNewType);

    /* Show new window */
    pNewChartWin->show();

    return pNewChartWin;
}

void systemevalDlg::SetStatus(CMultColorLED* LED, ETypeRxStatus state)
{
    switch(state)
    {
    case NOT_PRESENT:
        LED->Reset(); /* GREY */
        break;

    case CRC_ERROR:
        LED->SetLight(CMultColorLED::RL_RED);
        break;

    case DATA_ERROR:
        LED->SetLight(CMultColorLED::RL_YELLOW);
        break;

    case RX_OK:
        LED->SetLight(CMultColorLED::RL_GREEN);
        break;
    }
}

void systemevalDlg::OnTimer()
{
    CParameter& ReceiverParam = *(DRMReceiver.GetParameters());

    ReceiverParam.Lock();

    if (this->isVisible())
    {

        SetStatus(LEDMSC, ReceiverParam.ReceiveStatus.Audio.GetStatus());
        SetStatus(LEDSDC, ReceiverParam.ReceiveStatus.SDC.GetStatus());
        SetStatus(LEDFAC, ReceiverParam.ReceiveStatus.FAC.GetStatus());
        SetStatus(LEDFrameSync, ReceiverParam.ReceiveStatus.FSync.GetStatus());
        SetStatus(LEDTimeSync, ReceiverParam.ReceiveStatus.TSync.GetStatus());
        SetStatus(LEDIOInterface, ReceiverParam.ReceiveStatus.Interface.GetStatus());

        /* Show SNR if receiver is in tracking mode */
        if (DRMReceiver.GetAcquiState() == AS_WITH_SIGNAL)
        {
            /* Get a consistant snapshot */

            /* We only get SNR from a local DREAM Front-End */
            _REAL rSNR = ReceiverParam.GetSNR();
            if (rSNR >= 0.0)
            {
                /* SNR */
                ValueSNR->setText("<b>" +
                                  QString().setNum(rSNR, 'f', 1) + " dB</b>");
            }
            else
            {
                ValueSNR->setText("<b>---</b>");
            }
            /* We get MER from a local DREAM Front-End or an RSCI input but not an MDI input */
            _REAL rMER = ReceiverParam.rMER;
            if (rMER >= 0.0 )
            {
                ValueMERWMER->setText(QString().
                                      setNum(ReceiverParam.rWMERMSC, 'f', 1) + " dB / "
                                      + QString().setNum(rMER, 'f', 1) + " dB");
            }
            else
            {
                ValueMERWMER->setText("<b>---</b>");
            }

            /* Doppler estimation (assuming Gaussian doppler spectrum) */
            if (ReceiverParam.rSigmaEstimate >= 0.0)
            {
                /* Plot delay and Doppler values */
                ValueWiener->setText(
                    QString().setNum(ReceiverParam.rSigmaEstimate, 'f', 2) + " Hz / "
                    + QString().setNum(ReceiverParam.rMinDelay, 'f', 2) + " ms");
            }
            else
            {
                /* Plot only delay, Doppler not available */
                ValueWiener->setText("--- / "
                                     + QString().setNum(ReceiverParam.rMinDelay, 'f', 2) + " ms");
            }

            /* Sample frequency offset estimation */
            const _REAL rCurSamROffs = ReceiverParam.rResampleOffset;

            /* Display value in [Hz] and [ppm] (parts per million) */
            ValueSampFreqOffset->setText(
                QString().setNum(rCurSamROffs, 'f', 2) + " Hz (" +
                QString().setNum((int) (rCurSamROffs / SOUNDCRD_SAMPLE_RATE * 1e6))
                + " ppm)");

        }
        else
        {
            ValueSNR->setText("<b>---</b>");
            ValueMERWMER->setText("<b>---</b>");
            ValueWiener->setText("--- / ---");
            ValueSampFreqOffset->setText("---");
        }

#ifdef _DEBUG_
        TextFreqOffset->setText("DC: " +
                                QString().setNum(ReceiverParam.
                                        GetDCFrequency(), 'f', 3) + " Hz ");

        /* Metric values */
        ValueFreqOffset->setText(tr("Metrics [dB]: MSC: ") +
                                 QString().setNum(
                                     DRMReceiver.GetMSCMLC()->GetAccMetric(), 'f', 2) +	"\nSDC: " +
                                 QString().setNum(
                                     DRMReceiver.GetSDCMLC()->GetAccMetric(), 'f', 2) +	" / FAC: " +
                                 QString().setNum(
                                     DRMReceiver.GetFACMLC()->GetAccMetric(), 'f', 2));
#else
        /* DC frequency */
        ValueFreqOffset->setText(QString().setNum(
                                     ReceiverParam.GetDCFrequency(), 'f', 2) + " Hz");
#endif

        /* _WIN32 fix because in Visual c++ the GUI files are always compiled even
           if USE_QT_GUI is set or not (problem with MDI in DRMReceiver) */
#ifdef USE_QT_GUI
        /* If MDI in is enabled, do not show any synchronization parameter */
        if (DRMReceiver.GetRSIIn()->GetInEnabled() == TRUE)
        {
            ValueSNR->setText("<b>---</b>");
            if (ReceiverParam.vecrRdelThresholds.GetSize() > 0)
                ValueWiener->setText(QString().setNum(ReceiverParam.rRdop, 'f', 2) + " Hz / "
                                     + QString().setNum(ReceiverParam.vecrRdelIntervals[0], 'f', 2) + " ms ("
                                     + QString().setNum(ReceiverParam.vecrRdelThresholds[0]) + "%)");
            else
                ValueWiener->setText(QString().setNum(ReceiverParam.rRdop, 'f', 2) + " Hz / ---");

            ValueSampFreqOffset->setText("---");
            ValueFreqOffset->setText("---");
        }
#endif


        /* FAC info static ------------------------------------------------------ */
        QString strFACInfo;

        /* Robustness mode #################### */
        strFACInfo = GetRobModeStr() + " / " + GetSpecOccStr();

        FACDRMModeBWL->setText(tr("DRM Mode / Bandwidth:")); /* Label */
        FACDRMModeBWV->setText(strFACInfo); /* Value */


        /* Interleaver Depth #################### */
        switch (ReceiverParam.eSymbolInterlMode)
        {
        case CParameter::SI_LONG:
            strFACInfo = tr("2 s (Long Interleaving)");
            break;

        case CParameter::SI_SHORT:
            strFACInfo = tr("400 ms (Short Interleaving)");
            break;
        }

        FACInterleaverDepthL->setText(tr("Interleaver Depth:")); /* Label */
        FACInterleaverDepthV->setText(strFACInfo); /* Value */


        /* SDC, MSC mode #################### */
        /* SDC */
        switch (ReceiverParam.eSDCCodingScheme)
        {
        case CS_1_SM:
            strFACInfo = "4-QAM / ";
            break;

        case CS_2_SM:
            strFACInfo = "16-QAM / ";
            break;

        default:
            break;
        }

        /* MSC */
        switch (ReceiverParam.eMSCCodingScheme)
        {
        case CS_2_SM:
            strFACInfo += "SM 16-QAM";
            break;

        case CS_3_SM:
            strFACInfo += "SM 64-QAM";
            break;

        case CS_3_HMSYM:
            strFACInfo += "HMsym 64-QAM";
            break;

        case CS_3_HMMIX:
            strFACInfo += "HMmix 64-QAM";
            break;

        default:
            break;
        }

        FACSDCMSCModeL->setText(tr("SDC / MSC Mode:")); /* Label */
        FACSDCMSCModeV->setText(strFACInfo); /* Value */


        /* Code rates #################### */
        strFACInfo = QString().setNum(ReceiverParam.MSCPrLe.iPartB);
        strFACInfo += " / ";
        strFACInfo += QString().setNum(ReceiverParam.MSCPrLe.iPartA);

        FACCodeRateL->setText(tr("Prot. Level (B / A):")); /* Label */
        FACCodeRateV->setText(strFACInfo); /* Value */


        /* Number of services #################### */
        strFACInfo = tr("Audio: ");
        strFACInfo += QString().setNum(ReceiverParam.iNumAudioService);
        strFACInfo += tr(" / Data: ");
        strFACInfo +=QString().setNum(ReceiverParam.iNumDataService);

        FACNumServicesL->setText(tr("Number of Services:")); /* Label */
        FACNumServicesV->setText(strFACInfo); /* Value */


        /* Time, date #################### */
        if ((ReceiverParam.iUTCHour == 0) &&
                (ReceiverParam.iUTCMin == 0) &&
                (ReceiverParam.iDay == 0) &&
                (ReceiverParam.iMonth == 0) &&
                (ReceiverParam.iYear == 0))
        {
            /* No time service available */
            strFACInfo = tr("Service not available");
        }
        else
        {
#ifdef GUI_QT_DATE_TIME_TYPE
            /* QT type of displaying date and time */
            QDateTime DateTime;
            DateTime.setDate(QDate(ReceiverParam.iYear,
                                   ReceiverParam.iMonth,
                                   ReceiverParam.iDay));
            DateTime.setTime(QTime(ReceiverParam.iUTCHour,
                                   ReceiverParam.iUTCMin));

            strFACInfo = DateTime.toString();
#else
            /* Set time and date */
            QString strMin;
            const int iMin = ReceiverParam.iUTCMin;

            /* Add leading zero to number smaller than 10 */
            if (iMin < 10)
                strMin = "0";
            else
                strMin = "";

            strMin += QString().setNum(iMin);

            strFACInfo =
                /* Time */
                QString().setNum(ReceiverParam.iUTCHour) + ":" +
                strMin + "  -  " +
                /* Date */
                QString().setNum(ReceiverParam.iMonth) + "/" +
                QString().setNum(ReceiverParam.iDay) + "/" +
                QString().setNum(ReceiverParam.iYear);
#endif
        }

        FACTimeDateL->setText(tr("Received time - date:")); /* Label */
        FACTimeDateV->setText(strFACInfo); /* Value */

        UpdateGPS(ReceiverParam);

        UpdateControls();
    }
    ReceiverParam.Unlock();
}

void systemevalDlg::UpdateGPS(CParameter& ReceiverParam)
{
    gps_data_t& gps = ReceiverParam.gps_data;

    if((gps.set&STATUS_SET)==0) {
        LEDGPS->SetLight(CMultColorLED::RL_RED);
    } else {

        if(gps.status==0)
            LEDGPS->SetLight(CMultColorLED::RL_YELLOW);
        else
            LEDGPS->SetLight(CMultColorLED::RL_GREEN);
    }

    QString qStrPosition;
    if (gps.set&LATLON_SET)
        qStrPosition = QString(tr("Lat: %1\260  Long: %2\260")).arg(gps.fix.latitude, 0, 'f', 4).arg(gps.fix.longitude,0, 'f',4);
    else
        qStrPosition = tr("Lat: ?  Long: ?");

    QString qStrAltitude;
    if (gps.set&ALTITUDE_SET)
        qStrAltitude = QString(tr("  Alt: %1 m")).arg(gps.fix.altitude, 0, 'f', 0);
    else
        qStrAltitude = tr("  Alt: ?");
    QString qStrSpeed;
    if (gps.set&SPEED_SET)
        qStrSpeed = QString(tr("Speed: %1 m/s")).arg(gps.fix.speed, 0, 'f', 1);
    else
        qStrSpeed = tr("Speed: ?");
    QString qStrTrack;
    if (gps.set&TRACK_SET)
        qStrTrack =  QString(tr("  Track: %1\260")).arg(gps.fix.track);
    else
        qStrTrack =  tr("  Track: ?");
    QString qStrTime;
    if (gps.set&TIME_SET)
    {
        struct tm * p_ts;
        time_t tt = time_t(gps.fix.time);
        p_ts = gmtime(&tt);
        QChar fill('0');
        qStrTime = QString("UTC: %1/%2/%3 %4:%5:%6  ")
                .arg(1900 + p_ts->tm_year)
                .arg(1 + p_ts->tm_mon, 2, 10, fill)
                .arg(p_ts->tm_mday, 2, 10, fill)
                .arg(p_ts->tm_hour, 2, 10, fill)
                .arg(p_ts->tm_min, 2, 10, fill)
                .arg(p_ts->tm_sec,2, 10, fill);
    }
    else
	qStrTime = "UTC: ?";
    QString qStrSat;
    if (gps.set&SATELLITE_SET)
        qStrSat = tr("Satellites: ") + QString().setNum(gps.satellites_used);
    else
        qStrSat = tr("Satellites: ?");

    TextLabelGPSPosition->setText(qStrPosition+qStrAltitude);
    TextLabelGPSSpeedHeading->setText(qStrSpeed+qStrTrack);
    TextLabelGPSTime->setText(qStrTime+qStrSat);
}

void systemevalDlg::OnRadioTimeLinear()
{
    if (DRMReceiver.GetTimeInt() != CChannelEstimation::TLINEAR)
        DRMReceiver.SetTimeInt(CChannelEstimation::TLINEAR);
}

void systemevalDlg::OnRadioTimeWiener()
{
    if (DRMReceiver.GetTimeInt() != CChannelEstimation::TWIENER)
        DRMReceiver.SetTimeInt(CChannelEstimation::TWIENER);
}

void systemevalDlg::OnRadioFrequencyLinear()
{
    if (DRMReceiver.GetFreqInt() != CChannelEstimation::FLINEAR)
        DRMReceiver.SetFreqInt(CChannelEstimation::FLINEAR);
}

void systemevalDlg::OnRadioFrequencyDft()
{
    if (DRMReceiver.GetFreqInt() != CChannelEstimation::FDFTFILTER)
        DRMReceiver.SetFreqInt(CChannelEstimation::FDFTFILTER);
}

void systemevalDlg::OnRadioFrequencyWiener()
{
    if (DRMReceiver.GetFreqInt() != CChannelEstimation::FWIENER)
        DRMReceiver.SetFreqInt(CChannelEstimation::FWIENER);
}

void systemevalDlg::OnRadioTiSyncFirstPeak()
{
    if (DRMReceiver.GetTiSyncTracType() !=
            CTimeSyncTrack::TSFIRSTPEAK)
    {
        DRMReceiver.SetTiSyncTracType(CTimeSyncTrack::TSFIRSTPEAK);
    }
}

void systemevalDlg::OnRadioTiSyncEnergy()
{
    if (DRMReceiver.GetTiSyncTracType() !=
            CTimeSyncTrack::TSENERGY)
    {
        DRMReceiver.SetTiSyncTracType(CTimeSyncTrack::TSENERGY);
    }
}

void systemevalDlg::OnSliderIterChange(int value)
{
    /* Set new value in working thread module */
    DRMReceiver.GetMSCMLC()->SetNumIterations(value);

    /* Show the new value in the label control */
    TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
                                 QString().setNum(value));
}

void systemevalDlg::OnListSelChanged(QTreeWidgetItem *curr, QTreeWidgetItem *)
{
    /* Get char type from selected item and setup chart */
    MainPlot->SetupChart(CDRMPlot::ECharType(curr->data(0, Qt::UserRole).toInt()));
}

void systemevalDlg::OnCheckFlipSpectrum()
{
    /* Set parameter in working thread module */
    DRMReceiver.GetReceiveData()->
    SetFlippedSpectrum(CheckBoxFlipSpec->isChecked());
}

void systemevalDlg::OnCheckRecFilter()
{
    /* Set parameter in working thread module */
    DRMReceiver.GetFreqSyncAcq()->
    SetRecFilter(CheckBoxRecFilter->isChecked());

    /* If filter status is changed, a new aquisition is necessary */
    DRMReceiver.RequestNewAcquisition();
}

void systemevalDlg::OnCheckModiMetric()
{
    /* Set parameter in working thread module */
    DRMReceiver.SetIntCons(CheckBoxModiMetric->isChecked());
}

void systemevalDlg::OnCheckBoxMuteAudio()
{
    /* Set parameter in working thread module */
    DRMReceiver.GetWriteData()->MuteAudio(CheckBoxMuteAudio->isChecked());
}

void systemevalDlg::OnCheckBoxReverb()
{
    /* Set parameter in working thread module */
    DRMReceiver.GetAudSorceDec()->SetReverbEffect(CheckBoxReverb->isChecked());
}

void systemevalDlg::OnCheckSaveAudioWAV()
{
    /*
    	This code is copied in AnalogDemDlg.cpp. If you do changes here, you should
    	apply the changes in the other file, too
    */
    if (CheckBoxSaveAudioWave->isChecked() == TRUE)
    {
        /* Show "save file" dialog */
        QString strFileName =
            QFileDialog::getSaveFileName(this, "*.wav", tr("DreamOut.wav"));

        /* Check if user not hit the cancel button */
        if (!strFileName.isEmpty())
        {
			string s = strFileName.toUtf8().constData();
            DRMReceiver.GetWriteData()->StartWriteWaveFile(s);
        }
        else
        {
            /* User hit the cancel button, uncheck the button */
            CheckBoxSaveAudioWave->setChecked(FALSE);
        }
    }
    else
        DRMReceiver.GetWriteData()->StopWriteWaveFile();
}


void systemevalDlg::OnCheckWriteLog()
{
    if (CheckBoxWriteLog->isChecked())
    {
		emit startLogging();
    }
    else
    {
		emit stopLogging();
    }

    /* set the focus */
    if (EdtFrequency->isEnabled())
    {
        EdtFrequency->setFocus();
    }
}

QString	systemevalDlg::GetRobModeStr()
{
    CParameter& Parameters = *DRMReceiver.GetParameters();
    switch (Parameters.GetWaveMode())
    {
    case RM_ROBUSTNESS_MODE_A:
        return "A";
        break;

    case RM_ROBUSTNESS_MODE_B:
        return "B";
        break;

    case RM_ROBUSTNESS_MODE_C:
        return "C";
        break;

    case RM_ROBUSTNESS_MODE_D:
        return "D";
        break;

    default:
        return "A";
    }
}

QString	systemevalDlg::GetSpecOccStr()
{
    switch (DRMReceiver.GetParameters()->GetSpectrumOccup())
    {
    case SO_0:
        return "4,5 kHz";
        break;

    case SO_1:
        return "5 kHz";
        break;

    case SO_2:
        return "9 kHz";
        break;

    case SO_3:
        return "10 kHz";
        break;

    case SO_4:
        return "18 kHz";
        break;

    case SO_5:
        return "20 kHz";
        break;

    default:
        return "10 kHz";
    }
}

void systemevalDlg::AddWhatsThisHelp()
{
    /*
    	This text was taken from the only documentation of Dream software
    */
    /* DC Frequency Offset */
    const QString strDCFreqOffs =
        tr("<b>DC Frequency Offset:</b> This is the "
           "estimation of the DC frequency offset. This offset corresponds "
           "to the resulting sound card intermedia frequency of the front-end. "
           "This frequency is not restricted to a certain value. The only "
           "restriction is that the DRM spectrum must be completely inside the "
           "bandwidth of the sound card.");

    TextFreqOffset->setWhatsThis(strDCFreqOffs);
    ValueFreqOffset->setWhatsThis(strDCFreqOffs);

    /* Sample Frequency Offset */
    const QString strFreqOffset =
        tr("<b>Sample Frequency Offset:</b> This is the "
           "estimation of the sample rate offset between the sound card sample "
           "rate of the local computer and the sample rate of the D / A (digital "
           "to analog) converter in the transmitter. Usually the sample rate "
           "offset is very constant for a given sound card. Therefore it is "
           "useful to inform the Dream software about this value at application "
           "startup to increase the acquisition speed and reliability.");

    TextSampFreqOffset->setWhatsThis(strFreqOffset);
    ValueSampFreqOffset->setWhatsThis(strFreqOffset);

    /* Doppler / Delay */
    const QString strDopplerDelay =
        tr("<b>Doppler / Delay:</b> The Doppler frequency "
           "of the channel is estimated for the Wiener filter design of channel "
           "estimation in time direction. If linear interpolation is set for "
           "channel estimation in time direction, this estimation is not updated. "
           "The Doppler frequency is an indication of how fast the channel varies "
           "with time. The higher the frequency, the faster the channel changes "
           "are.<br>The total delay of the Power Delay Spectrum "
           "(PDS) is estimated from the impulse response estimation derived from "
           "the channel estimation. This delay corresponds to the range between "
           "the two vertical dashed black lines in the Impulse Response (IR) "
           "plot.");

	TextWiener->setWhatsThis(strDopplerDelay);
    ValueWiener->setWhatsThis(strDopplerDelay);

    /* I / O Interface LED */
    const QString strLEDIOInterface =
        tr("<b>I / O Interface LED:</b> This LED shows the "
           "current status of the sound card interface. The yellow light shows "
           "that the audio output was corrected. Since the sample rate of the "
           "transmitter and local computer are different, from time to time the "
           "audio buffers will overflow or under run and a correction is "
           "necessary. When a correction occurs, a \"click\" sound can be heard. "
           "The red light shows that a buffer was lost in the sound card input "
           "stream. This can happen if a thread with a higher priority is at "
           "100% and the Dream software cannot read the provided blocks fast "
           "enough. In this case, the Dream software will instantly loose the "
           "synchronization and has to re-synchronize. Another reason for red "
           "light is that the processor is too slow for running the Dream "
           "software.");

    TextLabelLEDIOInterface->setWhatsThis(strLEDIOInterface);
    LEDIOInterface->setWhatsThis(strLEDIOInterface);

    /* Time Sync Acq LED */
    const QString strLEDTimeSyncAcq =
        tr("<b>Time Sync Acq LED:</b> This LED shows the "
           "state of the timing acquisition (search for the beginning of an OFDM "
           "symbol). If the acquisition is done, this LED will stay green.");

    TextLabelLEDTimeSyncAcq->setWhatsThis(strLEDTimeSyncAcq);
    LEDTimeSync->setWhatsThis(strLEDTimeSyncAcq);

    /* Frame Sync LED */
    const QString strLEDFrameSync =
        tr("<b>Frame Sync LED:</b> The DRM frame "
           "synchronization status is shown with this LED. This LED is also only "
           "active during acquisition state of the Dream receiver. In tracking "
           "mode, this LED is always green.");

    TextLabelLEDFrameSync->setWhatsThis(strLEDFrameSync);
    LEDFrameSync->setWhatsThis(strLEDFrameSync);

    /* FAC CRC LED */
    const QString strLEDFACCRC =
        tr("<b>FAC CRC LED:</b> This LED shows the Cyclic "
           "Redundancy Check (CRC) of the Fast Access Channel (FAC) of DRM. FAC "
           "is one of the three logical channels and is always modulated with a "
           "4-QAM. If the FAC CRC check was successful, the receiver changes to "
           "tracking mode. The FAC LED is the indication whether the receiver "
           "is synchronized to a DRM transmission or not.<br>"
           "The bandwidth of the DRM signal, the constellation scheme of MSC and "
           "SDC channels and the interleaver depth are some of the parameters "
           "which are provided by the FAC.");

    TextLabelLEDFACCRC->setWhatsThis(strLEDFACCRC);
    LEDFAC->setWhatsThis(strLEDFACCRC);

    /* SDC CRC LED */
    const QString strLEDSDCCRC =
        tr("<b>SDC CRC LED:</b> This LED shows the CRC "
           "check result of the Service Description Channel (SDC) which is one "
           "logical channel of the DRM stream. This data is transmitted in "
           "approx. 1 second intervals and contains information about station "
           "label, audio and data format, etc. The error protection is normally "
           "lower than the protection of the FAC. Therefore this LED will turn "
           "to red earlier than the FAC LED in general.<br>If the CRC check "
           "is ok but errors in the content were detected, the LED turns "
           "yellow.");

    TextLabelLEDSDCCRC->setWhatsThis(strLEDSDCCRC);
    LEDSDC->setWhatsThis(strLEDSDCCRC);

    /* MSC CRC LED */
    const QString strLEDMSCCRC =
        tr("<b>MSC CRC LED:</b> This LED shows the status "
           "of the Main Service Channel (MSC). This channel contains the actual "
           "audio and data bits. The LED shows the CRC check of the AAC core "
           "decoder. The SBR has a separate CRC, but this status is not shown "
           "with this LED. If SBR CRC is wrong but the AAC CRC is ok one can "
           "still hear something (of course, the high frequencies are not there "
           "in this case). If this LED turns red, interruptions of the audio are "
           "heard. The yellow light shows that only one 40 ms audio frame CRC "
           "was wrong. This causes usually no hearable artifacts.");

    TextLabelLEDMSCCRC->setWhatsThis(strLEDMSCCRC);
    LEDMSC->setWhatsThis(strLEDMSCCRC);

    /* MLC, Number of Iterations */
    const QString strNumOfIterations =
        tr("<b>MLC, Number of Iterations:</b> In DRM, a "
           "multilevel channel coder is used. With this code it is possible to "
           "iterate the decoding process in the decoder to improve the decoding "
           "result. The more iterations are used the better the result will be. "
           "But switching to more iterations will increase the CPU load. "
           "Simulations showed that the first iteration (number of "
           "iterations = 1) gives the most improvement (approx. 1.5 dB at a "
           "BER of 10-4 on a Gaussian channel, Mode A, 10 kHz bandwidth). The "
           "improvement of the second iteration will be as small as 0.3 dB."
           "<br>The recommended number of iterations given in the DRM "
           "standard is one iteration (number of iterations = 1).");

    TextNumOfIterations->setWhatsThis(strNumOfIterations);
    SliderNoOfIterations->setWhatsThis(strNumOfIterations);

    /* Flip Input Spectrum */
    CheckBoxFlipSpec->setWhatsThis(
                     tr("<b>Flip Input Spectrum:</b> Checking this box "
                        "will flip or invert the input spectrum. This is necessary if the "
                        "mixer in the front-end uses the lower side band."));

    /* Mute Audio */
    CheckBoxMuteAudio->setWhatsThis(
                     tr("<b>Mute Audio:</b> The audio can be muted by "
                        "checking this box. The reaction of checking or unchecking this box "
                        "is delayed by approx. 1 second due to the audio buffers."));

    /* Reverberation Effect */
    CheckBoxReverb->setWhatsThis(
                     tr("<b>Reverberation Effect:</b> If this check box is checked, a "
                        "reverberation effect is applied each time an audio drop-out occurs. "
                        "With this effect it is possible to mask short drop-outs."));

    /* Log File */
    CheckBoxWriteLog->setWhatsThis(
                     tr("<b>Log File:</b> Checking this box brings the "
                        "Dream software to write a log file about the current reception. "
                        "Every minute the average SNR, number of correct decoded FAC and "
                        "number of correct decoded MSC blocks are logged including some "
                        "additional information, e.g. the station label and bit-rate. The "
                        "log mechanism works only for audio services using AAC source coding. "
#ifdef _WIN32
                        "During the logging no Dream windows "
                        "should be moved or re-sized. This can lead to incorrect log files "
                        "(problem with QT timer implementation under Windows). This problem "
                        "does not exist in the Linux version of Dream."
#endif
                        "<br>The log file will be "
                        "written in the directory were the Dream application was started and "
                        "the name of this file is always DreamLog.txt"));

    /* Freq */
    EdtFrequency->setWhatsThis(
                     tr("<b>Freq:</b> In this edit control, the current "
                        "selected frequency on the front-end can be specified. This frequency "
                        "will be written into the log file."));

    /* Wiener */
    const QString strWienerChanEst =
        tr("<b>Channel Estimation Settings:</b> With these "
           "settings, the channel estimation method in time and frequency "
           "direction can be selected. The default values use the most powerful "
           "algorithms. For more detailed information about the estimation "
           "algorithms there are a lot of papers and books available.<br>"
           "<b>Wiener:</b> Wiener interpolation method "
           "uses estimation of the statistics of the channel to design an optimal "
           "filter for noise reduction.");

    RadioButtonFreqWiener->setWhatsThis(strWienerChanEst);
    RadioButtonTiWiener->setWhatsThis(strWienerChanEst);

    /* Linear */
    const QString strLinearChanEst =
        tr("<b>Channel Estimation Settings:</b> With these "
           "settings, the channel estimation method in time and frequency "
           "direction can be selected. The default values use the most powerful "
           "algorithms. For more detailed information about the estimation "
           "algorithms there are a lot of papers and books available.<br>"
           "<b>Linear:</b> Simple linear interpolation "
           "method to get the channel estimate. The real and imaginary parts "
           "of the estimated channel at the pilot positions are linearly "
           "interpolated. This algorithm causes the lowest CPU load but "
           "performs much worse than the Wiener interpolation at low SNRs.");

    RadioButtonFreqLinear->setWhatsThis(strLinearChanEst);
    RadioButtonTiLinear->setWhatsThis(strLinearChanEst);

    /* DFT Zero Pad */
    RadioButtonFreqDFT->setWhatsThis(
                     tr("<b>Channel Estimation Settings:</b> With these "
                        "settings, the channel estimation method in time and frequency "
                        "direction can be selected. The default values use the most powerful "
                        "algorithms. For more detailed information about the estimation "
                        "algorithms there are a lot of papers and books available.<br>"
                        "<b>DFT Zero Pad:</b> Channel estimation method "
                        "for the frequency direction using Discrete Fourier Transformation "
                        "(DFT) to transform the channel estimation at the pilot positions to "
                        "the time domain. There, a zero padding is applied to get a higher "
                        "resolution in the frequency domain -> estimates at the data cells. "
                        "This algorithm is very speed efficient but has problems at the edges "
                        "of the OFDM spectrum due to the leakage effect."));

    /* Guard Energy */
    RadioButtonTiSyncEnergy->setWhatsThis(
                     tr("<b>Guard Energy:</b> Time synchronization "
                        "tracking algorithm utilizes the estimation of the impulse response. "
                        "This method tries to maximize the energy in the guard-interval to set "
                        "the correct timing."));

    /* First Peak */
    RadioButtonTiSyncFirstPeak->setWhatsThis(
                     tr("<b>First Peak:</b> This algorithms searches for "
                        "the first peak in the estimated impulse response and moves this peak "
                        "to the beginning of the guard-interval (timing tracking algorithm)."));

    /* SNR */
    const QString strSNREst =
        tr("<b>SNR:</b> Signal to Noise Ratio (SNR) "
           "estimation based on FAC cells. Since the FAC cells are only "
           "located approximately in the region 0-5 kHz relative to the DRM DC "
           "frequency, it may happen that the SNR value is very high "
           "although the DRM spectrum on the left side of the DRM DC frequency "
           "is heavily distorted or disturbed by an interferer so that the true "
           "overall SNR is lower as indicated by the SNR value. Similarly, "
           "the SNR value might show a very low value but audio can still be "
           "decoded if only the right side of the DRM spectrum is degraded "
           "by an interferer.");

    ValueSNR->setWhatsThis(strSNREst);
    TextSNRText->setWhatsThis(strSNREst);

    /* MSC WMER / MSC MER */
    const QString strMERWMEREst =
        tr("<b>MSC WMER / MSC MER:</b> Modulation Error Ratio (MER) and "
           "weighted MER (WMER) calculated on the MSC cells is shown. The MER is "
           "calculated as follows: For each equalized MSC cell (only MSC cells, "
           "no FAC cells, no SDC cells, no pilot cells), the error vector from "
           "the nearest ideal point of the constellation diagram is measured. The "
           "squared magnitude of this error is found, and a mean of the squared "
           "errors is calculated (over one frame). The MER is the ratio in [dB] "
           "of the mean of the squared magnitudes of the ideal points of the "
           "constellation diagram to the mean squared error. This gives an "
           "estimate of the ratio of the total signal power to total noise "
           "power at the input to the equalizer for channels with flat frequency "
           "response.<br> In case of the WMER, the calculations of the means are "
           "multiplied by the squared magnitude of the estimated channel "
           "response.<br>For more information see ETSI TS 102 349.");

    ValueMERWMER->setWhatsThis(strMERWMEREst);
    TextMERWMER->setWhatsThis(strMERWMEREst);

    /* DRM Mode / Bandwidth */
    const QString strRobustnessMode =
        tr("<b>DRM Mode / Bandwidth:</b> In a DRM system, "
           "four possible robustness modes are defined to adapt the system to "
           "different channel conditions. According to the DRM standard:<ul>"
           "<li><i>Mode A:</i> Gaussian channels, with "
           "minor fading</li><li><i>Mode B:</i> Time "
           "and frequency selective channels, with longer delay spread</li>"
           "<li><i>Mode C:</i> As robustness mode B, but "
           "with higher Doppler spread</li>"
           "<li><i>Mode D:</i> As robustness mode B, but "
           "with severe delay and Doppler spread</li></ul>The "
           "bandwith is the gross bandwidth of the current DRM signal");

    FACDRMModeBWL->setWhatsThis(strRobustnessMode);
    FACDRMModeBWV->setWhatsThis(strRobustnessMode);

    /* Interleaver Depth */
    const QString strInterleaver =
        tr("<b>Interleaver Depth:</b> The symbol "
           "interleaver depth can be either short (approx. 400 ms) or long "
           "(approx. 2 s). The longer the interleaver the better the channel "
           "decoder can correct errors from slow fading signals. But the "
           "longer the interleaver length the longer the delay until (after a "
           "re-synchronization) audio can be heard.");

    FACInterleaverDepthL->setWhatsThis(strInterleaver);
    FACInterleaverDepthV->setWhatsThis(strInterleaver);

    /* SDC / MSC Mode */
    const QString strSDCMSCMode =
        tr("<b>SDC / MSC Mode:</b> Shows the modulation "
           "type of the SDC and MSC channel. For the MSC channel, some "
           "hierarchical modes are defined which can provide a very strong "
           "protected service channel.");

    FACSDCMSCModeL->setWhatsThis(strSDCMSCMode);
    FACSDCMSCModeV->setWhatsThis(strSDCMSCMode);

    /* Prot. Level (B/A) */
    const QString strProtLevel =
        tr("<b>Prot. Level (B/A):</b> The error protection "
           "level of the channel coder. For 64-QAM, there are four protection "
           "levels defined in the DRM standard. Protection level 0 has the "
           "highest protection whereas level 3 has the lowest protection. The "
           "letters A and B are the names of the higher and lower protected parts "
           "of a DRM block when Unequal Error Protection (UEP) is used. If Equal "
           "Error Protection (EEP) is used, only the protection level of part B "
           "is valid.");

    FACCodeRateL->setWhatsThis(strProtLevel);
    FACCodeRateV->setWhatsThis(strProtLevel);

    /* Number of Services */
    const QString strNumServices =
        tr("<b>Number of Services:</b> This shows the "
           "number of audio and data services transmitted in the DRM stream. "
           "The maximum number of streams is four.");

    FACNumServicesL->setWhatsThis(strNumServices);
    FACNumServicesV->setWhatsThis(strNumServices);

    /* Received time - date */
    const QString strTimeDate =
        tr("<b>Received time - date:</b> This label shows "
           "the received time and date in UTC. This information is carried in "
           "the SDC channel.");

    FACTimeDateL->setWhatsThis(strTimeDate);
    FACTimeDateV->setWhatsThis(strTimeDate);

    /* Save audio as wave */
    CheckBoxSaveAudioWave->setWhatsThis(
                     tr("<b>Save Audio as WAV:</b> Save the audio signal "
                        "as stereo, 16-bit, 48 kHz sample rate PCM wave file. Checking this "
                        "box will let the user choose a file name for the recording."));

    /* Interferer Rejection */
    const QString strInterfRej =
        tr("<b>Interferer Rejection:</b> There are two "
           "algorithms available to reject interferers:<ul>"
           "<li><b>Bandpass Filter (BP-Filter):</b>"
           " The bandpass filter is designed to have the same bandwidth as "
           "the DRM signal. If, e.g., a strong signal is close to the border "
           "of the actual DRM signal, under some conditions this signal will "
           "produce interference in the useful bandwidth of the DRM signal "
           "although it is not on the same frequency as the DRM signal. "
           "The reason for that behaviour lies in the way the OFDM "
           "demodulation is done. Since OFDM demodulation is a block-wise "
           "operation, a windowing has to be applied (which is rectangular "
           "in case of OFDM). As a result, the spectrum of a signal is "
           "convoluted with a Sinc function in the frequency domain. If a "
           "sinusoidal signal close to the border of the DRM signal is "
           "considered, its spectrum will not be a distinct peak but a "
           "shifted Sinc function. So its spectrum is broadened caused by "
           "the windowing. Thus, it will spread in the DRM spectrum and "
           "act as an in-band interferer.<br>"
           "There is a special case if the sinusoidal signal is in a "
           "distance of a multiple of the carrier spacing of the DRM signal. "
           "Since the Sinc function has zeros at certain positions it happens "
           "that in this case the zeros are exactly at the sub-carrier "
           "frequencies of the DRM signal. In this case, no interference takes "
           "place. If the sinusoidal signal is in a distance of a multiple of "
           "the carrier spacing plus half of the carrier spacing away from the "
           "DRM signal, the interference reaches its maximum.<br>"
           "As a result, if only one DRM signal is present in the 20 kHz "
           "bandwidth, bandpass filtering has no effect. Also,  if the "
           "interferer is far away from the DRM signal, filtering will not "
           "give much improvement since the squared magnitude of the spectrum "
           "of the Sinc function is approx -15 dB down at 1 1/2 carrier "
           "spacing (approx 70 Hz with DRM mode B) and goes down to approx "
           "-30 dB at 10 times the carrier spacing plus 1 / 2 of the carrier "
           "spacing (approx 525 Hz with DRM mode B). The bandpass filter must "
           "have very sharp edges otherwise the gain in performance will be "
           "very small.</li>"
           "<li><b>Modified Metrics:</b> Based on the "
           "information from the SNR versus sub-carrier estimation, the metrics "
           "for the Viterbi decoder can be modified so that sub-carriers with "
           "high noise are attenuated and do not contribute too much to the "
           "decoding result. That can improve reception under bad conditions but "
           "may worsen the reception in situations where a lot of fading happens "
           "and no interferer are present since the SNR estimation may be "
           "not correct.</li></ul>");

    GroupBoxInterfRej->setWhatsThis(strInterfRej);
    CheckBoxRecFilter->setWhatsThis(strInterfRej);
    CheckBoxModiMetric->setWhatsThis(strInterfRej);
}
