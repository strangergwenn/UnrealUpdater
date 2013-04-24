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
}

Downloader::~Downloader()
{
	delete ftp;
}


/*----------------------------------------------
		   Slots
----------------------------------------------*/

/*--- Launch thread ---*/
void Downloader::Connect()
{
    // Files init
    QFile::remove(FTP_OLD_RELEASE_NOTES_FILE);
    QFile::copy(FTP_RELEASE_NOTES_FILE, FTP_OLD_RELEASE_NOTES_FILE);
    QFile::remove(FTP_RELEASE_NOTES_FILE);
    QFile::remove(FTP_MANIFEST_FILE);

    // FTP setup
    get = -1;
    bLoggingIn = true;
    currentFtpFile = "";
    ftp = new QFtp(this);
    timeout = new QTimer(this);
    connect(timeout, SIGNAL(timeout()), this, SLOT(Reconnect()));
    connect(ftp, SIGNAL(readyRead(void)), this, SLOT(FilePart(void)));
    connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(StatusUpdate(int)));
    connect(ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(FileFinished(int, bool)));

    // Launch
    ftp->connectToHost(FTP_SERVER);
    emit AskForPassword();
}

/*--- Timeout has expired, restart file ---*/
void Downloader::Reconnect()
{
    emit PrintHeavyStreamedMessage("Timeout downloading " + currentFtpFile + ", retrying");
    ftp->connectToHost(FTP_SERVER);
    ftp->login(FTP_USER, passWd);
}

/*--- Use password to login ---*/
void Downloader::Login(QString pwd)
{
    passWd = pwd;
    ftp->login(FTP_USER, pwd);
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

/*--- Received on FTP status change ---*/
void Downloader::StatusUpdate(int status)
{
	switch(status)
	{
		case QFtp::LoggedIn:
			bLoggingIn = false;
            emit PrintStreamedMessage("> Logged in");
			DownloadFile("", FTP_RELEASE_NOTES_FILE);
            break;
    case QFtp::Connecting:
        emit PrintStreamedMessage("> Connecting");
        break;
    case QFtp::Connected:
        emit PrintStreamedMessage("> Connected");
        break;
    case QFtp::Closing:
        emit PrintStreamedMessage("> Disconnecting");
        break;
    case QFtp::Unconnected:
        emit PrintStreamedMessage("> Disconnected");
        break;
        default: break;
	}
}

/*--- Received on FTP part downloaded ---*/
void Downloader::FilePart(void)
{
	int count = currentFile->write(ftp->readAll());
	downloadedSize += count;
	StartTimeout(5000);

	emit BytesDownloaded(count);
	emit PrintCurrentFile(currentFile->fileName());
}

/*--- Received when a command has ended (login, cd, get) ---*/
void Downloader::FileFinished(int id, bool error)
{
	if (error)
	{
		if (currentFtpFile != "")
		{
			emit PrintStreamedMessage("Error downloading " + currentFtpFile);
		}
		emit BytesDownloaded(-downloadedSize);
		emit FileDownloaded();
		if (bLoggingIn)
		{
            emit Reconnect();
		}
	}
	else if (get == id && currentFtpFile != "")
	{
		timeout->stop();
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

/*--- Start a timer ---*/
void Downloader::StartTimeout(int millis)
{
	timeout->stop();
	timeout->setSingleShot(true);
	timeout->start(millis);
}
