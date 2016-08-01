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
    delete chrono;
    delete timeoutTimer;
    delete speedUpdateTimer;
    delete downloadUpdateTimer;
}


/*----------------------------------------------
		   Slots
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
    baseUrl = new QUrl();
    baseUrl->setScheme("ftp");
    baseUrl->setPort(21);
    baseUrl->setHost(FTP_SERVER);
    baseUrl->setUserName(FTP_USER);

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
#if USE_PASSWORD
    emit AskForPassword();
#else
    Login("");
#endif
}

/*--- Timeout has expired, restart file ---*/
void Downloader::Reconnect()
{
    qDebug() << "Downloader::Reconnect";

    // Suppress the FTP handler
    emit Log("Timeout downloading " + currentFtpFile, false);
    disconnect(timeoutTimer, &QTimer::timeout, this, &Downloader::Reconnect);
    disconnect(ftp, &QNetworkAccessManager::finished, this, &Downloader::FileFinished);
    ftp->deleteLater();

    // Restart it
    ftp = new QNetworkAccessManager();
    connect(timeoutTimer, &QTimer::timeout, this, &Downloader::Reconnect);
    connect(ftp, &QNetworkAccessManager::finished, this, &Downloader::FileFinished);
    emit DownloadFile(currentFtpDir, currentFtpFile);
}

/*--- Use password to login ---*/
void Downloader::Login(QString pwd)
{
    qDebug() << "Downloader::Login";

    passWd = pwd;
    baseUrl->setPassword(passWd);
    DownloadFile("", FTP_RELEASE_NOTES_FILE);
}

/*--- Prepare download, then issue the FTP commands ---*/
void Downloader::DownloadFile(QString dir, QString file)
{
    qDebug() << "Downloader::DownloadFile " + dir + file;

    // Data
    QUrl fileUrl(*baseUrl);
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

/*--- Received when a command has failed ---*/
void Downloader::FileError(QNetworkReply::NetworkError code)
{
    qDebug() << "Downloader::FileError on " + currentFtpDir + currentFtpFile;

    switch (code)
    {
        case QNetworkReply::NoError:
            break;

        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TimeoutError:
            qDebug() << "Downloader::FileError : network error " + QString::number(code);
            Log("Networking error (" + QString::number(code) + ")", true);
            break;

        case QNetworkReply::AuthenticationRequiredError:
            qDebug() << "Downloader::FileError : authentication error " + QString::number(code);
#if USE_PASSWORD
            Log("Invalid password", true);
#else
            Log("Password is required", true);
#endif
            Log("This IP address will be temporary banned after too many invalid passwords", true);
            timeoutTimer->stop();
            speedUpdateTimer->stop();
#if USE_PASSWORD
            disconnect(timeout, &QTimer::timeout, this, &Downloader::Reconnect);
            disconnect(ftp, &QNetworkAccessManager::finished, this, &Downloader::FileFinished);
            ftp->deleteLater();
            timeout->deleteLater();
            speedUpdateTimer->deleteLater();
            emit Connect();
#endif
            break;

        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::UnknownProxyError:
            qDebug() << "Downloader::FileError : proxy error " + QString::number(code);
            Log("Proxy error (" + QString::number(code) + ")", true);
            break;

        default:
            qDebug() << "Downloader::FileError : server error " + QString::number(code);
            Log("Server error (" + QString::number(code) + ")", true);
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
