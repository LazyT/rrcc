# **RoboRock Control Center**

## Description

Xiaomi vacuum cleaners are really nice, but need a cloud connection to use all features. You don't like your data in chinese hands? Then try RRCC for free and use it on Windows, Linux and MacOS!

The current version supports the following features:

* control vacuum (start, pause, stop, spot cleaning, return dock, find)
* set fanspeed
* reset consumables
* add, change and delete timers
* show cleaning history
* set sound volume
* decrypt, encrypt and install voice packages
* control dnd and carpet modes
* set wifi ssid and key
* convert 96->32 byte token
* multi language (English and German)
* cross platform (same look & feel on Windows, Linux, MacOS)

Only on rooted devices:

* extract token via ssh
* map functions via [Valetudo](https://github.com/hypfer/valetudo) (goto spot, zone cleaning, show coordinates, flip hor/ver, rotate 90/180/270°, zoom)

## Screenshots

![main window](https://raw.github.com/LazyT/rrcc/gh-pages/screenshots/mainwindow.png)

## Download

Download the latest version for your operating system. All 3 files (exe, dmg, AppImage) contain the binary, translations and required libraries to run RRCC on the target platform without installing anything.

* [Windows (exe)](https://github.com/LazyT/rrcc/releases)

This is an [7zip](https://www.7-zip.org) self extracting archive. It will be automatically extracted to "%temp%\7zxxxxxxxx" and after that the "rrcc.exe" is started. You can copy this directory or extract the file with 7z if you want the content.

* [MacOS (dmg)](https://github.com/LazyT/rrcc/releases)

This is an Apple disc image. You can mount and run or copy the application.

* [Linux (AppImage)](https://github.com/LazyT/rrcc/releases)

This is an [AppImage](https://appimage.org) package. Don't forget to "chmod +x *.AppImage" after download and then run it. You can use the parameter "--appimage-extract" if you want the content.

## Build from Source

If the release binaries doesn't work for you build it yourself:

1) checkout the source code

		git clone https://github.com/LazyT/rrcc

2) change into the new rrcc directory and generate the Makefile

		cd rrcc && qmake

3) compile the source code

		make

GUI fans just install the [Qt-Environment](http://www.qt.io/download-open-source), open the "rrcc.pro" project file and click on the build button.

***Hints for compiling:***

 - Qt5.11 or greater required
 - libarchive and libssh required

## Credits

RRCC is based on

* [Qt](http://www.qt.io)
* [QSshSocket](https://github.com/mikemvk/QSshSocket)
* [QAES](https://github.com/bricke/Qt-AES)
* [QArchive](https://github.com/antony-jr/QArchive)
* [ccrypt](http://ccrypt.sourceforge.net)
* [libarchive](https://www.libarchive.org)
* [zlib](https://zlib.net)
* [libssh](https://www.libssh.org)
* [openssl](https://www.openssl.org)

Thanks for this great software!
