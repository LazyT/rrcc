#include "update.h"

updateDialog::updateDialog(QWidget *parent, QFile *package) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	file = package;

	label_file->setText(QFileInfo(file->fileName()).fileName());
	label_bytes->setText(QString("0 / %1 Byte").arg(file->size()));

	connect(&timer, SIGNAL(timeout()), this, SLOT(timer_refreshTime()));

	time.start();
	timer.start(500);

	QTimer::singleShot(1, this, SLOT(startUpdating()));
}

void updateDialog::startUpdating()
{
	pkg = file->readAll();
	file->close();

	size = pkg.size();

	server = new QTcpServer(this);
	socket = nullptr;

	connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

	if(server->listen(QHostAddress(((MainWindow*)parent())->src_ip), 8080))
	{
		if(!((MainWindow*)parent())->sendUDP(QString(MIIO_OTA).arg(QString("http://%1:8080/%2").arg(((MainWindow*)parent())->src_ip).arg(QFileInfo(file->fileName()).fileName())).arg(QString(QCryptographicHash::hash(pkg, QCryptographicHash::Md5).toHex())).arg("%1")))
		{
			timer.stop();

			canceled = true;

			QMessageBox::warning(this, APPNAME, tr("Could not start firmware update!"));

			close();
		}
	}
	else
	{
		timer.stop();

		canceled = true;

		QMessageBox::warning(this, APPNAME, tr("Could not start HTTP server!\n\n%1").arg(server->errorString()));

		close();
	}
}

void updateDialog::timer_refreshTime()
{
	QTime elapsed = QTime::fromMSecsSinceStartOfDay(time.elapsed());

	label_time->setText(QString("%1h:%2m:%3s").arg(elapsed.hour(), 2, 10, QChar('0')).arg(elapsed.minute(), 2, 10, QChar('0')).arg(elapsed.second(), 2, 10, QChar('0')));

	if(!requested && time.elapsed() >= 15000)
	{
		timer.stop();

		canceled = true;

		QMessageBox::warning(this, APPNAME, tr("Robot does not send firmware package request!\n\nMake sure your firewall accepts incoming tcp connections for\n\n   %1 -> %2:8080\n\nand try again...").arg(((MainWindow*)parent())->cfg.ip).arg(((MainWindow*)parent())->src_ip));

		close();
	}
}

void updateDialog::newConnection()
{
	socket = server->nextPendingConnection();

	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));

	requested = true;
}

void updateDialog::readyRead()
{
	socket->readAll();

	header = QString("HTTP/1.1 200 OK\r\nDate: %1 GMT\r\nContent-Type: application/octet-stream\r\nContent-Length: %2\r\nConnection: close\r\n\r\n").arg(QLocale("en_US").toString(QDateTime::currentDateTime().toUTC(), "ddd, dd MMM yyyy hh:mm:ss")).arg(size).toUtf8();

	total = header.size() * -1;

	socket->write(header);
	socket->write(pkg);
}

void updateDialog::bytesWritten(qint64 byte)
{
	total += byte;

	progressBar_upload->setValue((100 * total) / size);
	label_bytes->setText(QString("%1 / %2 Byte").arg(total).arg(size));
}

void updateDialog::disconnected()
{
	finished = true;

	socket->close();
	server->close();

	if(canceled)
	{
		timer.stop();

		QMessageBox::warning(this, APPNAME, tr("Update aborted by user!"));

		close();

		return;
	}

	progressBar_install->setValue(50);

	do
	{
		((MainWindow*)parent())->sendUDP(MIIO_GET_OTA_STATE);

		QThread::msleep(1000);
		QCoreApplication::processEvents();
	}
	while(((MainWindow*)parent())->robo.ota.state == "downloading" || ((MainWindow*)parent())->robo.ota.state == "installing");

	progressBar_install->setValue(100);

	timer.stop();

	((MainWindow*)parent())->robo.ota.state == "failed" ? QMessageBox::warning(this, APPNAME, tr("Firmware update failed!")) : QMessageBox::information(this, APPNAME, tr("Firmware update successful."));

	((MainWindow*)parent())->sendUDP(nullptr);

	close();
}

void updateDialog::reject()
{
	if(!finished && !canceled)
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort update?"), QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) ==  QMessageBox::Yes)
		{
			canceled = true;

			if(socket)
			{
				socket->abort();

				reject();
			}
			else
			{
				server->close();

				accept();
			}
		}
	}
	else
	{
		accept();
	}
}

void updateDialog::on_buttonBox_clicked(__attribute__((unused)) QAbstractButton *button)
{
	close();
}
