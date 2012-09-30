/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#include "downloader.h"


/*----------------------------------------------
	 Constructor & destructor
----------------------------------------------*/

Downloader::Downloader()
{
	moveToThread(this);
}

Downloader::~Downloader()
{
	delete ftp;
}


/*----------------------------------------------
		   Slots
----------------------------------------------*/

/*--- Received on FTP status change ---*/
void Downloader::StatusUpdate(int status)
{
	switch(status)
	{
		case QFtp::LoggedIn:
		    DownloadFile("", FTP_RELEASE_NOTES_FILE);
		    break;

		default: break;
	}
}

/*--- Prepare download, then issue the FTP commands ---*/
void Downloader::DownloadFile(QString dir, QString file)
{
	// Data
	QDir tempDir(".");
	downloadedSize = 0;
	currentFtpDir = dir;
	currentFtpFile = file;
	currentFile = new QFile(currentFtpDir + currentFtpFile);

	// Local file preparation
	if (currentFtpDir.length() > 0)
	{
		tempDir.mkpath(currentFtpDir);
	}
	if (currentFtpFile.length() > 0 && currentFile->exists())
	{
		currentFile->remove();
	}
	currentFile->open(QIODevice::WriteOnly);

	// Download command
	ftp->cd(FTP_UPDATE_ROOT + QString("/") + currentFtpDir);
	get = ftp->get(currentFtpFile, 0, QFtp::Binary);
}

/*--- Received on FTP part downloaded ---*/
void Downloader::FilePart(void)
{
	int count = currentFile->write(ftp->readAll());
	downloadedSize += count;
	emit BytesDownloaded(count);
	emit PrintCurrentFile(currentFile->fileName());
}

/*--- Received when a command has ended (login, cd, get) ---*/
void Downloader::FileFinished(int id, bool error)
{
	if (error)
	{
		emit PrintStreamedMessage("Error downloading " + currentFtpFile);
		emit BytesDownloaded(-downloadedSize);
		emit FileDownloaded();
	}
	else if (get == id && currentFtpFile != "")
	{
		currentFile->close();
		delete currentFile;
		emit PrintCurrentFile("");

		if (currentFtpFile == FTP_RELEASE_NOTES_FILE)
		{
			emit ShowReleaseNotes();
			DownloadFile(FTP_MANIFEST_ROOT, FTP_MANIFEST_FILE);
		}

		else if (currentFtpFile == FTP_MANIFEST_FILE)
		{
			emit DownloadTreeFromManifest(QString(FTP_MANIFEST_ROOT) + QString(FTP_MANIFEST_FILE));
		}

		else
		{
			emit FileDownloaded();
		}
	}
}


/*----------------------------------------------
	       Private methods
----------------------------------------------*/

/*--- Launch thread ---*/
void Downloader::run()
{
	// Files init
	QFile::remove(FTP_OLD_RELEASE_NOTES_FILE);
	QFile::copy(FTP_RELEASE_NOTES_FILE, FTP_OLD_RELEASE_NOTES_FILE);
	QFile::remove(FTP_RELEASE_NOTES_FILE);
	QFile::remove(FTP_MANIFEST_FILE);

	// FTP setup
	get = -1;
	currentFtpFile = "";
	ftp = new QFtp(this);
	connect(ftp, SIGNAL(readyRead(void)), this, SLOT(FilePart(void)));
	connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(StatusUpdate(int)));
	connect(ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(FileFinished(int, bool)));

	// Launch
	ftp->connectToHost(FTP_SERVER);
	ftp->login(FTP_USER, FTP_PASSWORD);
	exec();
}
