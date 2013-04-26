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
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QDomDocument>
#include <QTextStream>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <QDate>

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
        void StartDownload(void);
        void BytesDownloaded(int number);
        void FileDownloaded(void);
        void AskForPassword(void);

        void PrintStreamedMessage(QString message);
        void PrintHeavyStreamedMessage(QString message);
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

		QString HashFile(QFile* file);
        void InstallNetFramework(void);
		void SetLock(bool bLockState);
		void SetSettingState(bool bState, QString settingName);
        bool GetSettingState(QString settingName);

        bool        	bAbortUpdate;
		bool            bDownloadPart;
		bool            bServerMode;
		bool            bAutoLaunch;

		int             downloadedBytes;
		int             downloadSize;
		QDate           currentDate;
		QDate           nextDate;
		QString         currentVersion;
		QString         nextVersion;

        QDomDocument*   dom;
        File_t          currentFd;
		QList<QString>  notUpdatedFiles;
		QList<File_t>   filesToDownload;

        Downloader*     dlObject;
        QThread*		dlThread;
		Ui::Updater     *ui;
};

extern QString TextToHtml(QString data);

#endif // UPDATER_H
