#include "installer.h"

installerDialog::installerDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Install"));
	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

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
		else
		{
			buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
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

		if(url.endsWith("valetudo"))
		{
			file.setFileName(VALETUDO_BIN_SRC);
		}
		else if(url.endsWith("valetudo.conf"))
		{
			file.setFileName(VALETUDO_CFG_SRC);
		}
		else if(url.endsWith("hosts"))
		{
			file.setFileName(VALETUDO_HOSTS_SRC + "_github");
		}
		else if(url.endsWith("rc.local"))
		{
			file.setFileName(VALETUDO_RCLOCAL_SRC + "_github");
		}

		if(file.open(QIODevice::WriteOnly))
		{
			file.write(download);
			file.close();
		}
	}

	return failed;
}

void installerDialog::mergeFiles(int type)
{
	QFile file(type == HOSTS ? VALETUDO_HOSTS_SRC : VALETUDO_RCLOCAL_SRC);
	QFile file_robot((type == HOSTS ? VALETUDO_HOSTS_SRC : VALETUDO_RCLOCAL_SRC)  + "_robot");
	QFile file_github((type == HOSTS ? VALETUDO_HOSTS_SRC : VALETUDO_RCLOCAL_SRC)  + "_github");
	QString hosts, hosts_robot, hosts_github;
	QString rclocal, rclocal_robot, rclocal_github;
	int init, exit;

	if(file.open(QIODevice::WriteOnly))
	{
		if(type == HOSTS)
		{
			if(file_robot.open(QIODevice::ReadOnly))
			{
				hosts_robot = file_robot.readAll();
				file_robot.close();
			}

			if(file_github.open(QIODevice::ReadOnly))
			{
				hosts_github = file_github.readAll();
				file_github.close();
			}

			if(hosts_robot.contains(HOSTS_INIT) && hosts_robot.contains(HOSTS_EXIT))
			{
				init = hosts_robot.indexOf(HOSTS_INIT);
				exit = hosts_robot.indexOf(HOSTS_EXIT);

				hosts = hosts_robot.remove(init, exit + QString(HOSTS_EXIT).length() - init);

				plainTextEdit->appendPlainText(tr("Old Valetudo host entries removed!\n"));
			}
			else
			{
				hosts = hosts_robot;
			}

			hosts.append(hosts_github);
		}
		else if(type == RCLOCAL)
		{
			if(file_robot.open(QIODevice::ReadOnly))
			{
				rclocal_robot = file_robot.readAll();
				file_robot.close();
			}

			if(file_github.open(QIODevice::ReadOnly))
			{
				rclocal_github = file_github.readAll();
				file_github.close();
			}

			if(rclocal_robot.contains(RCLOCAL_INIT) && rclocal_robot.contains(RCLOCAL_EXIT))
			{
				init = rclocal_robot.indexOf(RCLOCAL_INIT);
				exit = rclocal_robot.indexOf(RCLOCAL_EXIT);

				rclocal = rclocal_robot.remove(init, exit + QString(RCLOCAL_EXIT).length() - init);

				plainTextEdit->appendPlainText(tr("Old Valetudo rc.local entries removed!\n"));
			}
			else
			{
				rclocal = rclocal_robot;
			}

			rclocal.replace("exit", rclocal_github + "\nexit");
		}

		file.write((type == HOSTS ? hosts : rclocal).toUtf8());
		file.close();
	}
}

void installerDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if(bytesReceived && bytesTotal)
	{
		if(step == 1)
		{
			progressBar_download_bin->setFormat(QString("%1 / %2").arg(bytesReceived).arg(bytesTotal));
			progressBar_download_bin->setValue(static_cast<int>((bytesReceived * 100) / bytesTotal));
		}
		else if(step == 2)
		{
			progressBar_download_cfg->setFormat(QString("%1 / %2").arg(bytesReceived).arg(bytesTotal));
			progressBar_download_cfg->setValue(static_cast<int>((bytesReceived * 100) / bytesTotal));
		}
		else if(step == 3)
		{
			progressBar_download_hosts_github->setFormat(QString("%1 / %2").arg(bytesReceived).arg(bytesTotal));
			progressBar_download_hosts_github->setValue(static_cast<int>((bytesReceived * 100) / bytesTotal));
		}
		else if(step == 4)
		{
			progressBar_download_rclocal_github->setFormat(QString("%1 / %2").arg(bytesReceived).arg(bytesTotal));
			progressBar_download_rclocal_github->setValue(static_cast<int>((bytesReceived * 100) / bytesTotal));
		}
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
		progressBar_download_hosts_github->setValue(failed ? 50 : 100);

		label_download_hosts_github->setPixmap(pixmap);
	}
	else if(step == 4)
	{
		progressBar_download_rclocal_github->setValue(failed ? 50 : 100);

		label_download_rcocal_github->setPixmap(pixmap);
	}
	else if(step == 5)
	{
		progressBar_download_hosts_robot->setValue(failed ? 50 : 100);

		label_download_hosts_robot->setPixmap(pixmap);
	}
	else if(step == 6)
	{
		progressBar_download_rclocal_robot->setValue(failed ? 50 : 100);

		label_download_rclocal_robot->setPixmap(pixmap);
	}
	else if(step == 7)
	{
		progressBar_stop_service->setValue(failed ? 50 : 100);

		label_stop_service->setPixmap(pixmap);
	}
	else if(step == 8)
	{
		progressBar_copy_bin->setValue(failed ? 50 : 100);

		label_copy_bin->setPixmap(pixmap);
	}
	else if(step == 9)
	{
		progressBar_copy_cfg->setValue(failed ? 50 : 100);

		label_copy_cfg->setPixmap(pixmap);
	}
	else if(step == 10)
	{
		progressBar_copy_hosts->setValue(failed ? 50 : 100);

		label_copy_hosts->setPixmap(pixmap);
	}
	else if(step == 11)
	{
		progressBar_copy_rclocal->setValue(failed ? 50 : 100);

		label_copy_rclocal->setPixmap(pixmap);
	}
	else if(step == 12)
	{
		progressBar_chmod_bin->setValue(failed ? 50 : 100);

		label_chmod_bin->setPixmap(pixmap);
	}
	else if(step == 13)
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

	plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_HOSTS_DST).arg(VALETUDO_HOSTS_SRC + "_robot"));

	ssh->pullFile(VALETUDO_HOSTS_SRC + "_robot", VALETUDO_HOSTS_DST);
}

void installerDialog::ssh_pullSuccessful(__attribute__((unused)) QString localFile, QString remoteFile)
{
	plainTextEdit->appendPlainText(tr("SCP: finished\n"));

	QThread::msleep(250);

	if(remoteFile == VALETUDO_HOSTS_DST)
	{
		updateProgress();

		mergeFiles(HOSTS);

		plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_RCLOCAL_DST).arg(VALETUDO_RCLOCAL_SRC + "_robot"));

		ssh->pullFile(VALETUDO_RCLOCAL_SRC + "_robot", VALETUDO_RCLOCAL_DST);
	}
	else if(remoteFile == VALETUDO_RCLOCAL_DST)
	{
		updateProgress();

		mergeFiles(RCLOCAL);

		ssh->executeCommand(VALETUDO_CMD_STOP);
	}
}

