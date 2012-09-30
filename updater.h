/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef UPDATER_H
#define UPDATER_H

/*----------------------------------------------
	Includes & definitions
----------------------------------------------*/

#include <QCryptographicHash>
#include <QTextStream>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <QDate>
#include <QList>

#include "common.h"
#include "about.h"
#include "downloader.h"


/*----------------------------------------------
	Class definitions
----------------------------------------------*/

namespace Ui {
	class Updater;
}

class Updater : public QMainWindow
{
	Q_OBJECT

	public:
		explicit Updater(QWidget *parent = 0);
		~Updater();

		static bool GetLock(void)
		{
			return QFile::exists(S_LOCK_FILE);
		}

	public slots:
		void ShowReleaseNotes(void);
		void DownloadTreeFromManifest(QString fileName);
		void BytesDownloaded(int number);

		void PrintStreamedMessage(QString message);
		void PrintStreamedIfNotNull(QString message, QList<File_t> list);
		void PrintCurrentFile(QString fileName);
		void PrintUserMessage(QString message);

		void SetServerMode(int bNewState);
		void SetAutoLaunch(int bNewState);
		void LaunchGame(void);
		void AboutMe(void);

	signals:
		void DownloadFile(QString dir, QString file);

	private:
		void FormatReleaseNotes(QDomNode node, bool bIsCurrent);
		void GetFilesToDownload(QDomNode node, QString dirName);
		void UpdateEnded(void);
		QString HashFile(QFile* file);
		void SetLock(bool bLockState);

		void SetSettingState(bool bState, QString settingName);
		bool GetSettingState(QString settingName);
		void InstallNetFramework(void);

		bool            bDownloadPart;
		bool            bServerMode;
		bool            bAutoLaunch;

		int             downloadedBytes;
		int             downloadSize;
		QDate           currentDate;
		QDate           nextDate;
		QString         currentVersion;
		QString         nextVersion;

		QList<QString>  notUpdatedFiles;
		QList<File_t>   filesToDownload;

		Downloader*     dlThread;
		Ui::Updater     *ui;
};

extern QString TextToHtml(QString data);

#endif // UPDATER_H
