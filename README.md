Unreal Updater
=============

A C++ updater for UE4 games, built with Qt5, licensed under LGPL. Other game engines might work too.

![Screenshot](https://raw.githubusercontent.com/arbonagw/UnrealUpdater/master/Launcher.jpg)

Overview
--------

Unreal Updater is a lightweight updater software, built with Qt5, to deploy your Unreal Engine game simply over anonymous FTP.  
It was developped for the free FPS [DeepVoid](http://deepvoid.eu/ "DeepVoid") and the space sim [Helium Rain](http://helium-rain.com), and is now available for everyone to use. 

Features :

 - game download and updating
 - consistency checking
 - installation of dependencies
 - download from anonymous FTP, with support for anonyous password

User-side configuration files
--------

There are currently a few configuration files in the *Config/* folder, created on startup.

1.  *UU_Lock.setting* is generated on start and removed on exit to prevent multiple instances. Remove it if the launcher doesn't start.
2.  *UU_Accepted.setting* is generated when the user accepts to install the game.
3.  *UU_Installed.setting* is generated when the game's dependencies have been installed.


How to use
--------

The updater use a fresh installation of your game - you need to have a full deployed package of your game. The process will generate a file tree that you can use with this tool. 

1.  Rename the *Res_Sample/* into *Res/* and edit the images to match your project's look.
2.  Edit *Res/project.h* to set your game name, FTP address, executable path...
3.  Use QTCreator to build the updater. You **should** use a static build of Qt so that the updater is a single file, [see this guide](http://stackoverflow.com/questions/14932315/how-to-compile-qt-5-under-windows-or-linux-32-or-64-bit-static-or-dynamic-on-v) to see how.
4.  Create a ReleaseNotes.xml on your server to update the description on the updater main window. See below.
5.  Create a GameManifest.xml on your server to tell the updater about the files you want. See below.
6.  Your updater is ready, feel free to rename it and distribute it.


ReleaseNotes.xml syntax
--------

You can edit the file by hand.

	<ReleaseNotes>
		<Version>MajorRelease42</Version>
		<Date>DDMMYY</Date>
		<Notes>*Title*
	
		Text, text, text.</Notes>
	</ReleaseNotes>


GameManifest.xml syntax
--------

You should probably use a tool for this. The syntax is very easy, but you will probably have a lot of files here. You can use my other tool, [Unreal Manifest](https://github.com/arbonagw/UnrealManifest) for this.

	<FolderProperties FolderName=".">
		<Folders>
			<FolderProperties FolderName="FolderA">
				<Folders/>
				<Files>
					<FileProperties FileName="FileA" Size="889416" md5="53406e9988306cbd4537677c5336aba4"/>
					<FileProperties FileName="FileB" Size="45014" md5="54498899854584bd4534584bd336a544"/>
				</Files>
			</FolderProperties>
		</Folders>
		<Files/>
	</FolderProperties>


Contact, people, etc
--------

Unreal Updater is developped by Gwennaël ARBONA.  
