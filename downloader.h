/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "common.h"

/*----------------------------------------------
	Class definitions
----------------------------------------------*/

class Downloader : public QThread
{
	Q_OBJECT

	public:
		Downloader();
		~Downloader();

	public slots:
		void StatusUpdate(int status);
		void DownloadFile(QString dir, QString file);
		void FilePart(void);
		void FileFinished(int id, bool error);

	signals:
		void FileDownloaded(void);
		void BytesDownloaded(int);
		void ShowReleaseNotes(void);
		void PrintStreamedMessage(QString message);
		void PrintCurrentFile(QString fileName);
		void DownloadTreeFromManifest(QString fileName);

	private:
		void run();

		int     downloadedSize;
		int     get;
		QFtp*   ftp;
		QFile*  currentFile;
		QString currentFtpDir;
		QString currentFtpFile;

};

#endif // DOWNLOADER_H
