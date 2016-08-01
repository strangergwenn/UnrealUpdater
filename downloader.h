/**
 *  This work is distributed under the Lesser General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

/*----------------------------------------------
    Includes & definitions
----------------------------------------------*/

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QTime>
#include <QUrl>

#include "common.h"


/*----------------------------------------------
	Class definitions
----------------------------------------------*/

class Downloader : public QObject
{
	Q_OBJECT

public:
	Downloader();
    ~Downloader();

public slots:

    // Connection handling
    void Connect(void);
    void Reconnect(void);
    void Login(QString pwd);

    // File handling
    void DownloadFile(QString dir, QString file);
    void FilePart(void);
    void FileError(QNetworkReply::NetworkError code);
    void FileFinished(QNetworkReply* mreply);

    // Downloading info
    void UpdateDownloadInfo(void);
    void UpdateSpeedInfo(void);

signals:

    // Downloader events
    void PasswordRequired(void);
    void BytesDownloaded(int, float);
    void FileDownloaded(QString downloadedFile);
    void NetworkError(QString message, bool bIsHeavy);

private:

    float   currentSpeed;
    bool    bDownloading;
    int     downloadedSize;
    int     lastDownloadedSize;
    int     chronoSize;
    QString password;
    QUrl*   ftpClient;
	QFile*  currentFile;
    QTime*  chrono;
    QTimer* timeoutTimer;
    QTimer* speedUpdateTimer;
    QTimer* downloadUpdateTimer;
	QString currentFtpDir;
	QString currentFtpFile;
    QNetworkReply* reply;
    QNetworkAccessManager* ftp;

};

#endif // DOWNLOADER_H
