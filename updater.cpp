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
	ui(new Ui::Updater)
{
	// Configuration
	SetLock(true);
	bAbortUpdate = false;
	bDownloadPart = false;
	bServerMode = GetSettingState(S_SERVER_FILE);
	bAutoLaunch = GetSettingState(S_AUTOLAUNCH_FILE);

	// UI settings
	ui->setupUi(this);
	ui->isServer->setCheckState(bServerMode? Qt::Checked : Qt::Unchecked);
	ui->isAutoLaunch->setCheckState(bAutoLaunch? Qt::Checked : Qt::Unchecked);
	setWindowTitle(WINDOW_TITLE);
	#ifdef USE_AERO_EFFECTS
		SetupAeroEffects(this);
	#endif

    // Downloader settings
	downloadSize = 0;
    downloadedBytes = 0;
    dlThread = new QThread;
    dlObject = new Downloader();
    dlObject->moveToThread(dlThread);

    // Signals - From UI
    connect(ui->about, SIGNAL(clicked()), this, SLOT(AboutMe()));
    connect(ui->launchGame, SIGNAL(clicked()), this, SLOT(LaunchGame()));
	connect(ui->isServer, SIGNAL(stateChanged(int)), this, SLOT(SetServerMode(int)));
	connect(ui->isAutoLaunch, SIGNAL(stateChanged(int)), this, SLOT(SetAutoLaunch(int)));

    // Signals - From downloader
    connect(dlObject, SIGNAL(DownloadTreeFromManifest(QString)), this, SLOT(DownloadTreeFromManifest(QString)));
    connect(dlObject, SIGNAL(PrintCurrentFile(QString)), this, SLOT(PrintCurrentFile(QString)));
    connect(dlObject, SIGNAL(BytesDownloaded(int)), this, SLOT(BytesDownloaded(int)));
    connect(dlObject, SIGNAL(FileDownloaded()), this, SLOT(FileDownloaded()));
    connect(dlObject, SIGNAL(ShowReleaseNotes()), this, SLOT(ShowReleaseNotes()));
    connect(dlObject, SIGNAL(AskForPassword()), this, SLOT(AskForPassword()));
    connect(dlObject, SIGNAL(PrintStreamedMessage(QString)), this, SLOT(PrintStreamedMessage(QString)));
    connect(dlObject, SIGNAL(PrintHeavyStreamedMessage(QString)), this, SLOT(PrintHeavyStreamedMessage(QString)));

    // Signals - To downloader
    connect(dlThread, SIGNAL(started()), dlObject, SLOT(Connect()));
    connect(dlThread, SIGNAL(finished()), dlThread, SLOT(deleteLater()));
    connect(this, SIGNAL(DownloadFile(QString, QString)), dlObject, SLOT(DownloadFile(QString, QString)));

    // Downloader start-up
    PrintUserMessage("downloading release notes");
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

/*--- On release notes download, launch the next steps ---*/
void Updater::ShowReleaseNotes(void)
{
    // New version : use downloaded manifest
	QDomDocument* dom = new QDomDocument("notes");
	QFile* file = new QFile(FTP_RELEASE_NOTES_FILE);
	if(file->open(QFile::ReadOnly))
	{
		dom->setContent(file);
		file->close();
		FormatReleaseNotes(*dom, false);
	}
	delete file;

	// Old version
	file = new QFile(FTP_OLD_RELEASE_NOTES_FILE);
	if(file->open(QFile::ReadOnly))
	{
		dom->setContent(file);
		file->close();
		FormatReleaseNotes(*dom, true);
	}
	delete file;

	// Ready to go
	PrintUserMessage("downloading file tree");
	delete dom;
}

/*--- Parse the XML asynchronously ---*/
void Updater::DownloadTreeFromManifest(QString fileName)
{
    // Update preparation
    dom = new QDomDocument("files");
    QFile* file = new QFile(fileName);
    if(file->open(QFile::ReadOnly))
    {
        dom->setContent(file);
        file->close();
    }
    delete file;

    // Parse the XML and signal us back when done
    filesToDownload.clear();
    QFutureWatcher<void>* watcher = new QFutureWatcher<void>();
    connect(watcher, SIGNAL(finished()), this, SLOT(StartDownload()));
    QFuture<void> parser = QtConcurrent::run(this, &Updater::GetFilesToDownload, *dom, QString(""));
    watcher->setFuture(parser);
}

/*--- Download the whole XML content ---*/
void Updater::StartDownload(void)
{
    // XML is ready
    delete dom;
    ui->downloadProgress->setRange(0, downloadSize);
    PrintUserMessage("downloading " + nextVersion + " (" + QString::number(downloadSize / (1024*1024)) + " MB)");
    PrintStreamedIfNotNull("files need downloading", filesToDownload);

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
        ui->launchGame->setEnabled(true);

        PrintUserMessage("up to date");
        InstallNetFramework();

        if (bAutoLaunch)
        {
            LaunchGame();
        }
    }
}

