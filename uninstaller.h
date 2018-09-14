#ifndef UNINSTALLERDLG_H
#define UNINSTALLERDLG_H

#include "ui_uninstaller.h"
#include "mainwindow.h"

#define VALETUDO_CMD_STOP	"service valetudo stop"
#define VALETUDO_CMD_RMBIN	"rm /usr/local/bin/valetudo"
#define VALETUDO_CMD_RMCFG	"rm /etc/init/valetudo.conf"

class uninstallerDialog : public QDialog, private Ui::Dialog_Uninstaller
{
	Q_OBJECT

public:

	uninstallerDialog(QWidget*);

private:

	void updateProgress();

	QSshSocket *ssh = NULL;
	bool failed;
	int step;

private slots:

	void ssh_connected();
	void ssh_disconnected();
	void ssh_error(QSshSocket::SshError);
	void ssh_loginSuccessful();
	void ssh_commandExecuted(QString, QString);

	void reject();

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // UNINSTALLERDLG_H
