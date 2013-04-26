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

#include <QDialog>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDir>

#include "Res/project.h"
#include "windows-specific.h"


/*----------------------------------------------
	     Definitions
---------------------------------------------*/

#define ABOUT_TITLE                 "About"

#define FTP_TIMEOUT                 (1000)
#define FTP_PART_SIZE               (64 * 1024)
#define FTP_MANIFEST_ROOT           "Binaries/InstallData/"
#define FTP_MANIFEST_FILE           "GameManifest.xml"
#define FTP_RELEASE_NOTES_FILE      "ReleaseNotes.xml"
#define FTP_OLD_RELEASE_NOTES_FILE  "ReleaseNotes.old"

#define NET_INSTALLER_KEY_PATH      "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full"
#define NET_INSTALLER_PATH          "Binaries/InstallData/dotNetFx40_Full_setup.exe"

#define UDK_EXE_PATH_32             "Binaries/Win32/UDK.exe"

#define S_CONFIG_DIR                "Config"
#define S_LOCK_FILE                 "Config/UU_Lock.setting"
#define S_SERVER_FILE               "Config/UU_Dedicated.setting"
#define S_AUTOLAUNCH_FILE           "Config/UU_AutoLaunch.setting"
#define S_MAP_NAME                  "Config/UU_Map.setting"

#define USE_PASSWORD
#define USE_AERO_EFFECTS


/*----------------------------------------------
	     Custom types
---------------------------------------------*/

typedef struct File_tag {
    QString     dir;
    QString     file;
    int         size;
} File_t;

#endif // COMMON_H
