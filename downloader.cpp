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

    // URL setup
    baseUrl = new QUrl();
    baseUrl->setScheme("ftp");
    baseUrl->setHost(FTP_SERVER);
    baseUrl->setUserName(FTP_USER);

    // FTP setup
    currentFtpFile = "";
    timeout = new QTimer(this);
    ftp = new QNetworkAccessManager();
    connect(timeout, SIGNAL(timeout()), this, SLOT(Reconnect()));

    // Launch
#ifdef USE_PASSWORD
    emit AskForPassword();
#else
    Login("");
#endif
}

/*--- Timeout has expired, restart file ---*/
void Downloader::Reconnect()
{
    emit PrintHeavyStreamedMessage("Timeout downloading " + currentFtpFile + ", retrying");
}

/*--- Use password to login ---*/
void Downloader::Login(QString pwd)
{
    passWd = pwd;
    baseUrl->setPassword(passWd);
    DownloadFile("", FTP_RELEASE_NOTES_FILE);
}

/*--- Prepare download, then issue the FTP commands ---*/
void Downloader::DownloadFile(QString dir, QString file)
{
    // Data
    QUrl fileUrl(*baseUrl);
    QNetworkRequest r;
    QDir tempDir(".");

    // Setup
    downloadedSize = 0;
    currentFtpDir = dir;
    currentFtpFile = file;
    currentFile = new QFile(currentFtpDir + currentFtpFile);
    fileUrl.setPath(FTP_UPDATE_ROOT + currentFtpDir + currentFtpFile);
    r.setUrl(fileUrl);

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
    bDownloading = true;
    reply = ftp->get(r);
    reply->setReadBufferSize(FTP_PART_SIZE);
    connect(reply, SIGNAL(readyRead()), this, SLOT(FilePart()));
    connect(ftp, SIGNAL(finished(QNetworkReply*)), this, SLOT(FileFinished(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(FileError(QNetworkReply::NetworkError)));
}

/*--- Received on FTP part downloaded ---*/
void Downloader::FilePart(void)
{
    int count = currentFile->write(reply->readAll());
    timeout->stop();
    timeout->setSingleShot(true);
    timeout->start(8192);
    downloadedSize += count;

    emit BytesDownloaded(count);
	emit PrintCurrentFile(currentFile->fileName());
}

/*--- Received when a command has failed ---*/
void Downloader::FileError(QNetworkReply::NetworkError code)
{
    if (bDownloading)
    {
        emit PrintStreamedMessage("Error downloading " + currentFtpFile);
        emit BytesDownloaded(-downloadedSize);
        emit FileDownloaded();
    }
    else
    {
        emit PrintStreamedMessage("Networking error");
        emit Reconnect();
    }
    emit PrintStreamedMessage("(error " + QString::number(code) + ")");
    reply->deleteLater();
}

/*--- Received when a command has ended (login, cd, get) ---*/
void Downloader::FileFinished(QNetworkReply* mreply)
{
    if (bDownloading)
	{
        bDownloading = false;
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
    mreply->deleteLater();
}
