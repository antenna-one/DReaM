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
#ifndef __TransmDlg_H
#define __TransmDlg_H

#include <qpushbutton.h>
#include <qstring.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qmenubar.h>
#include <qlayout.h>
#include <qthread.h>
#include <qtimer.h>
#include <qwt_thermo.h>
#if QT_VERSION < 0x040000
# include <qpopupmenu.h>
# include "TransmDlgbase.h"
#else
# include <QMenu>
# include <QDialog>
# include "ui_TransmDlgbase.h"
#endif

#ifdef _WIN32
# include "windows.h"
#endif
#include "DialogUtil.h"
#include "../DrmTransmitter.h"
#include "../Parameter.h"
#include "../util/Settings.h"


/* Classes ********************************************************************/
/* Thread class for the transmitter */
class CTransmitterThread : public QThread 
{
public:
	void Stop()
	{
		/* Stop working thread and wait until it is ready for terminating. We
		   set a time-out of 5 seconds */
		DRMTransmitter.Stop();

		if (wait(5000) == FALSE)
			ErrorMessage("Termination of working thread failed.");
	}

	virtual void run()
	{
		/* Set thread priority (The working thread should have a higher priority
		   than the GUI) */
#ifdef _WIN32
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif

		try
		{
			/* Call receiver main routine */
			DRMTransmitter.Start();
		}

		catch (CGenErr GenErr)
		{
			ErrorMessage(GenErr.strError);
		}
	}

	CDRMTransmitter	DRMTransmitter;
};

#if QT_VERSION >= 0x040000
class TransmDlgBase : public QDialog, public Ui_TransmDlgBase
{
public:
	TransmDlgBase(QWidget* parent = 0, const char* name = 0,
		bool modal = FALSE, Qt::WFlags f = 0):
		QDialog(parent,f){(void)name;(void)modal;setupUi(this);}
	virtual ~TransmDlgBase() {}
};
#endif
class TransmDialog : public TransmDlgBase
{
	Q_OBJECT

public:
	TransmDialog(CSettings&,
		QWidget* parent=0, const char* name=0, bool modal=FALSE, Qt::WFlags f=0);
	virtual ~TransmDialog();

protected:
	void DisableAllControlsForSet();
	void EnableAllControlsForSet();

	CSettings&			Settings;
	QMenuBar*			pMenu;
#if QT_VERSION < 0x040000
	QPopupMenu*			pSettingsMenu;
#else
	QMenu*				pSettingsMenu;
#endif
	QTimer				Timer;

	CTransmitterThread	TransThread; /* Working thread object */
	_BOOLEAN			bIsStarted;
	CVector<string>		vecstrTextMessage;
	int					iIDCurrentText;
	_BOOLEAN			GetMessageText(const int iID);
	void				UpdateMSCProtLevCombo();
	void				EnableTextMessage(const _BOOLEAN bFlag);
	void				EnableAudio(const _BOOLEAN bFlag);
	void				EnableData(const _BOOLEAN bFlag);
	void				AddWhatsThisHelp();


public slots:
	void OnButtonStartStop();
	void OnPushButtonAddText();
	void OnButtonClearAllText();
	void OnPushButtonAddFileName();
	void OnButtonClearAllFileNames();
	void OnToggleCheckBoxEnableData(bool bState);
	void OnToggleCheckBoxEnableAudio(bool bState);
	void OnToggleCheckBoxEnableTextMessage(bool bState);
	void OnComboBoxMSCInterleaverHighlighted(int iID);
	void OnComboBoxMSCConstellationHighlighted(int iID);
	void OnComboBoxSDCConstellationHighlighted(int iID);
	void OnComboBoxLanguageHighlighted(int iID);
	void OnComboBoxProgramTypeHighlighted(int iID);
	void OnComboBoxTextMessageHighlighted(int iID);
	void OnComboBoxMSCProtLevHighlighted(int iID);
	void OnRadioRobustnessMode(int iID);
	void OnRadioBandwidth(int iID);
	void OnRadioOutput(int iID);
	void OnTextChangedServiceLabel(const QString& strLabel);
	void OnTextChangedServiceID(const QString& strID);
	void OnTextChangedSndCrdIF(const QString& strIF);
	void OnTimer();
	void OnHelpWhatsThis();
};
#endif
