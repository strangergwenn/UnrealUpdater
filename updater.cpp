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
    bDownloadPart = false;
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
    dlObject->moveToThread(dlThread);

    // Signals - From UI
    connect(ui->about, SIGNAL(clicked()), this, SLOT(AboutMe()));
    connect(ui->launchGame, SIGNAL(clicked()), this, SLOT(LaunchGame()));
    connect(ui->launchServer, SIGNAL(clicked()), this, SLOT(LaunchServer()));

    // Signals - From downloader
    connect(dlObject, SIGNAL(AskForPassword()), this, SLOT(AskForPassword()));
    connect(dlObject, SIGNAL(Stage1()), this, SLOT(Stage1()));
    connect(dlObject, SIGNAL(Stage2()), this, SLOT(Stage2()));
    connect(dlObject, SIGNAL(BytesDownloaded(int)), this, SLOT(BytesDownloaded(int)));
    connect(dlObject, SIGNAL(FileDownloaded()), this, SLOT(FileDownloaded()));
    connect(dlObject, SIGNAL(Log(QString, bool)), this, SLOT(Log(QString, bool)));

    // Signals - To downloader
    connect(dlThread, SIGNAL(started()), dlObject, SLOT(Connect()));
    connect(dlThread, SIGNAL(finished()), dlThread, SLOT(deleteLater()));
    connect(this, SIGNAL(DownloadFile(QString, QString)), dlObject, SLOT(DownloadFile(QString, QString)));

    // Launch
    SetUserMessage("Downloading release notes");
    dlThread->start();
}

Updater::~Updater()
{
	delete ui;
	SetLock(false);
}


/*----------------------------------------------
           Updating slots
----------------------------------------------*/

/*--- Stage 1 : parse the release notes, then download the manifest ---*/
void Updater::Stage1(void)
{
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
    connect(watcher, SIGNAL(finished()), this, SLOT(Stage3()));
    QFuture<void> parser = QtConcurrent::run(this, &Updater::ParseManifest, *dom, QString(""));
    watcher->setFuture(parser);
}

/*--- Stage 3 : download the files if some are needed ---*/
void Updater::Stage3(void)
{
    // XML is ready
    delete dom;
    remainingFiles = filesToDownload.size();
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
    else if (filesToDownload.size() == 0)
    {
        ui->downloadProgress->setRange(0, 100);
        ui->downloadProgress->setValue(100);

        InstallNetFramework();
        InstallVC2010();
        InstallUE3();
        SetUserMessage("Ready to play !");

        ui->launchGame->setEnabled(true);
        ui->launchServer->setEnabled(true);
    }
}

/*--- Received on FTP file downloaded ---*/
void Updater::FileDownloaded(void)
{
    emit Log(currentFd.dir + currentFd.file, false);

    // Download in progress
    if (filesToDownload.size() > 0)
    {
        remainingFiles--;
        currentFd = filesToDownload.takeFirst();
        emit DownloadFile(currentFd.dir, currentFd.file);
    }

    // Download complete : check for corruptions and run
    else
    {
        emit Log("Download complete", false);
        filesToDownload.clear();
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
#ifndef USE_FILE_LOG
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

/*--- Received on FTP part received ---*/
void Updater::BytesDownloaded(int number)
{
    QString message;

    downloadedBytes += number;
    bDownloadPart = !bDownloadPart;
    ui->downloadProgress->setValue(downloadedBytes);

    message = "Downloading " + QString::number(remainingFiles);
    message += (remainingFiles > 1) ? " files " : "file ";
    if (downloadSize - downloadedBytes > 1024*1024)
    {
        message += "(" + QString::number((downloadSize - downloadedBytes) / (1024*1024)) + " MB) ";
    }
    message += "at " + QString::number((int)(dlObject->GetCurrentSpeed())) + " KB/s";

    SetUserMessage(message);
}

/*--- Launch UDK, exit updater ---*/
void Updater::LaunchGame(void)
{
    QProcess udk(this);
    udk.startDetached(UDK_EXE_PATH_32);
    udk.waitForStarted();
    QApplication::quit();
}

/*--- Launch UDK server, exit updater ---*/
void Updater::LaunchServer(void)
{
    ServerConfig w;
    hide();
    w.exec();
    QApplication::quit();
}

/*--- Print the about dialog ---*/
void Updater::AboutMe(void)
{
    About* aboutDialog = new About(this);
	aboutDialog->show();
}

/*--- Relaunch the connecting process with a password prompt ---*/
void Updater::AskForPassword()
{
    Password* pwdDialog = new Password(this);
    connect(pwdDialog, SIGNAL(PasswordEntered(QString)), dlObject, SLOT(Login(QString)));
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

/*--- Check if .NET needs to be installed, then install it if needed ---*/
void Updater::InstallNetFramework(void)
{
    QStringList argList;
    QProcess netInstaller(this);
    QSettings setting(NET_INSTALLER_KEY_PATH, QSettings::NativeFormat);

    if (!setting.value("Install").toBool())
    {
        SetUserMessage("Installing .NET Framework");
        argList << "/passive";
        argList << "/norestart";
        netInstaller.startDetached(NET_INSTALLER_PATH, argList);
        netInstaller.waitForFinished();
    }
}

/*--- Check if the UE3 redistributables have been installed (DX) ---*/
void Updater::InstallUE3(void)
{
    QProcess ueInstaller(this);

    if (!GetSettingState(S_UE_INSTALLED))
    {
        SetUserMessage("Installing UE3 redistributable");
        ueInstaller.startDetached(UE_INSTALLER_PATH);
        ueInstaller.waitForFinished();
        SetSettingState(true, S_UE_INSTALLED);
    }
}

/*--- Check if VS2010 needs to be installed, then install it if needed ---*/
void Updater::InstallVC2010(void)
{
    QStringList argList;
    QProcess vcInstaller(this);
    QSettings setting(VC_INSTALLER_KEY_PATH, QSettings::NativeFormat);

    if (!setting.value("Installed").toBool())
    {
        SetUserMessage("Installing VC2010 redistributable");
        argList << "/passive";
        argList << "/norestart";
        vcInstaller.startDetached(VC_INSTALLER_PATH, argList);
        vcInstaller.waitForFinished();
    }
}
