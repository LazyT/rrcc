#include "onlineupd.h"

onlineUpdDialog::onlineUpdDialog(QWidget *parent, bool mode) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	silent = mode;

	textEdit->setFixedHeight(5 * textEdit->fontMetrics().height() + textEdit->document()->documentMargin() + 2);

	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Ignore"));
	buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Update"));

	label_icon->setPixmap(QPixmap(IMG));

	QTimer::singleShot(1, this, SLOT(checkUpdate()));
}

void onlineUpdDialog::checkUpdate()
{
	QElapsedTimer timeout;
	netmgr = new QNetworkAccessManager(this);
	request = QNetworkRequest(QUrl(RELEASE));

	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

	reply = netmgr->get(request);

	reply->ignoreSslErrors();

	timeout.start();

	while(reply->isRunning())
	{
		QCoreApplication::processEvents();

		if(timeout.hasExpired(5000))
		{
			reply->abort();
		}
	}

	if(!reply->error())
	{
		QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
		QJsonObject obj = doc.object();
		QJsonArray arr;
		QString version, date, changelog;

		if(obj.contains("tag_name"))
		{
			version = obj.value("tag_name").toString();
		}

		if(obj.contains("published_at"))
		{
			date = obj.value("published_at").toString();
		}

		if(obj.contains("body"))
		{
			changelog = obj.value("body").toString();
		}

		if(obj.contains("assets"))
		{
			arr = obj.value("assets").toArray();

			for(int i = 0; i < arr.count(); i++)
			{
				if(arr[i].toObject().contains("size"))
				{
					size = arr[i].toObject().value("size").toInt();
				}

				if(arr[i].toObject().contains("browser_download_url"))
				{
					url = arr[i].toObject().value("browser_download_url").toString();

					if(url.endsWith(BIN))
					{
						break;
					}
				}
			}
		}

		if(QVersionNumber::compare(QVersionNumber::fromString(version), QVersionNumber::fromString(APPVERS)) == 1)
		{
			label_installed->setText(QString("%1 [ %2 ]").arg(APPVERS).arg(APPDATE));
			label_available->setText(QString("%1 [ %2 ]").arg(version).arg(QDate::fromString(date.left(10), "yyyy-MM-dd").toString("dd.MM.yyyy")));

			textEdit->setText(changelog);

			progressBar->setFormat(QString("%p% [ 0 / %1 ]").arg(size));

			show();
		}
		else
		{
			if(!silent)
			{
				QMessageBox::information((QWidget*)parent(), APPNAME, tr("No new version found."));
			}

			close();
		}
	}
	else
	{
		if(!silent)
		{
			QMessageBox::warning((QWidget*)parent(), APPNAME, tr("Online update check failed!\n\n%1").arg(reply->errorString()));
		}

		close();
	}
}

void onlineUpdDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if(bytesReceived && bytesTotal)
	{
		progressBar->setValue((100 * bytesReceived) / bytesTotal);
		progressBar->setFormat(QString("%p% [ %1 / %2 ]").arg(bytesReceived).arg(bytesTotal));
	}
}

void onlineUpdDialog::finished(QNetworkReply *reply)
{
	if(reply->error())
	{
		QMessageBox::warning(this, APPNAME, tr("Download from Github failed!\n\n%1").arg(reply->errorString()));
	}
	else
	{
		download = reply->readAll();

		QFile file(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + url.split('/').last());

		if(file.open(QIODevice::WriteOnly))
		{
			file.write(download);
#ifdef Q_OS_LINUX
			file.setPermissions(file.permissions() | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
#endif
			file.close();

			if(QMessageBox::question(this, APPNAME, tr("Download successfully saved to %1.\n\nRun new version now?").arg(file.fileName()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
			{
#ifdef Q_OS_OSX
				if(QProcess::startDetached("open", {file.fileName()}))
#else
				if(QProcess::startDetached(file.fileName()))
#endif
				{
					((MainWindow*)parent())->forceclose = true;

					((MainWindow*)parent())->close();
				}
				else
				{
					QMessageBox::warning(this, APPNAME, tr("Could not run new version!"));
				}
			}
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Could not save download to %1!\n\n%2").arg(file.fileName()).arg(file.errorString()));
		}
	}

	close();
}

void onlineUpdDialog::reject()
{
	if(reply->isRunning())
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort download?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			reply->abort();
		}
		else
		{
			return;
		}
	}

	accept();
}

void onlineUpdDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
	{
		close();
	}
	else
	{
		netmgr = new QNetworkAccessManager(this);
		request = QNetworkRequest(url);

		request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

		reply = netmgr->get(request);

		reply->ignoreSslErrors();

		connect(netmgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
		connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));

		buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);
		buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	}
}
