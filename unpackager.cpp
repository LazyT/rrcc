#include "unpackager.h"

unpackagerDialog::unpackagerDialog(QWidget *parent, QFile *package, QFile *directory) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	connect(&timer, SIGNAL(timeout()), this, SLOT(timer_refreshTime()));

	file_pkg = package;
	file_dir = directory;

	finished = false;
	canceled = false;

	time.start();
	timer.start(500);

	label_file->setText(file_pkg->fileName());
	label_size->setText(QString::number(file_pkg->size()));

	QTimer::singleShot(100, this, SLOT(startUnpackaging()));
}

void unpackagerDialog::startUnpackaging()
{
	QFile file_tgz(TGZ);
	QByteArray pkg;
	QByteArray tgz;
	ccrypt_stream_t ccrypt;
	short err_code;
	QString err_msg;
	int count = 0;

	pkg = file_pkg->readAll();
	file_pkg->close();

	tgz.resize(pkg.size() - 32);

	ccrypt.next_in = pkg.data();
	ccrypt.avail_in = static_cast<uint>(pkg.size());
	ccrypt.next_out = tgz.data();
	ccrypt.avail_out = static_cast<uint>(tgz.size());

	progressBar_decrypt->setValue(50);

	ccdecrypt_init(&ccrypt, PKG_KEY, 0);
	ccdecrypt(&ccrypt);
	ccdecrypt_end(&ccrypt);

	progressBar_decrypt->setValue(100);

	QCoreApplication::processEvents();

	if(file_tgz.open(QIODevice::WriteOnly))
	{
		file_tgz.write(tgz);

		file_tgz.close();
	}

	extractor
	.setArchive(file_tgz.fileName(), file_dir->fileName())
	.setFunc(QArchive::EXTRACTED, [&](QString file)
	{
		label_file->setText(QFileInfo(file).fileName());
		label_count->setText(QString::number(++count));
		label_size->setText(QString("%1 Byte").arg(QFileInfo(file).size()));
	})
	.setFunc(QArchive::PROGRESS, [&](int prog)
	{
		progressBar_decompress->setValue(prog);
	})
	.setFunc(QArchive::FINISHED, [&]()
	{
		finished = true;
	})
	.setFunc(QArchive::CANCELED, [&]()
	{
		canceled = true;
	})
	.setFunc([&](short errorCode, QString eMsg)
	{
		canceled = true;
		finished = true;
		err_code = errorCode;
		err_msg = eMsg;
	})
	.start();

	while(!finished && !canceled)
	{
		QThread::msleep(10);
		QCoreApplication::processEvents();
	}

	timer.stop();

	if(canceled)
	{
		if(!finished)
		{
			QMessageBox::warning(this, APPNAME, tr("Unpackaging aborted by user!"));
		}
		else
		{
			QMessageBox::critical(this, APPNAME, tr("Unpackaging error!\n\n%1\n\n%2").arg(((MainWindow*)parent())->qarchive_error_strings.at(err_code)).arg(err_msg));
		}

		close();

		return;
	}

	QDesktopServices::openUrl(QUrl("file:///" + file_dir->fileName()));

	close();
}

void unpackagerDialog::timer_refreshTime()
{
	QTime elapsed = QTime::fromMSecsSinceStartOfDay(time.elapsed());

	label_time->setText(QString("%1h:%2m:%3s").arg(elapsed.hour(), 2, 10, QChar('0')).arg(elapsed.minute(), 2, 10, QChar('0')).arg(elapsed.second(), 2, 10, QChar('0')));
}

void unpackagerDialog::reject()
{
	if(!finished && !canceled)
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort unpackaging?"), QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) ==  QMessageBox::Yes)
		{
			extractor.cancel();
		}
	}
	else
	{
		accept();
	}
}

void unpackagerDialog::on_buttonBox_clicked(__attribute__((unused)) QAbstractButton *button)
{
	close();
}
