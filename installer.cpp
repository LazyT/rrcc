#include "installer.h"

installerDialog::installerDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Install"));

	QTimer::singleShot(1, this, SLOT(getReleases()));
}

void installerDialog::getReleases()
{
	step = 0;

	if(!Download(VALETUDO_REL))
	{
		QJsonDocument doc = QJsonDocument::fromJson(download);
		QJsonArray arr = doc.array();
		QJsonArray sub;
		QString name, date, file;

		comboBox->removeItem(0);

		for(int i = 0; i < arr.count(); i++)
		{
			if(arr[i].toObject().contains("name"))
			{
				name = arr[i].toObject().value("name").toString();
			}

			if(arr[i].toObject().contains("tag_name") && name.isEmpty())
			{
				name = arr[i].toObject().value("tag_name").toString();
			}

			if(arr[i].toObject().contains("published_at"))
			{
				date = arr[i].toObject().value("published_at").toString();
			}

			sub = arr[i].toObject().value("assets").toArray();

			if(sub[0].toObject().contains("browser_download_url"))
			{
				file = sub[0].toObject().value("browser_download_url").toString();
			}

			comboBox->addItem(QString("%1 [ %2 %3 ]").arg(name).arg(date.left(10)).arg(date.mid(11, 8)), file);
		}

		if(!comboBox->count())
		{
			hide();

			QMessageBox::warning(this, APPNAME, tr("No Valetudo releases found on Github!"));

			close();
		}
	}
	else
	{
		close();
	}
}

bool installerDialog::Download(QString url)
{
	QNetworkAccessManager *netmgr = new QNetworkAccessManager(this);
	QNetworkRequest request = QNetworkRequest(QUrl(url));

	if(step)
	{
		plainTextEdit->appendPlainText(tr("GET: %1 started\n").arg(url));
	}

	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

	finish = false;

	reply = netmgr->get(request);

	reply->ignoreSslErrors();

	connect(netmgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));

	while(!finish)
	{
		QCoreApplication::processEvents();
	}

	if(!failed && step)
	{
		QFile file;

		if(url.endsWith(".conf"))
		{
			file.setFileName(VALETUDO_CFG_SRC);
		}
		else
		{
			file.setFileName(VALETUDO_BIN_SRC);
		}

		if(file.open(QIODevice::WriteOnly))
		{
			file.write(download);
			file.close();
		}
	}

	return failed;
}

void installerDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if(step == 1)
	{
		progressBar_download_bin->setFormat(QString("%1 / %2").arg(bytesReceived).arg(bytesTotal));
		progressBar_download_bin->setValue((bytesReceived * 100) / bytesTotal);
	}
	else if(step == 2)
	{
		progressBar_download_cfg->setFormat(QString("%1 / %2").arg(bytesReceived).arg(bytesTotal));
		progressBar_download_cfg->setValue((bytesReceived * 100) / bytesTotal);
	}
}

void installerDialog::finished(QNetworkReply *reply)
{
	finish = true;

	if(reply->error())
	{
		failed = true;

		if(reply->url().toString() != VALETUDO_REL)
		{
			updateProgress();

			plainTextEdit->appendPlainText(tr("GET: failed -> %1\n").arg(reply->errorString()));

			buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
		}
		else
		{
			hide();

			QMessageBox::warning(this, APPNAME, tr("Download Valetudo release list from Github failed!\n\n%1").arg(reply->errorString()));
		}

		return;
	}

	download = reply->readAll();

	failed = false;

	if(step)
	{
		updateProgress();

		plainTextEdit->appendPlainText(tr("GET: finished\n"));
	}
}

void installerDialog::updateProgress()
{
	QPixmap pixmap = failed ? QPixmap(":/png/png/canceled.png") : QPixmap(":/png/png/complete.png");

	if(step == 1)
	{
		progressBar_download_bin->setValue(failed ? 50 : 100);

		label_download_bin->setPixmap(pixmap);
	}
	else if(step == 2)
	{
		progressBar_download_cfg->setValue(failed ? 50 : 100);

		label_download_cfg->setPixmap(pixmap);
	}
	else if(step == 3)
	{
		progressBar_stop_service->setValue(failed ? 50 : 100);

		label_stop_service->setPixmap(pixmap);
	}
	else if(step == 4)
	{
		progressBar_copy_bin->setValue(failed ? 50 : 100);

		label_copy_bin->setPixmap(pixmap);
	}
	else if(step == 5)
	{
		progressBar_copy_cfg->setValue(failed ? 50 : 100);

		label_copy_cfg->setPixmap(pixmap);
	}
	else if(step == 6)
	{
		progressBar_chmod_bin->setValue(failed ? 50 : 100);

		label_chmod_bin->setPixmap(pixmap);
	}
	else if(step == 7)
	{
		progressBar_start_service->setValue(failed ? 50 : 100);

		label_start_service->setPixmap(pixmap);
	}

	step++;
}

