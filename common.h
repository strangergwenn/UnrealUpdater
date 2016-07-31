/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef COMMON_H
#define COMMON_H

/*----------------------------------------------
	     Includes
---------------------------------------------*/

#include <QMainWindow>
#include <QDialog>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDir>

#include "Res/project.h"


/*----------------------------------------------
	     Definitions
---------------------------------------------*/

#define ABOUT_TITLE                 "About"

#define USE_FILE_LOG                1
#define USE_PASSWORD                0

#define FTP_TIMEOUT                 (5000)
#define FTP_SPEED_UPDATE_TIME       (1000)
#define FTP_DOWNLOAD_UPDATE_TIME    (100)
#define FTP_PART_SIZE               (256 * 1024)
#define FTP_MANIFEST_ROOT           ""
#define FTP_MANIFEST_FILE           "GameManifest.xml"
#define FTP_RELEASE_NOTES_FILE      "ReleaseNotes.xml"
#define FTP_OLD_RELEASE_NOTES_FILE  "ReleaseNotes.old"

#define UE_INSTALLER_PATH           "Engine/Extras/Redist/en-us/UE4PrereqSetup_x64.exe"

#define S_CONFIG_DIR                "Config"
#define S_LOCK_FILE                 "Config/UU_Lock.setting"
#define S_DVL_INSTALLED             "Config/UU_Accepted.setting"
#define S_UE_INSTALLED              "Config/UU_Installed.setting"


/*----------------------------------------------
	     Custom types
---------------------------------------------*/

typedef struct File_tag {
    QString     dir;
    QString     file;
    int         size;
} File_t;

#endif // COMMON_H
