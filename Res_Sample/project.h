/**
 *  This work is distributed under the Lesser General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/


/*----------------------------------------------
         Content settings
---------------------------------------------*/

// Title for the main window
#define WINDOW_TITLE                "Unreal Updater"

// Title for the "About" window
#define ABOUT_TITLE                 "About"

/*--- Content of about dialog ---
 *text* will be a title
 !text! will be italic
 _text_ will be bold
*/
#define ABOUT_CONTENT \
"*About Unreal Updater*\n \
\n \
This game updater is open-source software under the LGPL license.\n \
 You can find the sources on GitHub : https://github.com/arbonagw/UnrealUpdater\n \
\n \
_Gwenna&euml;l ARBONA_"

/*----------------------------------------------
         Path settings
---------------------------------------------*/

// Installer executable
#define GAME_INSTALLER_EXECUTABLE   "Engine/Extras/Redist/en-us/UE4PrereqSetup_x64.exe"

// Game executable
#define GAME_EXECUTABLE             "Game.exe"

// Config files used by Unreal Updater
#define UU_CONFIG_DIR               "Config"
#define UU_LOCK_FILE                "Config/UU_Lock.setting"
#define UU_UPDATER_INSTALLED        "Config/UU_Accepted.setting"
#define UU_GAME_INSTALLED           "Config/UU_Installed.setting"


/*----------------------------------------------
         FTP settings
---------------------------------------------*/

// FTP server data for connexion
#define FTP_SERVER                  "yoursite.com"
#define FTP_USER                    "anonymous"
#define FTP_UPDATE_ROOT             "/"

// FTP performance parameters
#define FTP_TIMEOUT                 (5000)
#define FTP_SPEED_UPDATE_TIME       (1000)
#define FTP_DOWNLOAD_UPDATE_TIME    (100)
#define FTP_PART_SIZE               (256 * 1024)

// Manifest files
#define FTP_MANIFEST_ROOT           ""
#define FTP_MANIFEST_FILE           "GameManifest.xml"
#define FTP_RELEASE_NOTES_FILE      "ReleaseNotes.xml"
#define FTP_OLD_RELEASE_NOTES_FILE  "ReleaseNotes.old"


/*----------------------------------------------
        Customization
---------------------------------------------*/

/*--- Colors and fonts for all the interface ---*/
#define HTML_TITLE_S                "<b><font size=\"3\" color=\"lightskyblue\">"
#define HTML_TITLE_E                "</font></b>"
#define HTML_HEAVY_S                "<b><font color=\"lightskyblue\">"
#define HTML_HEAVY_E                "</font></b>"
#define HTML_MEDIUM_S               "<b>"
#define HTML_MEDIUM_E               "</b>"
#define HTML_IT_S                   "<i>"
#define HTML_IT_E                   "</i>"

