#ifndef INSTALLERDLG_H
#define INSTALLERDLG_H

#include "ui_installer.h"
#include "mainwindow.h"

#define VALETUDO_REL			"https://api.github.com/repos/hypfer/valetudo/releases"
#define VALETUDO_CFG			"https://github.com/Hypfer/Valetudo/raw/master/deployment/valetudo.conf"
#define VALETUDO_HOSTS			"https://github.com/Hypfer/Valetudo/raw/master/deployment/etc/hosts"
#define VALETUDO_RCLOCAL		"https://github.com/Hypfer/Valetudo/raw/master/deployment/etc/rc.local"

#define VALETUDO_BIN_SRC		QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/valetudo"
#define VALETUDO_CFG_SRC		QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/valetudo.conf"
#define VALETUDO_HOSTS_SRC		QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/hosts"
#define VALETUDO_RCLOCAL_SRC	QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/rc.local"

#define VALETUDO_BIN_DST		"/usr/local/bin/valetudo"
#define VALETUDO_CFG_DST		"/etc/init/valetudo.conf"
#define VALETUDO_HOSTS_DST		"/etc/hosts"
#define VALETUDO_RCLOCAL_DST	"/etc/rc.local"

#define VALETUDO_CMD_CHMOD		"chmod +x " VALETUDO_BIN_DST
#define VALETUDO_CMD_STOP		"service valetudo stop"
#define VALETUDO_CMD_START		"service valetudo start"

#define HOSTS_INIT				"### VALETUDO HOSTS INIT ###"
#define HOSTS_EXIT				"### VALETUDO HOSTS EXIT ###"
#define RCLOCAL_INIT			"### VALETUDO RC.LOCAL INIT ###"
#define RCLOCAL_EXIT			"### VALETUDO RC.LOCAL EXIT ###"

enum {HOSTS, RCLOCAL};

class installerDialog : public QDialog, private Ui::Dialog_Installer
{
	Q_OBJECT

public:

	installerDialog(QWidget*);

private:

	bool Download(QString);
	void mergeFiles(int);
	void updateProgress();

	QNetworkReply *reply = nullptr;
	QByteArray download;
	QSshSocket *ssh = nullptr;
	bool finish, failed;
	int step;

private slots:

	void getReleases();

	void downloadProgress(qint64, qint64);
	void finished(QNetworkReply*);

	void ssh_connected();
	void ssh_disconnected();
	void ssh_error(QSshSocket::SshError);
	void ssh_loginSuccessful();
	void ssh_pullSuccessful(QString, QString);
	void ssh_pushSuccessful(QString, QString);
	void ssh_commandExecuted(QString, QString);

	void reject();

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // INSTALLERDLG_H
