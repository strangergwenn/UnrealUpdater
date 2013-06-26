/**
 *  This work is distributed under the General Public License,
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
    float GetCurrentSpeed();

public slots:
    void Connect(void);
    void Reconnect(void);
    void Login(QString pwd);
    void DownloadFile(QString dir, QString file);
    void FilePart(void);
    void UpdateSpeed(void);
    void FileError(QNetworkReply::NetworkError code);
    void FileFinished(QNetworkReply* mreply);

signals:
    void AskForPassword(void);
    void Stage1(void);
    void Stage2(void);
    void BytesDownloaded(int);
    void FileDownloaded(void);
    void SetCurrentFile(QString fileName);
    void Log(QString message, bool bIsHeavy);

private:
    float   currentSpeed;
    bool    bDownloading;
    int     downloadedSize;
    int     chronoSize;
    QString passWd;
    QUrl*   baseUrl;
	QFile*  currentFile;
    QTime*  chrono;
    QTimer* timeout;
    QTimer* chronoTimeout;
	QString currentFtpDir;
	QString currentFtpFile;
    QNetworkReply* reply;
    QNetworkAccessManager* ftp;

};

#endif // DOWNLOADER_H
