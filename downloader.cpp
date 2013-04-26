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
    baseUrl->setPort(21);
    baseUrl->setHost(FTP_SERVER);
    baseUrl->setUserName(FTP_USER);

    // FTP setup
    currentFtpFile = "";
    timeout = new QTimer(this);
    ftp = new QNetworkAccessManager();
    connect(timeout, SIGNAL(timeout()), this, SLOT(Reconnect()));
    connect(ftp, SIGNAL(finished(QNetworkReply*)), this, SLOT(FileFinished(QNetworkReply*)));

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
    // Suppress the FTP handler
    emit Log("Timeout downloading " + currentFtpFile, true);
    disconnect(timeout, SIGNAL(timeout()), this, SLOT(Reconnect()));
    disconnect(ftp, SIGNAL(finished(QNetworkReply*)), this, SLOT(FileFinished(QNetworkReply*)));
    delete ftp;

    // Restart it
    ftp = new QNetworkAccessManager();
    connect(timeout, SIGNAL(timeout()), this, SLOT(Reconnect()));
    connect(ftp, SIGNAL(finished(QNetworkReply*)), this, SLOT(FileFinished(QNetworkReply*)));
    emit DownloadFile(currentFtpDir, currentFtpFile);
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
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(FileError(QNetworkReply::NetworkError)));
}

/*--- Received on FTP part downloaded ---*/
void Downloader::FilePart(void)
{
    int count = currentFile->write(reply->readAll());
    timeout->stop();
    timeout->setSingleShot(true);
    timeout->start(FTP_TIMEOUT);
    downloadedSize += count;

    emit BytesDownloaded(count);
    emit SetCurrentFile(currentFile->fileName());
}

/*--- Received when a command has failed ---*/
void Downloader::FileError(QNetworkReply::NetworkError code)
{
    switch (code)
    {
        case QNetworkReply::NoError:
            break;

        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
            Log("Networking error (" + QString::number(code) + ")", true);
            break;

        case QNetworkReply::AuthenticationRequiredError:
        case QNetworkReply::SslHandshakeFailedError:
            Log("Security error (" + QString::number(code) + ")", true);
#ifdef USE_PASSWORD
            emit AskForPassword();
#else
            Login("");
#endif
            break;

        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::UnknownProxyError:
            Log("Proxy error (" + QString::number(code) + ")", true);
            break;

        default:
            Log("Server error (" + QString::number(code) + ")", true);
            break;
    }
}

/*--- Received when a download has ended ---*/
void Downloader::FileFinished(QNetworkReply* mreply)
{
    disconnect(reply, SIGNAL(readyRead()), this, SLOT(FilePart()));
    disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(FileError(QNetworkReply::NetworkError)));
    mreply->deleteLater();

    if (bDownloading)
	{
        bDownloading = false;
		timeout->stop();
		currentFile->close();
		delete currentFile;
        emit SetCurrentFile("");

		if (currentFtpFile == FTP_RELEASE_NOTES_FILE)
		{
            emit Stage1();
            DownloadFile(FTP_MANIFEST_ROOT, FTP_MANIFEST_FILE);
		}
		else if (currentFtpFile == FTP_MANIFEST_FILE)
		{
            emit Stage2();
		}
        else
		{
            emit FileDownloaded();
        }
    }
}