void installerDialog::ssh_pushSuccessful(__attribute__((unused)) QString localFile, QString remoteFile)
{
	plainTextEdit->appendPlainText(tr("SCP: finished\n"));

	QThread::msleep(250);

	if(remoteFile == VALETUDO_BIN_DST)
	{
		updateProgress();

		plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_CFG_SRC).arg(VALETUDO_CFG_DST));

		ssh->pushFile(VALETUDO_CFG_SRC, VALETUDO_CFG_DST);
	}
	else if(remoteFile == VALETUDO_CFG_DST)
	{
		updateProgress();

		plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_HOSTS_SRC).arg(VALETUDO_HOSTS_DST));

		ssh->pushFile(VALETUDO_HOSTS_SRC, VALETUDO_HOSTS_DST);
	}
	else if(remoteFile == VALETUDO_HOSTS_DST)
	{
		updateProgress();

		plainTextEdit->appendPlainText(tr("SCP: %1 -> %2 started\n").arg(VALETUDO_RCLOCAL_SRC).arg(VALETUDO_RCLOCAL_DST));

		ssh->pushFile(VALETUDO_RCLOCAL_SRC, VALETUDO_RCLOCAL_DST);
	}
	else if(remoteFile == VALETUDO_RCLOCAL_DST)
	{
		updateProgress();

		ssh->executeCommand(VALETUDO_CMD_CHMOD);
	}
}

void installerDialog::ssh_commandExecuted(QString command, QString response)
{
	plainTextEdit->appendPlainText(tr("SSH: %1 -> %2\n").arg(command).arg(response.isEmpty() ? QString("OK") : response));

	QThread::msleep(250);

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
	ssh = nullptr;

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
		label_download_hosts_github->setPixmap(QPixmap(":/png/png/question.png"));
		label_download_rcocal_github->setPixmap(QPixmap(":/png/png/question.png"));
		label_download_hosts_robot->setPixmap(QPixmap(":/png/png/question.png"));
		label_download_rclocal_robot->setPixmap(QPixmap(":/png/png/question.png"));
		label_stop_service->setPixmap(QPixmap(":/png/png/question.png"));
		label_copy_bin->setPixmap(QPixmap(":/png/png/question.png"));
		label_copy_cfg->setPixmap(QPixmap(":/png/png/question.png"));
		label_copy_hosts->setPixmap(QPixmap(":/png/png/question.png"));
		label_copy_rclocal->setPixmap(QPixmap(":/png/png/question.png"));
		label_chmod_bin->setPixmap(QPixmap(":/png/png/question.png"));
		label_start_service->setPixmap(QPixmap(":/png/png/question.png"));

		progressBar_download_bin->setValue(0);
		progressBar_download_cfg->setValue(0);
		progressBar_download_hosts_github->setValue(0);
		progressBar_download_rclocal_github->setValue(0);
		progressBar_download_hosts_robot->setValue(0);
		progressBar_download_rclocal_robot->setValue(0);
		progressBar_stop_service->setValue(0);
		progressBar_copy_bin->setValue(0);
		progressBar_copy_cfg->setValue(0);
		progressBar_copy_hosts->setValue(0);
		progressBar_copy_rclocal->setValue(0);
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

		if(Download(VALETUDO_HOSTS))
		{
			QMessageBox::warning(this, APPNAME, tr("Valetudo installation failed!"));

			return;
		}

		if(Download(VALETUDO_RCLOCAL))
		{
			QMessageBox::warning(this, APPNAME, tr("Valetudo installation failed!"));

			return;
		}

		ssh = new QSshSocket(this);

		connect(ssh, SIGNAL(connected()), this, SLOT(ssh_connected()));
		connect(ssh, SIGNAL(disconnected()), this, SLOT(ssh_disconnected()));
		connect(ssh, SIGNAL(error(QSshSocket::SshError)), this, SLOT(ssh_error(QSshSocket::SshError)));
		connect(ssh, SIGNAL(loginSuccessful()), this, SLOT(ssh_loginSuccessful()));
		connect(ssh, SIGNAL(pullSuccessful(QString, QString)), this, SLOT(ssh_pullSuccessful(QString, QString)));
		connect(ssh, SIGNAL(pushSuccessful(QString, QString)), this, SLOT(ssh_pushSuccessful(QString, QString)));
		connect(ssh, SIGNAL(commandExecuted(QString, QString)), this, SLOT(ssh_commandExecuted(QString, QString)));

		ssh->connectToHost(((MainWindow*)parent())->cfg.ip);
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
