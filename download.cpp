#include "download.h"

downloadDialog::downloadDialog(QWidget *parent, int model, int server, QString firmware) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->button(QDialogButtonBox::Save)->setText(tr("Download"));

	comboBox_model->setCurrentIndex(model);
	comboBox_server->setCurrentIndex(server);

	if(!firmware.isEmpty())
	{
		lineEdit->setText(firmware);

		QTimer::singleShot(1, this, SLOT(startDownload()));
	}
	else
	{
		lineEdit->setFocus();
		lineEdit->setSelection(6, 4);
	}

	lineEdit->setMinimumWidth(lineEdit->fontMetrics().boundingRect(lineEdit->text()).width() + 15);
}

void downloadDialog::startDownload()
{
	emit on_buttonBox_clicked(buttonBox->button(QDialogButtonBox::Save));
}

bool downloadDialog::Download(QString url)
{
	QNetworkAccessManager *netmgr = new QNetworkAccessManager(this);
	QNetworkRequest request = QNetworkRequest(QUrl(url));

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

	return failed;
}

void downloadDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if(bytesTotal)
	{
		progressBar->setFormat(QString("%p% [%1 / %2]").arg(bytesReceived).arg(bytesTotal));
		progressBar->setValue(static_cast<int>((bytesReceived * 100) / bytesTotal));
	}
}

void downloadDialog::finished(QNetworkReply *reply)
{
	finish = true;

	if(reply->error())
	{
		failed = true;

		buttonBox->button(QDialogButtonBox::Save)->setDisabled(false);

		progressBar->setFormat(QString("%p%"));
		progressBar->setValue(0);

		QMessageBox::warning(this, APPNAME, tr("Downloading firmware failed!\n\n%1").arg(reply->errorString()));

		return;
	}

	download = reply->readAll();

	failed = false;
}

void downloadDialog::reject()
{
	if(reply && reply->isRunning())
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort download?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			reply->abort();
		}

		return;
	}

	accept();
}

void downloadDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Save)
	{
		if(lineEdit->text() == "v11_00????.fullos.pkg")
		{
			QMessageBox::warning(this, APPNAME, tr("Enter valid firmware name!"));

			return;
		}

		QFile firmware(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QString("/%1").arg(lineEdit->text()));

		if(firmware.open(QIODevice::WriteOnly))
		{
			button->setDisabled(true);

			if(!Download(QString("http://%1/%2/%3").arg(comboBox_server->currentText()).arg(comboBox_model->currentIndex() ? "rubys/updpkg" : "updpkg").arg(lineEdit->text())))
			{
				firmware.write(download);

				hide();

				QMessageBox::information(this, APPNAME, tr("Downloading firmware successfull."));

				close();
			}
			else
			{
				firmware.remove();
			}

			firmware.close();
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Could not open firmware file!\n\n%1 : %2").arg(firmware.fileName()).arg(firmware.errorString()));
		}
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
	{
		close();
	}
}
