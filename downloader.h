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
    void Connect(void);
    void Reconnect(void);
    void Login(QString pwd);
    void DownloadFile(QString dir, QString file);
    void FilePart(void);
    void FileError(QNetworkReply::NetworkError code);
    void FileFinished(QNetworkReply* mreply);

signals:
	void FileDownloaded(void);
	void BytesDownloaded(int);
    void ShowReleaseNotes(void);
    void AskForPassword(void);
    void PrintStreamedMessage(QString message);
    void PrintHeavyStreamedMessage(QString message);
	void PrintCurrentFile(QString fileName);
	void DownloadTreeFromManifest(QString fileName);

private:
    bool    bDownloading;
    int     downloadedSize;
    QString passWd;
    QUrl*   baseUrl;
	QFile*  currentFile;
	QTimer* timeout;
	QString currentFtpDir;
	QString currentFtpFile;
    QNetworkReply* reply;
    QNetworkAccessManager* ftp;

};

#endif // DOWNLOADER_H
