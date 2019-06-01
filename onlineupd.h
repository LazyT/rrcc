#ifndef ONLINEUPDDLG_H
#define ONLINEUPDDLG_H

#include "ui_onlineupd.h"
#include "mainwindow.h"

#ifdef Q_OS_LINUX
	#define BIN ".AppImage"
	#define IMG ":/png/png/upd_lin.png"
#elif defined Q_OS_WIN
	#define BIN ".exe"
	#define IMG ":/png/png/upd_win.png"
#elif defined Q_OS_OSX
	#define BIN ".dmg"
	#define IMG ":/png/png/upd_mac.png"
#endif

#define RELEASE "https://api.github.com/repos/lazyt/rrcc/releases/latest"

class onlineUpdDialog : public QDialog, private Ui::Dialog_OnlineUpdate
{
	Q_OBJECT

public:

	onlineUpdDialog(QWidget*, bool);

private:

	bool silent;

	QNetworkAccessManager *netmgr;
	QNetworkRequest request;
	QNetworkReply *reply;
	QByteArray download;
	QString url;
	int size;

private slots:

	void checkUpdate();
	void downloadProgress(qint64, qint64);
	void finished(QNetworkReply*);
	void reject();
	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // ONLINEUPDDLG_H
