#include "setup.h"

setupDialog::setupDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	connect(&timerVolume, SIGNAL(timeout()), this, SLOT(timer_setVolume()));

	lineEdit_ip->setValidator(new QRegExpValidator(QRegExp("(([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.){3}([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])")));
	lineEdit_token->setValidator(new QRegExpValidator(QRegExp("[0-9a-fA-F]{32}")));

	spinBox_id->setValue(((MainWindow*)parent)->cfg.msgid);
	lineEdit_ip->setText(((MainWindow*)parent)->cfg.ip);
	lineEdit_token->setText(QString(((MainWindow*)parent)->cfg.token).toUpper());

	horizontalSlider_volume->blockSignals(true);
	horizontalSlider_volume->setValue(((MainWindow*)parent)->robo.volume);
	horizontalSlider_volume->blockSignals(false);
	label_volume->setText(QString::number(((MainWindow*)parent)->robo.volume));
	groupBox_dnd->setChecked(((MainWindow*)parent)->robo.dnd.enabled ? true : false);
	timeEdit_dnd1->setTime(QTime(((MainWindow*)parent)->robo.dnd.start_hour, ((MainWindow*)parent)->robo.dnd.start_minute));
	timeEdit_dnd0->setTime(QTime(((MainWindow*)parent)->robo.dnd.end_hour, ((MainWindow*)parent)->robo.dnd.end_minute));

	groupBox_carpet->setChecked(((MainWindow*)parent)->robo.carpetmode.enable);
	spinBox_carpet_integral->setValue(((MainWindow*)parent)->robo.carpetmode.current_integral);
	spinBox_carpet_high->setValue(((MainWindow*)parent)->robo.carpetmode.current_high);
	spinBox_carpet_low->setValue(((MainWindow*)parent)->robo.carpetmode.current_low);
	spinBox_carpet_stalltime->setValue(((MainWindow*)parent)->robo.carpetmode.stall_time);

	lineEdit_ssh_username->setText(((MainWindow*)parent)->cfg.ssh_user);
	lineEdit_ssh_password->setText(((MainWindow*)parent)->cfg.ssh_pass);
	lineEdit_ssh_keyfile->setText(((MainWindow*)parent)->cfg.ssh_pkey);

	if(((MainWindow*)parent)->cfg.ssh_auth == "PKey")
	{
		groupBox_ssh_password->setChecked(false);
		groupBox_ssh_keyfile->setChecked(true);
	}
	else
	{
		groupBox_ssh_password->setChecked(true);
		groupBox_ssh_keyfile->setChecked(false);
	}

	if(((MainWindow*)parent)->cfg.token == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
	{
		lineEdit_token->setFocus();
	}

	if(((MainWindow*)parent)->provisioning)
	{
		tabWidget->setCurrentWidget(tabWidget->findChild<QWidget*>("tab_wifi"));
	}

	if(parent->isHidden())
	{
		move(qApp->desktop()->screen()->rect().center() - rect().center());
	}
}

// tab communication

void setupDialog::on_toolButton_ssh_clicked()
{
	if(((MainWindow*)parent())->cfg.ssh_auth == "PKey" && ((MainWindow*)parent())->cfg.ssh_pkey.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please choose ssh private keyfile first!"));

		tabWidget->setCurrentWidget(tabWidget->findChild<QWidget*>("tab_ssh"));

		emit on_toolButton_ssh_keyfile_clicked();

		return;
	}
	else if(((MainWindow*)parent())->cfg.ssh_auth == "Pass" && ((MainWindow*)parent())->cfg.ssh_pass.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please enter ssh password first!"));

		tabWidget->setCurrentWidget(tabWidget->findChild<QWidget*>("tab_ssh"));
		lineEdit_ssh_password->setFocus();

		return;
	}
	else if(((MainWindow*)parent())->cfg.ssh_user.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please enter ssh username first!"));

		tabWidget->setCurrentWidget(tabWidget->findChild<QWidget*>("tab_ssh"));
		lineEdit_ssh_username->setFocus();

		return;
	}

	ssh = new QSshSocket(this);

	connect(ssh, SIGNAL(connected()), this, SLOT(ssh_connected()));
	connect(ssh, SIGNAL(disconnected()), this, SLOT(ssh_disconnected()));
	connect(ssh, SIGNAL(error(QSshSocket::SshError)), this, SLOT(ssh_error(QSshSocket::SshError)));
	connect(ssh, SIGNAL(loginSuccessful()), this, SLOT(ssh_loginSuccessful()));
	connect(ssh, SIGNAL(commandExecuted(QString, QString)), this, SLOT(ssh_commandExecuted(QString, QString)));

	ssh->connectToHost(((MainWindow*)parent())->cfg.ip);
}