void installerDialog::ssh_connected()
{
	plainTextEdit->appendPlainText(tr("SSH: connected\n"));

	if(((MainWindow*)parent())->cfg.ssh_auth == "PKey")
	{
		QFile file_key(((MainWindow*)parent())->cfg.ssh_pkey);

		if(file_key.open(QIODevice::ReadOnly))
		{
			QByteArray key = file_key.readAll();

			file_key.close();

			ssh->loginKey(((MainWindow*)parent())->cfg.ssh_user, key, ((MainWindow*)parent())->cfg.ssh_pkpp);
		}
	}
	else
	{
		ssh->login(((MainWindow*)parent())->cfg.ssh_user, ((MainWindow*)parent())->cfg.ssh_pass);
	}
}

void installerDialog::ssh_loginSuccessful()
{
	plainTextEdit->appendPlainText(tr("SSH: logged in\n"));

	ssh->executeCommand(VALETUDO_CMD_STOP);
}

void installerDialog::ssh_pushSuccessful(__attribute__((unused)) QString localFile, QString remoteFile)
{
	plainTextEdit->appendPlainText(tr("SCP: finished\n"));

	if(remoteFile == VALETUDO_BIN_DST)
	{
		updateProgress();

		plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_CFG_SRC).arg(VALETUDO_CFG_DST));

		ssh->pushFile(VALETUDO_CFG_SRC, VALETUDO_CFG_DST);
	}
	else if(remoteFile == VALETUDO_CFG_DST)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_CHMOD);
	}
}

void installerDialog::ssh_commandExecuted(QString command, QString response)
{
	plainTextEdit->appendPlainText(tr("SSH: %1 -> %2\n").arg(command).arg(response.isEmpty() ? QString("OK") : response));

	if(command == VALETUDO_CMD_STOP)
	{
		updateProgress();

		plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_BIN_SRC).arg(VALETUDO_BIN_DST));

		ssh->pushFile(VALETUDO_BIN_SRC, VALETUDO_BIN_DST);
	}
	else if(command == VALETUDO_CMD_CHMOD)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_START);
	}
	else if(command == VALETUDO_CMD_START)
	{
		updateProgress();

		ssh->disconnectFromHost();
	}
}

void installerDialog::ssh_disconnected()
{
	plainTextEdit->appendPlainText(tr("SSH: disconnected\n"));

	delete ssh;
	ssh = NULL;

	failed ? QMessageBox::warning(this, APPNAME, tr("Valetudo installation failed!")) : QMessageBox::information(this, APPNAME, tr("Valetudo installation finished.\n\nRead log for details."));

	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void installerDialog::ssh_error(QSshSocket::SshError error)
{
	plainTextEdit->appendPlainText(tr("SSH: %1\n").arg(((MainWindow*)parent())->ssh_error_strings.at(error)));

	failed = true;

	updateProgress();

	ssh->disconnectFromHost();
}

void installerDialog::reject()
{
	if(reply && reply->isRunning())
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort download?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			reply->abort();
		}

		return;
	}
	else if(ssh && ssh->isConnected())
	{
		QMessageBox::warning(this, APPNAME, tr("Can not abort current operation, please wait..."));

		return;
	}

	accept();
}

void installerDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Apply)
	{
		button->setDisabled(true);

		failed = false;
		step = 1;

		label_download_bin->setPixmap(QPixmap(":/png/png/question.png"));
		label_download_cfg->setPixmap(QPixmap(":/png/png/question.png"));
		label_stop_service->setPixmap(QPixmap(":/png/png/question.png"));
		label_copy_bin->setPixmap(QPixmap(":/png/png/question.png"));
		label_copy_cfg->setPixmap(QPixmap(":/png/png/question.png"));
		label_chmod_bin->setPixmap(QPixmap(":/png/png/question.png"));
		label_start_service->setPixmap(QPixmap(":/png/png/question.png"));

		progressBar_download_bin->setValue(0);
		progressBar_download_cfg->setValue(0);
		progressBar_stop_service->setValue(0);
		progressBar_copy_bin->setValue(0);
		progressBar_copy_cfg->setValue(0);
		progressBar_chmod_bin->setValue(0);
		progressBar_start_service->setValue(0);

		progressBar_download_bin->setFormat("");
		progressBar_download_cfg->setFormat("");

		plainTextEdit->clear();

		if(Download(comboBox->itemData(comboBox->currentIndex()).toString()))
		{
			QMessageBox::warning(this, APPNAME, tr("Valetudo installation failed!"));

			return;
		}

		if(Download(VALETUDO_CFG))
		{
			QMessageBox::warning(this, APPNAME, tr("Valetudo installation failed!"));

			return;
		}

		ssh = new QSshSocket(this);

		connect(ssh, SIGNAL(connected()), this, SLOT(ssh_connected()));
		connect(ssh, SIGNAL(disconnected()), this, SLOT(ssh_disconnected()));
		connect(ssh, SIGNAL(error(QSshSocket::SshError)), this, SLOT(ssh_error(QSshSocket::SshError)));
		connect(ssh, SIGNAL(loginSuccessful()), this, SLOT(ssh_loginSuccessful()));
		connect(ssh, SIGNAL(pushSuccessful(QString, QString)), this, SLOT(ssh_pushSuccessful(QString, QString)));
		connect(ssh, SIGNAL(commandExecuted(QString, QString)), this, SLOT(ssh_commandExecuted(QString, QString)));

		ssh->connectToHost(((MainWindow*)parent())->cfg.ip);
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
