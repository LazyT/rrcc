#ifndef SETUPDLG_H
#define SETUPDLG_H

#include "ui_setup.h"
#include "mainwindow.h"

class setupDialog : public QDialog, private Ui::Dialog_Setup
{
	Q_OBJECT

public:

	setupDialog(QWidget*);

private:

	QTimer timerVolume;
	QSshSocket *ssh;

	QStringList ssh_error_strings
	{
		"SocketError",
		"SessionCreationError",
		"ChannelCreationError",
		"ScpChannelCreationError",
		"ScpPullRequestError",
		"ScpPushRequestError",
		"ScpFileNotCreatedError",
		"ScpReadError",
		"ScpWriteError",
		"PasswordAuthenticationFailedError",
		"KeyAuthenticationFailedError"
	};

private slots:

	void on_horizontalSlider_volume_valueChanged(int);

	void on_toolButton_ssh_clicked();
	void on_toolButton_convert_clicked();
	void on_toolButton_sound_install_clicked();
	void on_toolButton_sound_pack_clicked();
	void on_toolButton_sound_unpack_clicked();
	void on_toolButton_carpet_reset_clicked();
	void on_toolButton_ssh_keyfile_clicked();

	void on_groupBox_ssh_password_clicked(bool checked);
	void on_groupBox_ssh_keyfile_clicked(bool checked);

	void on_buttonBox_clicked(QAbstractButton*);

	void timer_setVolume();

	void ssh_connected();
	void ssh_disconnected();
	void ssh_error(QSshSocket::SshError);
	void ssh_loginSuccessful();
	void ssh_commandExecuted(QString, QString);

	void reject();
};

#endif // SETUPDLG_H
