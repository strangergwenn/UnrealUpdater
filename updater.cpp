/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#include "updater.h"
#include "password.h"
#include "ui_updater.h"


/*----------------------------------------------
	  Constructor & destructor
----------------------------------------------*/

Updater::Updater(QWidget *parent) :
    QMainWindow(parent),
    filesToDownload(QList<File_t> ()),
    ui(new Ui::Updater)
{
    // Configuration
    bAbortUpdate = false;
    SetSettingState(true, S_DVL_INSTALLED);

	// UI settings
    SetLock(true);
    ui->setupUi(this);
    setWindowTitle(WINDOW_TITLE);

    // Downloader settings
	downloadSize = 0;
    downloadedBytes = 0;
    dlThread = new QThread;
    dlObject = new Downloader();
    aboutDialog = new About(this);
    dlObject->moveToThread(dlThread);

    // Signals - From UI
    connect(ui->about, &QPushButton::clicked, this, &Updater::AboutMe);
    connect(ui->launchGame, &QPushButton::clicked, this, &Updater::LaunchGame);

    // Signals - From downloader
    connect(dlObject, &Downloader::Stage1, this, &Updater::Stage1);
    connect(dlObject, &Downloader::Stage2, this, &Updater::Stage2);
    connect(dlObject, &Downloader::BytesDownloaded, this, &Updater::BytesDownloaded);
    connect(dlObject, &Downloader::FileDownloaded, this, &Updater::FileDownloaded);
    connect(dlObject, &Downloader::Log, this, &Updater::Log);

    // Signals - To downloader
    connect(dlThread, &QThread::started, dlObject, &Downloader::Connect);
    connect(dlThread, &QThread::finished, dlThread, &QThread::deleteLater);
    connect(this, &Updater::DownloadFile, dlObject, &Downloader::DownloadFile);

    // Launch
    SetUserMessage("Downloading release notes");
    dlThread->start();
}

Updater::~Updater()
{
    SetLock(false);

	delete ui;
    delete dlThread;
    delete aboutDialog;
}


/*----------------------------------------------
           Updating slots
----------------------------------------------*/

/*--- Stage 1 : parse the release notes, then download the manifest ---*/
void Updater::Stage1(void)
{
    qDebug() << "Updater::Stage1";

    // New version : use downloaded manifest
	QDomDocument* dom = new QDomDocument("notes");
	QFile* file = new QFile(FTP_RELEASE_NOTES_FILE);
	if(file->open(QFile::ReadOnly))
	{
		dom->setContent(file);
		file->close();
        ParseReleaseNotes(*dom, false);
	}
	delete file;

	// Old version
	file = new QFile(FTP_OLD_RELEASE_NOTES_FILE);
	if(file->open(QFile::ReadOnly))
	{
		dom->setContent(file);
		file->close();
        ParseReleaseNotes(*dom, true);
	}
	delete file;

	// Ready to go
    SetUserMessage("Downloading manifest");
	delete dom;
}

/*--- Stage 2 : parse the manifest, then start the actual download ---*/
void Updater::Stage2(void)
{
    qDebug() << "Updater::Stage2";

    // Update preparation
    dom = new QDomDocument("files");
    QFile* file = new QFile(QString(FTP_MANIFEST_ROOT) + QString(FTP_MANIFEST_FILE));
    if(file->open(QFile::ReadOnly))
    {
        dom->setContent(file);
        file->close();
    }
    delete file;

    // Parse the XML and signal us back when done
    SetUserMessage("Checking local files");
    filesToDownload.clear();
    QFutureWatcher<void>* watcher = new QFutureWatcher<void>();
    connect(watcher, &QFutureWatcher<void>::finished, this, &Updater::Stage3);
    QFuture<void> parser = QtConcurrent::run(this, &Updater::ParseManifest, *dom, QString(""));
    watcher->setFuture(parser);
}

