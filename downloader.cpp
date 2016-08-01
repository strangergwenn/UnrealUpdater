/**
 *  This work is distributed under the Lesser General Public License,
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
    delete chrono;
    delete timeoutTimer;
    delete speedUpdateTimer;
    delete downloadUpdateTimer;
}


/*----------------------------------------------
        Connection handling
----------------------------------------------*/

/*--- Launch thread ---*/
void Downloader::Connect()
{
    qDebug() << "Downloader::Connect";

    // Files init
    QFile::remove(FTP_OLD_RELEASE_NOTES_FILE);
    QFile::copy(FTP_RELEASE_NOTES_FILE, FTP_OLD_RELEASE_NOTES_FILE);
    QFile::remove(FTP_RELEASE_NOTES_FILE);
    QFile::remove(FTP_MANIFEST_FILE);

    // URL setup
    ftpClient = new QUrl();
    ftpClient->setScheme("ftp");
    ftpClient->setPort(21);
    ftpClient->setHost(FTP_SERVER);
    ftpClient->setUserName(FTP_USER);

    // FTP setup
    currentSpeed = 0.0;
    currentFtpFile = "";
    ftp = new QNetworkAccessManager();
    connect(ftp, &QNetworkAccessManager::finished, this, &Downloader::FileFinished);

    // Timers
    chrono = new QTime();
    timeoutTimer =        new QTimer(this);
    speedUpdateTimer =    new QTimer(this);
    downloadUpdateTimer = new QTimer(this);
    connect(timeoutTimer,        &QTimer::timeout, this, &Downloader::Reconnect);
    connect(speedUpdateTimer,    &QTimer::timeout, this, &Downloader::UpdateSpeedInfo);
    connect(downloadUpdateTimer, &QTimer::timeout, this, &Downloader::UpdateDownloadInfo);

    // Chrono
    chronoSize = 0;
    chrono->start();
    speedUpdateTimer->setSingleShot(true);
    speedUpdateTimer->start(FTP_SPEED_UPDATE_TIME);
    downloadUpdateTimer->setSingleShot(true);
    downloadUpdateTimer->start(FTP_DOWNLOAD_UPDATE_TIME);

    // Launch
    Login("");
}

/*--- Timeout has expired, restart the current file ---*/
void Downloader::Reconnect()
{
    qDebug() << "Downloader::Reconnect";

    // Destroy the FTP handler
    disconnect(timeoutTimer, &QTimer::timeout, this, &Downloader::Reconnect);
    disconnect(ftp, &QNetworkAccessManager::finished, this, &Downloader::FileFinished);
    ftp->deleteLater();

    // Restart
    ftp = new QNetworkAccessManager();
    connect(timeoutTimer, &QTimer::timeout, this, &Downloader::Reconnect);
    connect(ftp, &QNetworkAccessManager::finished, this, &Downloader::FileFinished);
    emit DownloadFile(currentFtpDir, currentFtpFile);
}

/*--- Use password to login ---*/
void Downloader::Login(QString pwd)
{
    qDebug() << "Downloader::Login";

    password = pwd;
    ftpClient->setPassword(password);
    DownloadFile("", FTP_RELEASE_NOTES_FILE);
}


/*----------------------------------------------
        File handling
----------------------------------------------*/

/*--- Prepare download, then issue the FTP commands ---*/
void Downloader::DownloadFile(QString dir, QString file)
{
    qDebug() << "Downloader::DownloadFile";

    // Data
    QUrl fileUrl(*ftpClient);
    QNetworkRequest r;
    QDir tempDir(".");

    // Setup
    downloadedSize = 0;
    lastDownloadedSize = 0;
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
    connect(reply, &QNetworkReply::readyRead, this, &Downloader::FilePart);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Downloader::FileError);

    // Timers
    timeoutTimer->setSingleShot(true);
    timeoutTimer->start(FTP_TIMEOUT);
}

/*--- Received on FTP part downloaded ---*/
void Downloader::FilePart(void)
{
    int count = currentFile->write(reply->readAll());
    timeoutTimer->stop();
    timeoutTimer->setSingleShot(true);
    timeoutTimer->start(FTP_TIMEOUT);
    downloadedSize += count;
    chronoSize += count;
}

/*--- Received when a command has failed ---*/
void Downloader::FileError(QNetworkReply::NetworkError code)
{
    qDebug() << "Downloader::FileError";

    switch (code)
    {
        case QNetworkReply::NoError:
            break;

        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
            emit NetworkError("Networking error (" + QString::number(code) + ")", true);
            break;

        case QNetworkReply::AuthenticationRequiredError:
            emit NetworkError("Password required", true);
            emit PasswordRequired();

            bDownloading = false;
            timeoutTimer->stop();
            speedUpdateTimer->stop();

            break;

        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::UnknownProxyError:
            emit NetworkError("Proxy error (" + QString::number(code) + ")", true);
            break;

        default:
            emit NetworkError("Server error (" + QString::number(code) + ")", true);
            break;
    }
}

/*--- Received when a download has ended ---*/
void Downloader::FileFinished(QNetworkReply* mreply)
{
    qDebug() << "Downloader::FileFinished";

    disconnect(reply, &QNetworkReply::readyRead, this, &Downloader::FilePart);
    disconnect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Downloader::FileError);
    mreply->deleteLater();
    lastDownloadedSize = 0;
    downloadedSize = 0;

    if (bDownloading)
	{
        bDownloading = false;
        timeoutTimer->stop();
		currentFile->close();
        delete currentFile;

        emit FileDownloaded(currentFtpFile);
    }
}


/*----------------------------------------------
        Downloading info
----------------------------------------------*/

/*--- Update current speed ---*/
void Downloader::UpdateSpeedInfo(void)
{
    currentSpeed = ((float)chronoSize / 1024.0) / ((float)chrono->restart() / 1000.0);
    chronoSize = 0;

    speedUpdateTimer->start(FTP_SPEED_UPDATE_TIME);
}

/*--- Update data about the current download ---*/
void Downloader::UpdateDownloadInfo(void)
{
    int sizeDelta = downloadedSize - lastDownloadedSize;
    emit BytesDownloaded(sizeDelta, currentSpeed);
    lastDownloadedSize = downloadedSize;

    downloadUpdateTimer->start(FTP_DOWNLOAD_UPDATE_TIME);
}