void setupDialog::on_toolButton_convert_clicked()
{
	converterDialog((QWidget*)parent()).exec();
}

// tab sound

void setupDialog::on_horizontalSlider_volume_valueChanged(int value)
{
	label_volume->setText(QString::number(value));

	timerVolume.setSingleShot(true);
	timerVolume.start(1000);
}

void setupDialog::timer_setVolume()
{
	int volume = horizontalSlider_volume->value();

	if(((MainWindow*)parent())->sendUDP(QString(MIIO_CHANGE_SOUND_VOLUME).arg(volume).arg("%1")))
	{
		((MainWindow*)parent())->robo.volume = volume;

		((MainWindow*)parent())->sendUDP(MIIO_TEST_SOUND_VOLUME);
	}
}

void setupDialog::on_toolButton_sound_install_clicked()
{
	QFile file(QFileDialog::getOpenFileName(this, tr("Select voice package to install"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "*.pkg", 0, QFileDialog::DontUseNativeDialog));

	if(!file.fileName().isEmpty())
	{
		if(file.open(QIODevice::ReadOnly))
		{
			if(file.size())
			{
				uploadDialog((QWidget*)parent(), &file).exec();
			}
			else
			{
				QMessageBox::warning(this, APPNAME, tr("Selected voice package is empty!"));

				emit on_toolButton_sound_install_clicked();
			}
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Could not open voice package!\n\n%1").arg(file.errorString()));

			emit on_toolButton_sound_install_clicked();
		}
	}
}

void setupDialog::on_toolButton_sound_pack_clicked()
{
	QFile dir(QFileDialog::getExistingDirectory(this, tr("Select folder with voice files"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly));

	if(!dir.fileName().isEmpty())
	{
		if(QDir(dir.fileName(), "*.wav", QDir::Name, QDir::Files).entryInfoList().count())
		{
			packagerDialog((QWidget*)parent(), &dir).exec();
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Selected folder contains no voices files!"));

			emit on_toolButton_sound_pack_clicked();
		}
	}
}

void setupDialog::on_toolButton_sound_unpack_clicked()
{
	QFile file_pkg(QFileDialog::getOpenFileName(this, tr("Select voice package to unpack"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "*.pkg", 0, QFileDialog::DontUseNativeDialog));

	if(!file_pkg.fileName().isEmpty())
	{
		if(file_pkg.open(QIODevice::ReadOnly))
		{
			if(file_pkg.size())
			{
				QFile file_dir(QFileDialog::getExistingDirectory(this, tr("Select folder to unpack voice package"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly));

				if(!file_dir.fileName().isEmpty())
				{
					unpackagerDialog((QWidget*)parent(), &file_pkg, &file_dir).exec();
				}
			}
			else
			{
				QMessageBox::warning(this, APPNAME, tr("Selected voice package is empty!"));

				emit on_toolButton_sound_unpack_clicked();
			}
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Could not open voice package!\n\n%1").arg(file_pkg.errorString()));

			emit on_toolButton_sound_unpack_clicked();
		}
	}
}

// tab misc

void setupDialog::on_toolButton_carpet_reset_clicked()
{
	spinBox_carpet_integral->setValue(450);
	spinBox_carpet_high->setValue(500);
	spinBox_carpet_low->setValue(400);
	spinBox_carpet_stalltime->setValue(10);
}

// tab ssh

void setupDialog::on_groupBox_ssh_password_clicked(bool checked)
{
	groupBox_ssh_keyfile->setChecked(!checked);

	if(lineEdit_ssh_password->text().isEmpty())
	{
		lineEdit_ssh_password->setFocus();
	}

	if(!checked && lineEdit_ssh_keyfile->text().isEmpty())
	{
		emit on_toolButton_ssh_keyfile_clicked();
	}
}

void setupDialog::on_groupBox_ssh_keyfile_clicked(bool checked)
{
	groupBox_ssh_password->setChecked(!checked);

	if(!checked && lineEdit_ssh_password->text().isEmpty())
	{
		lineEdit_ssh_password->setFocus();
	}

	if(checked && lineEdit_ssh_keyfile->text().isEmpty())
	{
		emit on_toolButton_ssh_keyfile_clicked();
	}
}

void setupDialog::on_toolButton_ssh_keyfile_clicked()
{
	QFile file_key(QFileDialog::getOpenFileName(this, tr("Select private keyfile for ssh login"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ssh", "*", 0, QFileDialog::DontUseNativeDialog));

	if(!file_key.fileName().isEmpty())
	{
		if(file_key.open(QIODevice::ReadOnly))
		{
			QByteArray key = file_key.readAll();

			file_key.close();

			if(key.startsWith("-----BEGIN RSA PRIVATE KEY-----\n") && key.endsWith("\n-----END RSA PRIVATE KEY-----\n"))
			{
				lineEdit_ssh_keyfile->setText(file_key.fileName());

				return;
			}
			else
			{
				QMessageBox::warning(this, APPNAME, tr("Does not look like valid private key!"));
			}
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Could not open keyfile!\n\n%1").arg(file_key.errorString()));
		}

		emit on_toolButton_ssh_keyfile_clicked();
	}
}

void setupDialog::ssh_connected()
{
	ssh->login(((MainWindow*)parent())->cfg.ssh_user, ((MainWindow*)parent())->cfg.ssh_auth == "PKey" ? "" : ((MainWindow*)parent())->cfg.ssh_pass);

	if(((MainWindow*)parent())->cfg.ssh_auth == "PKey")
	{
		QFile file_key(((MainWindow*)parent())->cfg.ssh_pkey);

		if(file_key.open(QIODevice::ReadOnly))
		{
			QByteArray key = file_key.readAll();

			file_key.close();

			ssh->setKey(key);
		}
	}
}

void setupDialog::ssh_loginSuccessful()
{
	ssh->executeCommand("cat /mnt/data/miio/device.token");
}

void setupDialog::ssh_commandExecuted(__attribute__((unused)) QString command, QString response)
{
	QByteArray token = response.toUtf8().toHex();

	ssh->disconnectFromHost();

	if(token.length() == 32)
	{
		lineEdit_token->setText(token.toUpper());

		QMessageBox::information(this, APPNAME, tr("AES-Token successfully extracted:\n\n%1").arg(QString(token)));
	}
	else
	{
		QMessageBox::warning(this, APPNAME, tr("AES-Token extraction failed:\n\n%1").arg(response.isEmpty() ? tr("got empty response") : response));
	}
}

void setupDialog::ssh_disconnected()
{
	delete ssh;
}

void setupDialog::ssh_error(QSshSocket::SshError error)
{
	ssh->disconnectFromHost();

	QMessageBox::warning(this, APPNAME, tr("SSH connection error!\n\n%1").arg(((MainWindow*)parent())->ssh_error_strings.at(error)));
}

void setupDialog::reject()
{
	if(((MainWindow*)parent())->cfg.token == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
	{
		QMessageBox::warning(this, APPNAME, tr("Program will not work without valid device token!"));
	}

	accept();
}

void setupDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Save)
	{
		if(tabWidget->currentWidget()->objectName() == "tab_communication")
		{
			QString token = lineEdit_token->text();
			QString ip = lineEdit_ip->text();

			if(ip == "255.255.255.255" || ip.isEmpty())
			{
				QMessageBox::warning(this, APPNAME, tr("Please enter your device ip-address!"));
			}
			else if(token == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" || token.length() < 32)
			{
				QMessageBox::warning(this, APPNAME, tr("Please enter your device token!"));
			}
			else
			{
				((MainWindow*)parent())->cfg.ip = ip;
				((MainWindow*)parent())->cfg.token = token;
				((MainWindow*)parent())->cfg.msgid = spinBox_id->value();
			}
		}
		else if(tabWidget->currentWidget()->objectName() == "tab_sound")
		{
			if(groupBox_dnd->isChecked())
			{
				int sh = timeEdit_dnd1->time().hour();
				int sm = timeEdit_dnd1->time().minute();
				int eh = timeEdit_dnd0->time().hour();
				int em = timeEdit_dnd0->time().minute();

				if(((MainWindow*)parent())->sendUDP(QString(MIIO_SET_DND_TIMER).arg(QString("%1,%2,%3,%4").arg(sh).arg(sm).arg(eh).arg(em)).arg("%1")))
				{
					((MainWindow*)parent())->robo.dnd.start_hour = sh;
					((MainWindow*)parent())->robo.dnd.start_minute = sm;
					((MainWindow*)parent())->robo.dnd.end_hour = eh;
					((MainWindow*)parent())->robo.dnd.end_minute = em;
					((MainWindow*)parent())->robo.dnd.enabled = 1;
				}
			}
			else
			{
				if(((MainWindow*)parent())->sendUDP(MIIO_CLOSE_DND_TIMER))
				{
					((MainWindow*)parent())->robo.dnd.enabled = 0;
				}
			}
		}
		else if(tabWidget->currentWidget()->objectName() == "tab_misc")
		{
			if(((MainWindow*)parent())->sendUDP(QString(MIIO_SET_CARPET_MODE).arg(groupBox_carpet->isChecked() ? 1 : 0).arg(spinBox_carpet_integral->value()).arg(spinBox_carpet_high->value()).arg(spinBox_carpet_low->value()).arg(spinBox_carpet_stalltime->value()).arg("%1")))
			{
				((MainWindow*)parent())->robo.carpetmode.enable = groupBox_carpet->isChecked();
				((MainWindow*)parent())->robo.carpetmode.current_integral = spinBox_carpet_integral->value();
				((MainWindow*)parent())->robo.carpetmode.current_high = spinBox_carpet_high->value();
				((MainWindow*)parent())->robo.carpetmode.current_low = spinBox_carpet_low->value();
				((MainWindow*)parent())->robo.carpetmode.stall_time = spinBox_carpet_stalltime->value();
			}
		}
		else if(tabWidget->currentWidget()->objectName() == "tab_ssh")
		{
			QString usr = lineEdit_ssh_username->text();
			QString pwd = lineEdit_ssh_password->text();
			QString key = lineEdit_ssh_keyfile->text();

			if(usr.isEmpty())
			{
				QMessageBox::warning(this, APPNAME, tr("Please enter your ssh username!"));
				return;
			}

			if(groupBox_ssh_password->isChecked() && pwd.isEmpty())
			{
				QMessageBox::warning(this, APPNAME, tr("Please enter your ssh password!"));
				return;
			}

			if(groupBox_ssh_keyfile->isChecked() && key.isEmpty())
			{
				QMessageBox::warning(this, APPNAME, tr("Please select your ssh private keyfile!"));
				return;
			}

			((MainWindow*)parent())->cfg.ssh_user = lineEdit_ssh_username->text();
			((MainWindow*)parent())->cfg.ssh_pass = lineEdit_ssh_password->text();
			((MainWindow*)parent())->cfg.ssh_pkey = lineEdit_ssh_keyfile->text();
			((MainWindow*)parent())->cfg.ssh_auth = groupBox_ssh_keyfile->isChecked() ? "PKey" : "Pass";
		}
		else if(tabWidget->currentWidget()->objectName() == "tab_wifi")
		{
			QString ssid = lineEdit_ssid->text();
			QString key = lineEdit_key->text();

			if(ssid == "<SSID>" || ssid.isEmpty())
			{
				QMessageBox::warning(this, APPNAME, tr("Please enter your wifi ssid!"));
			}
			else if(key == "<KEY>" || key.isEmpty())
			{
				QMessageBox::warning(this, APPNAME, tr("Please enter your wifi key!"));
			}
			else
			{
				QClipboard *clipboard = QGuiApplication::clipboard();

				((MainWindow*)parent())->sendUDP(QString(MIIO_CONFIG_ROUTER).arg(ssid).arg(key).arg("%1"));

				((MainWindow*)parent())->provisioning = false;

				QMessageBox::information(this, APPNAME, tr("Now extract and copy your new device token to clipboard.\nFor rooted devices you can use the integrated token extractor.\n\nWait until device has connected to your wifi and then press OK..."));

				((MainWindow*)parent())->cfg.ip = "255.255.255.255";
				((MainWindow*)parent())->cfg.token = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

				((MainWindow*)parent())->sendUDP(NULL);

				lineEdit_ip->setText(((MainWindow*)parent())->cfg.ip);
				lineEdit_token->setText(QString(((MainWindow*)parent())->cfg.token).toUpper());
				lineEdit_token->setFocus();

				if(clipboard->text().length() == 32)
				{
					lineEdit_token->setText(clipboard->text(QClipboard::Clipboard));
				}

				tabWidget->setCurrentWidget(tabWidget->findChild<QWidget*>("tab_communication"));
			}
		}
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