/*--- Stage 3 : download the files if some are needed ---*/
void Updater::Stage3(void)
{
    qDebug() << "Updater::Stage3";

    // XML is ready
    delete dom;
    ui->downloadProgress->setRange(0, downloadSize);
    SetUserMessage("Downloading");

    // Send first file to download
    if (!bAbortUpdate && filesToDownload.size() > 0)
    {
        currentFd = filesToDownload.takeFirst();
        emit DownloadFile(currentFd.dir, currentFd.file);
        return;
    }

    // File tree is OK
    else
    {
        ui->downloadProgress->setRange(0, 100);
        ui->downloadProgress->setValue(100);

        InstallUrealRedist();
        SetUserMessage("Ready to play !");

        ui->launchGame->setEnabled(true);
    }
}

/*--- Received on FTP file downloaded ---*/
void Updater::FileDownloaded(void)
{
    emit Log(currentFd.dir + currentFd.file, false);

    // Download in progress
    if (filesToDownload.size() > 0)
    {
        currentFd = filesToDownload.takeFirst();
        emit DownloadFile(currentFd.dir, currentFd.file);
    }

    // Download complete : check for corruptions and run
    else
    {
        filesToDownload.clear();
        ui->downloadProgress->setRange(0, 100);
        ui->downloadProgress->setValue(100);
        emit Stage2();
    }
}


/*----------------------------------------------
                UI slots
----------------------------------------------*/

/*--- Append something to the list of downloaded files ---*/
void Updater::Log(QString message, bool bIsHeavy)
{
    if (bIsHeavy)
    {
        message = QString(HTML_HEAVY_S) + message + QString(HTML_HEAVY_E);
    }
    else
    {
#if !USE_FILE_LOG
        return;
#endif
    }
    ui->streamedMessages->append(message);
}

/*--- Print the status message ---*/
void Updater::SetUserMessage(QString message)
{
	QString userInfo("");
	if (currentVersion.length() > 0)
	{
        userInfo += QString(HTML_HEAVY_S) + currentVersion + QString(HTML_HEAVY_E) + " | ";
	}
    userInfo += QString(HTML_HEAVY_S) + message + QString(HTML_HEAVY_E);
	ui->userInformation->setText(userInfo);
}

/*--- Received when the downloader updates the download status ---*/
void Updater::BytesDownloaded(int deltaBytes, float downloadSpeed)
{
    if (deltaBytes != 0)
    {
        downloadedBytes += deltaBytes;
        ui->downloadProgress->setValue(downloadedBytes);
        int remainingFiles = filesToDownload.size() + 1;

        QString message;
        message = "Downloading " + QString::number(remainingFiles);
        message += (remainingFiles > 1) ? " files " : " file ";

        if (downloadSize - downloadedBytes > 1024*1024)
        {
            message += "(" + QString::number((downloadSize - downloadedBytes) / (1024*1024)) + " MB) ";
        }
        else
        {
            message += "(" + QString::number((downloadSize - downloadedBytes) / (1024)) + " KB) ";
        }

        if (downloadSpeed > 1024)
        {
            message += "at " + QString::number(downloadSpeed / 1024.0f, 10, 2) + " MB/s";
        }
        else
        {
            message += "at " + QString::number(downloadSpeed, 10, 2) + " KB/s";
        }

        SetUserMessage(message);
    }
}

/*--- Launch Unreal, exit updater ---*/
void Updater::LaunchGame(void)
{
    QProcess unreal(this);
    unreal.startDetached(UNREAL_EXECUTABLE);
    unreal.waitForStarted();
    QApplication::quit();
}

/*--- Print the about dialog ---*/
void Updater::AboutMe(void)
{
	aboutDialog->show();
}

/*--- Relaunch the connecting process with a password prompt ---*/
void Updater::AskForPassword()
{
    Password* pwdDialog = new Password(this);
    connect(pwdDialog, &Password::PasswordEntered, dlObject, &Downloader::Login);
    pwdDialog->exec();
}


/*----------------------------------------------
           Update methods
----------------------------------------------*/