/*--- Received on FTP file downloaded ---*/
void Updater::FileDownloaded(void)
{
    PrintStreamedMessage(currentFd.dir + currentFd.file);

    // Download in progress
    if (filesToDownload.size() > 0)
    {
        currentFd = filesToDownload.takeFirst();
        emit DownloadFile(currentFd.dir, currentFd.file);
    }

    // Download complete : check for corruptions and run
    else
    {
        PrintHeavyStreamedMessage("Download complete");
        filesToDownload.clear();
        DownloadTreeFromManifest(QString(FTP_MANIFEST_ROOT) + QString(FTP_MANIFEST_FILE));
    }
}


/*----------------------------------------------
                UI slots
----------------------------------------------*/

/*--- Append something to the list of downloaded files ---*/
void Updater::PrintStreamedMessage(QString message)
{
    ui->streamedMessages->append(message);
}

/*--- Append something to the list of downloaded files in bold ---*/
void Updater::PrintHeavyStreamedMessage(QString message)
{
    PrintStreamedMessage(QString(HTML_HEAVY_S) + message + QString(HTML_HEAVY_E));
}

/*--- Check if the list is not null and display a message including count ---*/
void Updater::PrintStreamedIfNotNull(QString message, QList<File_t> list)
{
    int size = list.size();
    if (size > 0)
    {
        PrintHeavyStreamedMessage(QString::number(size) + " " + message);
    }
}

/*--- Display the file currently being downloaded ---*/
void Updater::PrintCurrentFile(QString fileName)
{
	QString text = fileName + ((bDownloadPart && fileName.length() > 0) ? "  >":"");
	ui->currentFile->setText(text);
}

/*--- Print the status message ---*/
void Updater::PrintUserMessage(QString message)
{
	QString userInfo("");
	if (currentVersion.length() > 0)
	{
		userInfo += "Current version : " + QString(HTML_HEAVY_S) + currentVersion + QString(HTML_HEAVY_E);
		userInfo += " | Updated ";
		userInfo += QString(HTML_HEAVY_S) + currentDate.toString("dd/MM/yy") + QString(HTML_HEAVY_E) + " | ";
	}
	userInfo += "Status : " + QString(HTML_HEAVY_S) + message + QString(HTML_HEAVY_E);
	ui->userInformation->setText(userInfo);
}

/*--- Received on FTP part received ---*/
void Updater::BytesDownloaded(int number)
{
    downloadedBytes += number;
    bDownloadPart = !bDownloadPart;
    ui->downloadProgress->setValue(downloadedBytes);
}

/*--- Set the server mode ---*/
void Updater::SetServerMode(int bNewState)
{
	bServerMode = (bNewState == Qt::Checked);
	SetSettingState((bNewState == Qt::Checked), S_SERVER_FILE);
}

/*--- Set the auto-launch mode ---*/
void Updater::SetAutoLaunch(int bNewState)
{
	bAutoLaunch = (bNewState == Qt::Checked);
	SetSettingState((bNewState == Qt::Checked), S_AUTOLAUNCH_FILE);
}

/*--- Launch UDK, exit updater ---*/
void Updater::LaunchGame(void)
{
	QStringList argList;
	QProcess udk(this);
	QFile mapName(S_MAP_NAME);

	if (bServerMode)
	{
		mapName.open(QFile::ReadOnly);
		argList << "server";
		argList << mapName.readAll() + "?dedicated=true";
		mapName.close();
	}

	udk.startDetached(UDK_EXE_PATH_32, argList);
	udk.waitForStarted();
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
void Updater::FormatReleaseNotes(QDomNode node, bool bIsCurrent)
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
                    PrintHeavyStreamedMessage("New release on server, please try again later");
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
				FormatReleaseNotes(n, bIsCurrent);
			}
		}
		n = n.nextSibling();
	}
}

/*--- Download a XML node content (recursively) ---*/
void Updater::GetFilesToDownload(QDomNode node, QString dirName)
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
                GetFilesToDownload(n, recDir);
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
        PrintHeavyStreamedMessage("Installing .NET Framework");
		argList << "/passive";
		argList << "/norestart";
		netInstaller.startDetached(NET_INSTALLER_PATH, argList);
		netInstaller.waitForFinished();
        PrintHeavyStreamedMessage("Installation done");
	}
}
