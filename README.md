# **RoboRock Control Center**
[![](https://img.shields.io/github/license/lazyt/rrcc.svg?color=blue)](https://github.com/LazyT/rrcc/blob/master/LICENSE)
[![](https://img.shields.io/badge/paypal-buy%20me%20a%20beer-red.svg)](https://paypal.me/LazyT)
[![](https://img.shields.io/github/downloads/lazyt/rrcc/total.svg?color=orange)](https://github.com/LazyT/rrcc/releases)
[![](https://img.shields.io/github/last-commit/lazyt/rrcc/master.svg?color=yellow)](https://github.com/LazyT/rrcc/commits/master)
[![](https://img.shields.io/github/release-date/lazyt/rrcc.svg?color=brightgreen)](https://github.com/LazyT/rrcc/releases/latest)
[![](https://img.shields.io/github/release/lazyt/rrcc.svg?color=brightgreen)](https://github.com/LazyT/rrcc/releases/latest)

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
* search, download and update firmware
* control dnd and carpet modes
* set wifi ssid and key
* convert 96->32 byte token
* multi language (English and German)
* cross platform (same look & feel on Windows, Linux, MacOS)

Only on rooted devices:

* extract token via ssh
* install/update/uninstall [Valetudo](https://github.com/hypfer/valetudo)
* map functions via [Valetudo](https://github.com/hypfer/valetudo) (goto, zone cleaning, virtual walls and nogo zones, flip, rotate, zoom)
* show installed firmware and valetudo version

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

 - Qt 5.11 or greater required
 - libarchive-dev and libssh-dev required

## Credits

RRCC is based on

* [Qt](http://www.qt.io)
* [QAES](https://github.com/bricke/Qt-AES)
* [QArchive](https://github.com/antony-jr/QArchive)
* [QSshSocket](https://github.com/mikemvk/QSshSocket)
* [ccrypt](http://ccrypt.sourceforge.net)
* [libarchive](https://www.libarchive.org)
* [libssh](https://www.libssh.org)
* [openssl](https://www.openssl.org)
* [zlib](https://zlib.net)

Thanks for this great software!
