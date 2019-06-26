#include "uninstaller.h"

uninstallerDialog::uninstallerDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Uninstall"));
}

void uninstallerDialog::updateProgress()
{
	QPixmap pixmap = failed ? QPixmap(":/png/png/canceled.png") : QPixmap(":/png/png/complete.png");

	if(step == 1)
	{
		progressBar_stop_service->setValue(failed ? 50 : 100);

		label_stop_service->setPixmap(pixmap);
	}
	else if(step == 2)
	{
		progressBar_remove_bin->setValue(failed ? 50 : 100);

		label_remove_bin->setPixmap(pixmap);
	}
	else if(step == 3)
	{
		progressBar_remove_cfg->setValue(failed ? 50 : 100);

		label_remove_cfg->setPixmap(pixmap);
	}
	else if(step == 4)
	{
		progressBar_restore_hosts->setValue(failed ? 50 : 100);

		label_restore_hosts->setPixmap(pixmap);
	}
	else if(step == 5)
	{
		progressBar_restore_rclocal->setValue(failed ? 50 : 100);

		label_restore_rclocal->setPixmap(pixmap);
	}

	step++;
}

void uninstallerDialog::ssh_connected()
{
	plainTextEdit->appendPlainText(tr("SSH: connected\n"));

	if(reinterpret_cast<MainWindow*>(parent())->cfg.ssh_auth == "PKey")
	{
		QFile file_key(reinterpret_cast<MainWindow*>(parent())->cfg.ssh_pkey);

		if(file_key.open(QIODevice::ReadOnly))
		{
			QByteArray key = file_key.readAll();

			file_key.close();

			ssh->loginKey(reinterpret_cast<MainWindow*>(parent())->cfg.ssh_user, key, reinterpret_cast<MainWindow*>(parent())->cfg.ssh_pkpp);
		}
	}
	else
	{
		ssh->login(reinterpret_cast<MainWindow*>(parent())->cfg.ssh_user, reinterpret_cast<MainWindow*>(parent())->cfg.ssh_pass);
	}
}

void uninstallerDialog::ssh_loginSuccessful()
{
	plainTextEdit->appendPlainText(tr("SSH: logged in\n"));

	ssh->executeCommand(VALETUDO_CMD_STOP);
}

void uninstallerDialog::ssh_commandExecuted(QString command, QString response)
{
	plainTextEdit->appendPlainText(tr("SSH: %1 -> %2\n").arg(command).arg(response.isEmpty() ? QString("OK") : response));

	QThread::msleep(250);

	if(command == VALETUDO_CMD_STOP)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_RMBIN);
	}
	else if(command == VALETUDO_CMD_RMBIN)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_RMCFG);
	}
	else if(command == VALETUDO_CMD_RMCFG)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_HOSTS);
	}
	else if(command == VALETUDO_CMD_HOSTS)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_RCLOCAL);
	}
	else if(command == VALETUDO_CMD_RCLOCAL)
	{
		updateProgress();

		ssh->disconnectFromHost();
	}
}

void uninstallerDialog::ssh_disconnected()
{
	plainTextEdit->appendPlainText(tr("SSH: disconnected\n"));

	delete ssh;
	ssh = nullptr;

	failed ? QMessageBox::warning(this, APPNAME, tr("Valetudo uninstallation failed!")) : QMessageBox::information(this, APPNAME, tr("Valetudo uninstallation finished.\n\nRead log for details."));

	buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
}

void uninstallerDialog::ssh_error(QSshSocket::SshError error)
{
	plainTextEdit->appendPlainText(tr("SSH: %1\n").arg(reinterpret_cast<MainWindow*>(parent())->ssh_error_strings.at(error)));

	failed = true;

	updateProgress();

	ssh->disconnectFromHost();
}

void uninstallerDialog::reject()
{
	if(ssh && ssh->isConnected())
	{
		QMessageBox::warning(this, APPNAME, tr("Can not abort current operation, please wait..."));

		return;
	}

	accept();
}

void uninstallerDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
	{
		button->setDisabled(true);

		failed = false;
		step = 1;

		label_stop_service->setPixmap(QPixmap(":/png/png/question.png"));
		label_remove_bin->setPixmap(QPixmap(":/png/png/question.png"));
		label_remove_cfg->setPixmap(QPixmap(":/png/png/question.png"));
		label_restore_hosts->setPixmap(QPixmap(":/png/png/question.png"));
		label_restore_rclocal->setPixmap(QPixmap(":/png/png/question.png"));

		progressBar_stop_service->setValue(0);
		progressBar_remove_bin->setValue(0);
		progressBar_remove_cfg->setValue(0);
		progressBar_restore_hosts->setValue(0);
		progressBar_restore_rclocal->setValue(0);

		plainTextEdit->clear();

		ssh = new QSshSocket(this);

		connect(ssh, SIGNAL(connected()), this, SLOT(ssh_connected()));
		connect(ssh, SIGNAL(disconnected()), this, SLOT(ssh_disconnected()));
		connect(ssh, SIGNAL(error(QSshSocket::SshError)), this, SLOT(ssh_error(QSshSocket::SshError)));
		connect(ssh, SIGNAL(loginSuccessful()), this, SLOT(ssh_loginSuccessful()));
		connect(ssh, SIGNAL(commandExecuted(QString, QString)), this, SLOT(ssh_commandExecuted(QString, QString)));

		ssh->connectToHost(reinterpret_cast<MainWindow*>(parent())->cfg.ip);
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
