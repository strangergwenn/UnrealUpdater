/**
 *  This work is distributed under the Lesser General Public License,
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
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QDomDocument>
#include <QTextStream>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <QDate>
#include <algorithm>

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

	public slots:

        // Update steps
        void Stage1(void);
        void Stage2(void);
        void Stage3(void);

        // Downloader interface
        void PasswordRequired(void);
        void BytesDownloaded(int deltaBytes, float downloadSpeed);
        void FileDownloaded(QString downloadedFile);

        // User interface
        void LaunchGame(void);
        void AboutMe(void);
        void Log(QString message, bool bIsHeavy);
        void SetUserMessage(QString message);

    signals:

        // Start downloading a file
        void DownloadFile(QString dir, QString file);

    private:

        // XML parsing
        void ParseReleaseNotes(QDomNode node, bool bIsCurrent);
        void ParseManifest(QDomNode node, QString dirName);

        // Utils
        QString HashFile(QFile* file);
        void SetSettingState(QString settingName, bool bState);
        bool GetSettingState(QString settingName);

    private:

        bool        	bAbortUpdate;

        int             downloadedBytes;
        int             downloadSize;

		QDate           currentDate;
		QDate           nextDate;
		QString         currentVersion;
		QString         nextVersion;

        QDomDocument*   currentDocument;
        File_t          currentFd;
        QList<QString>  filesToIgnore;
		QList<File_t>   filesToDownload;

        Downloader*     downloader;
        QThread*		downloaderThread;
		Ui::Updater     *ui;
        About*          aboutDialog;
};

extern QString TextToHtml(QString data);

#endif // UPDATER_H
