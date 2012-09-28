UnrealUpdater
=============

A C++ updater for UDK games, built with Qt4.

Overview
--------

UUpdater is a lightweight updater software built with Qt4, usable to deploy your UDK game simply over FTP, without additionnal work.
It was developped for the free FPS [DeepVoid](http://deepvoid.eu/ "DeepVoid"), and is now available for everyone to use. Right now, no major deployment has been made and the tool is still in development, use at your own risks.

When launched on client, it will trigger .NET 4 installation if not detected, since this is needed by UDK and packed in the standard installer.


User-side configuration files
--------

There are currently a few configuration files in the *Config/* folder, created on startup.
1.  *UU_Lock.setting* is generated, do not use.
2.  *UU_AutoLaunch.setting*, if existing, will trigger auto-launch of the game after updating. Delete it to abort this.
3.  *UU_Dedicated.setting*, if existing, will make the game launch on server mode using the configured map.
4.  *UU_Map.setting* contains the map name to launch on server mode, but you can very well add various parameters here for the UDK command line.


How to use
--------

Currently, the updater use a frehs installation of your game. This means that the first condition is that you build, cook and package your game with UnrealFrontend, then install (using the packaged installer) it in a new separated folder.
This process will generate a file tree that you can use here. Do not use this process on a UDK development folder.

1.  First, edit the image resources in the *Res/* folder to match your game's UI. Use the sample *Res_Sample/* folder and rename it.
2.  Edit *res/project.h* to set your about dialog and FTP data.
3.  Use QTCreator to build the game. You **should** use a static build of Qt so that the updater is a single file.
4.  Edit ReleaseNotes.xml on your server to update the description on the updater main window.
5.  Edit Binaries/InstallData/GameManifest.xml to add a "md5" field on each "FileProperties" node with the MD5 checksum of the file.
6.  You should really check that all of this works locally.
7.  Your updater is ready, feel free to rename it and distribute it.

**Beware : this is GPL'd software. You have to provide the sources (or a link to them on GitHub).**


ReleaseNotes.xml syntax
--------

`<ReleaseNotes>
	<Version>MajorRelease42</Version>
	<Date>DDMMYY</Date>
	<Notes>*Title*

	Text, text, text.</Notes>
	<NotUpdatedFile>UDKGame/Config/i-will-not-be-erased.ini</NotUpdatedFile>
</ReleaseNotes>`


GameManifest.xml syntax
--------

`<FolderProperties FolderName=".">
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
</FolderProperties>`


--------
Contact, people, etc

UnrealUpdater is developped by Gwennaël ARBONA.
In case of bugs, questions, requests, ideas, please use the message system on GitHub.