/*--- Analyze release notes, store data and display info ---*/
void Updater::ParseReleaseNotes(QDomNode node, bool bIsCurrent)
{
	QDomNode n = node.firstChild();
	QString tmp("");
	QDate tmpDate;

	while (!n.isNull())
	{
		if (n.hasChildNodes())
		{
			tmp = n.toElement().text();

			// Version ID
			if (n.nodeName() == "Version")
			{
				if (bIsCurrent)
					currentVersion = tmp;
				else
					nextVersion = tmp;
			}

			// Update in progress on server : abort !
			if (n.nodeName() == "UpdateInProgress" && !bIsCurrent)
			{
				if (tmp == "1")
				{
                    Log("New release on server, please try again later", true);
					bAbortUpdate = true;
				}
			}

			// Date
			else if (n.nodeName() == "Date")
			{
				tmpDate = QDate::fromString(tmp, "ddMMyy");
				if (bIsCurrent)
					currentDate = tmpDate;
				else
					nextDate = tmpDate;
			}

			// Ignore list
			else if (n.nodeName() == "NotUpdatedFile" && !bIsCurrent)
			{
				notUpdatedFiles.append(n.toElement().text());
			}

			// Release notes text
			else if (n.nodeName() == "Notes" && !bIsCurrent)
			{
				ui->releaseNotes->setHtml(TextToHtml(n.toElement().text()));
			}

			// Recursive call
			else
			{
                ParseReleaseNotes(n, bIsCurrent);
			}
		}
		n = n.nextSibling();
	}
}

/*--- Download a XML node content (recursively) ---*/
void Updater::ParseManifest(QDomNode node, QString dirName)
{
    QDomNode n = node.firstChild();
    QString fileName;
    QString fileHash;
    QString recDir;
    QFile* tempFile;
    File_t tempFileData;
    bool bShouldDownload;

    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            // Node : recursive call
            if (n.hasChildNodes())
            {
                recDir = dirName;
                if (e.tagName() == "FolderProperties" && e.attribute("FolderName", "") != ".")
                {
                    recDir += e.attribute("FolderName", "") + "/";
                }
                ParseManifest(n, recDir);
            }

            // Element : direct download
            else
            {
                if (e.tagName() == "FileProperties")
                {
                    bShouldDownload = false;
                    fileName = e.attribute("FileName", "");
                    tempFile = new QFile(dirName + fileName);

                    if (!tempFile->exists())
                    {
                        bShouldDownload = true;
                    }
                    else if (e.attribute("md5", "").length() > 0 && !notUpdatedFiles.contains(fileName))
                    {
                        fileHash = HashFile(tempFile);
                        if (fileHash != e.attribute("md5", "") && fileHash.length() > 0)
                        {
                            bShouldDownload = true;
                        }
                    }

                    if (bShouldDownload)
                    {
                        tempFileData.dir = dirName;
                        tempFileData.file = fileName;
                        tempFileData.size = e.attribute("Size", "").toInt();
                        filesToDownload.append(tempFileData);
                        downloadSize += tempFileData.size;
                    }
                    delete tempFile;
                }
            }
        }
        n = n.nextSibling();
    }
}


/*----------------------------------------------
           Utilitary methods
----------------------------------------------*/

/*--- Get a MD5 hash from a file on disk ---*/
QString Updater::HashFile(QFile* file)
{
	QByteArray hashed;
	if (file->exists() && file->open(QIODevice::ReadOnly))
	{
		QByteArray content = file->readAll();
		hashed = QCryptographicHash::hash(content, QCryptographicHash::Md5);
		file->close();
	}
	return hashed.toHex().data();
}

/*--- Write the lock file ---*/
void Updater::SetLock(bool bLockState)
{
	SetSettingState(bLockState, S_LOCK_FILE);
}

/*--- Generic, basic setting API : setter ---*/
void Updater::SetSettingState(bool bState, QString settingName)
{
	if (bState)
	{
		QFile settingFile(settingName);
		settingFile.open(QFile::WriteOnly);
		settingFile.close();
	}
	else
	{
		QFile::remove(settingName);
	}
}

/*--- Generic, basic setting API : getter---*/
bool Updater::GetSettingState(QString settingName)
{
	return QFile::exists(settingName);
}

/*--- Check if the UE4 redistributables have been installed ---*/
void Updater::InstallUrealRedist(void)
{
    QProcess ueInstaller(this);

    if (!GetSettingState(S_UE_INSTALLED))
    {
        SetUserMessage("Installing UE4 redistributables");
        ueInstaller.startDetached(UE_INSTALLER_PATH);
        ueInstaller.waitForFinished();
        SetSettingState(true, S_UE_INSTALLED);
    }
}
