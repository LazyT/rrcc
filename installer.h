#ifndef INSTALLERDLG_H
#define INSTALLERDLG_H

#include "ui_installer.h"
#include "mainwindow.h"

#define VALETUDO_REL		"https://api.github.com/repos/hypfer/valetudo/releases"
#define VALETUDO_CFG		"https://github.com/Hypfer/Valetudo/raw/master/deployment/valetudo.conf"
#define VALETUDO_BIN_SRC	QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/valetudo"
#define VALETUDO_CFG_SRC	QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/valetudo.conf"
#define VALETUDO_BIN_DST	"/usr/local/bin/valetudo"
#define VALETUDO_CFG_DST	"/etc/init/valetudo.conf"
#define VALETUDO_CMD_STOP	"service valetudo stop"
#define VALETUDO_CMD_CHMOD	"chmod +x /usr/local/bin/valetudo"
#define VALETUDO_CMD_START	"service valetudo start"

class installerDialog : public QDialog, private Ui::Dialog_Installer
{
	Q_OBJECT

public:

	installerDialog(QWidget*);

private:

	bool Download(QString);
	void updateProgress();

	QNetworkReply *reply = NULL;
	QByteArray download;
	QSshSocket *ssh = NULL;
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
	void ssh_commandExecuted(QString, QString);
	void ssh_pushSuccessful(QString, QString);

	void reject();

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // INSTALLERDLG_H
