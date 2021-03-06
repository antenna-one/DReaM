/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Andrea Russo
 *
 * Description:
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

#include "MultSettingsDlg.h"
#include <QFileDialog>
#include <QShowEvent>
#include <QHideEvent>
#include <QLabel>
#include <QToolTip>
#include "ThemeCustomizer.h"

/* Implementation *************************************************************/

MultSettingsDlg::MultSettingsDlg(CParameter& NP, CSettings& NSettings,
                                 QWidget* parent) :
    QDialog(parent), Parameters(NP), Settings(NSettings)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setupUi(this);

    /* Set help text for the controls */
    AddWhatsThisHelp();

    // TODO
    CheckBoxAddRefresh->hide();
    EdtSecRefresh->hide();

    /* Connect buttons */

    connect(PushButtonChooseDir, SIGNAL(clicked()),
            this, SLOT(OnbuttonChooseDir()));

    connect(buttonClearCacheMOT, SIGNAL(clicked()),
            this, SLOT(OnbuttonClearCacheMOT()));

    connect(buttonClearCacheEPG, SIGNAL(clicked()),
            this, SLOT(OnbuttonClearCacheEPG()));

    EdtSecRefresh->setValidator(new QIntValidator(MIN_MOT_BWS_REFRESH_TIME, MAX_MOT_BWS_REFRESH_TIME, EdtSecRefresh));

    APPLY_CUSTOM_THEME();
}

MultSettingsDlg::~MultSettingsDlg()
{
}

void MultSettingsDlg::hideEvent(QHideEvent*)
{
}

void MultSettingsDlg::showEvent(QShowEvent*)
{
    SetDataDirectoryControls();
}

void MultSettingsDlg::ClearCache(string subdir, QString sFilter = "", _BOOLEAN bDeleteDirs)
{
    /* Delete files into sPath directory with scan recursive */
    string p = Settings.Get("Receiver", "datafilesdirectory", string(DEFAULT_DATA_FILES_DIRECTORY));

    QDir dir(QString::fromUtf8((p+PATH_SEPARATOR+subdir).c_str()));
    ClearCache(dir, sFilter, bDeleteDirs);
}

void MultSettingsDlg::ClearCache(QDir dir, QString sFilter = "", _BOOLEAN bDeleteDirs)
{
    /* Check if the directory exists */
    if (dir.exists())
    {
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks);

        /* Eventually apply the filter */
        if (sFilter != "")
            dir.setNameFilters(QStringList(sFilter));

        dir.setSorting( QDir::DirsFirst );
        const QList<QFileInfo> list = dir.entryInfoList();
        for(QList<QFileInfo>::const_iterator fi = list.begin(); fi!=list.end(); fi++)
        {

            /* for each file/dir */
            /* if directory...=> scan recursive */
            if (fi->isDir())
            {
                if(fi->fileName()!="." && fi->fileName()!="..")
                {
                    ClearCache(QDir(fi->filePath()), sFilter, bDeleteDirs);

                    /* Eventually delete the directory */
                    if (bDeleteDirs == TRUE)
                        dir.rmdir(fi->fileName());
                }
            }
            else
            {
                /* Is a file so remove it */
                dir.remove(fi->fileName());
            }
        }
    }
}

void MultSettingsDlg::OnbuttonChooseDir()
{
    QString strFilename = QFileDialog::getExistingDirectory(this, TextLabelDir->text());
    /* Check if user not hit the cancel button */
    if (!strFilename.isEmpty())
    {
        strFilename.replace(QRegExp("/"), PATH_SEPARATOR);
        Settings.Put("Receiver", "datafilesdirectory",
                     string(strFilename.toUtf8().constData())
                    );
        SetDataDirectoryControls();
    }
}

void MultSettingsDlg::OnbuttonClearCacheMOT()
{
    ClearCache("MOT", "", TRUE);
}

void MultSettingsDlg::OnbuttonClearCacheEPG()
{
    /* Delete all EPG files */
    ClearCache("EPG", "*.EHA;*.EHB");
}

void MultSettingsDlg::SetDataDirectoryControls()
{
    QString sep(PATH_SEPARATOR);
    /* Delete files into sPath directory with scan recursive */
    string p = Settings.Get("Receiver", "datafilesdirectory", string(DEFAULT_DATA_FILES_DIRECTORY));
    QString strFilename = QString::fromUtf8(p.c_str());
    strFilename.replace(QRegExp("/"), sep);
    if (!strFilename.isEmpty() && strFilename.at(strFilename.length()-1) == QChar(sep[0]))
        strFilename.remove(strFilename.length()-1, 1);
    TextLabelDir->setToolTip(strFilename);
    TextLabelDir->setText(strFilename);
}

void MultSettingsDlg::AddWhatsThisHelp()
{
    //TODO
}
