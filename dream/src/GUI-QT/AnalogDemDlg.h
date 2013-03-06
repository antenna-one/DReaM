/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2005
 *
 * Author(s):
 *	Volker Fischer, Andrew Murphy
 *
 * Description:
 *	See AnalogDemDlg.cpp
 *
 * 11/21/2005 Andrew Murphy, BBC Research & Development, 2005
 *	- Additional widgets for displaying AMSS information
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


#include <QTimer>
#include <QDialog>
#include <QButtonGroup>
#include "ui_AMMainWindow.h"
#include "ui_AMSSDlgbase.h"
#include "SoundCardSelMenu.h"
#include "DialogUtil.h"
#include "DRMPlot.h"
#include "../GlobalDefinitions.h"
#include "../DrmReceiver.h"
#include "../util/Settings.h"
#include "../tables/TableAMSS.h"


/* Definitions ****************************************************************/
/* Update time of PLL phase dial control */
#define PLL_PHASE_DIAL_UPDATE_TIME				100


/* Classes ********************************************************************/


/* AMSS dialog -------------------------------------------------------------- */
class CAMSSDlg : public QDialog, public Ui_CAMSSDlgBase
{
	Q_OBJECT

public:
	CAMSSDlg(CDRMReceiver&, CSettings&, QWidget* parent = 0);

protected:
	CDRMReceiver&	DRMReceiver;
	CSettings&		Settings;
	CEventFilter	ef;

	QTimer			Timer;
	QTimer			TimerPLLPhaseDial;
	void			AddWhatsThisHelp();
	void			showEvent(QShowEvent*);
	void			hideEvent(QHideEvent*);

public slots:
	void OnTimer();
	void OnTimerPLLPhaseDial();
};


/* Analog demodulation dialog ----------------------------------------------- */
class AnalogDemDlg : public QMainWindow, public Ui_AMMainWindow
{
	Q_OBJECT

public:
	AnalogDemDlg(CDRMReceiver&, CSettings&, CFileMenu*, CSoundCardSelMenu*,
	QWidget* parent = 0);

	void SetWindowGeometry();

protected:
	CDRMReceiver&		DRMReceiver;
	CSettings&			Settings;

	QTimer				Timer;
	QTimer				TimerPLLPhaseDial;
	QTimer				TimerClose;
	CAMSSDlg			AMSSDlg;
    CDRMPlot*			MainPlot;
	CFileMenu*			pFileMenu;
	CSoundCardSelMenu*	pSoundCardMenu;
	CEventFilter		ef;

	void UpdateControls();
	void UpdateSliderBandwidth();
	void AddWhatsThisHelp();
	void showEvent(QShowEvent* pEvent);
	void hideEvent(QHideEvent* pEvent);

public slots:
	void switchEvent();
	void closeEvent(QCloseEvent* pEvent);
	void UpdatePlotStyle(int);
	void OnSampleRateChanged();
	void OnSoundFileChanged(CDRMReceiver::ESFStatus);
	void OnTimer();
	void OnTimerPLLPhaseDial();
	void OnTimerClose();
	void OnRadioDemodulation(int iID);
	void OnRadioAGC(int iID);
	void OnCheckBoxMuteAudio();
	void OnCheckSaveAudioWAV();
	void OnCheckAutoFreqAcq();
	void OnCheckPLL();
	void OnChartxAxisValSet(double dVal);
	void OnSliderBWChange(int value);
	void OnRadioNoiRed(int iID);
	void OnButtonWaterfall();
	void OnButtonAMSS();
	void OnSwitchToDRM();
	void OnSwitchToFM();
	void OnHelpAbout() {emit About();}
	void OnWhatsThis();

signals:
	void SwitchMode(int);
	void NewAMAcquisition();
	void ViewStationsDlg();
	void ViewLiveScheduleDlg();
	void Closed();
	void About();
};
