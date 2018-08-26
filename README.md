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

Only on rooted devices:

* extract token via ssh
* map functions (goto spot, zone cleaning, show coordinates, flip hor/ver, rotate 90/180/270°, zoom)

## Build from Source

If the installer binaries doesn't work for you build it yourself:

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
