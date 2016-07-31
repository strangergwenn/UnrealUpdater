Unreal Updater
=============

A C++ updater for UE4 games, built with Qt5.

Overview
--------

Unreal Updater is a lightweight updater software, built with Qt5, to deploy your Unreal Engine game simply over anonymous FTP.  
It was developped for the free FPS [DeepVoid](http://deepvoid.eu/ "DeepVoid") and the space sim [Helium Rain](http://helium-rain.com), and is now available for everyone to use. When launched on client, it will download the game, check its consistency and install the UE4 redistributables before running the game.

Right now, no major deployment has been made and the tool is still in development, use at your own risks.  

User-side configuration files
--------

There are currently a few configuration files in the *Config/* folder, created on startup.

1.  *UU_Lock.setting* is generated on start and removed on exit to prevent multiple instances. Remove it if the launcher doesn't start.
2.  *UU_Accepted.setting* is generated when the user accepts to install the game.
3.  *UU_Installed.setting* is generated when the redistributables have been installed.


How to use
--------

The updater use a fresh installation of your game - you need to have a full deployed package of your game. The process will generate a file tree that you can use with this tool. 

1.  Rename the *Res_Sample/* into *Res/* and edit the images to match your project's look.
2.  Edit *Res/project.h* to set your FTP data, game name, etc.
3.  Use QTCreator to build the updater. You **should** use a static build of Qt so that the updater is a single file.
4.  Create a ReleaseNotes.xml on your server to update the description on the updater main window. See below.
5.  Create a GameManifest.xml on your server to tell the updater about the files you want. See below.
6.  Your updater is ready, feel free to rename it and distribute it.

**Beware : this is GPL software. You have to provide the sources, or link them on GitHub if you didn't change anything.**


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
