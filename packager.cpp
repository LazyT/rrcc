#include "packager.h"

packagerDialog::packagerDialog(QWidget *parent, QFile *file) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	connect(&timer, SIGNAL(timeout()), this, SLOT(timer_refreshTime()));

	dir = file;

	finished = false;
	canceled = false;

	time.start();
	timer.start(500);

	QTimer::singleShot(100, this, SLOT(startPackaging()));
}

void packagerDialog::startPackaging()
{
	QFileInfoList entries;
	QStringList files;
	QFile file;
	QByteArray tgz;
	QByteArray pkg;
	ccrypt_stream_t ccrypt;
	int index = 0;
	short err_code;
	QString err_msg;

	entries = QDir(dir->fileName(), "*.wav", QDir::Name, QDir::Files).entryInfoList();

	foreach(QFileInfo fi, entries)
	{
		files.append(fi.canonicalFilePath());
	}

	file.setFileName(TGZ);
	file.remove();

	compressor
	.setArchive(file.fileName(), files)
	.setArchiveFormat(QArchive::GZIP)
	.setFunc(QArchive::COMPRESSED, [&](__attribute__ ((unused)) QString file)
	{
		label_file->setText(entries.at(index).fileName());
		label_count->setText(QString("%1 / %2").arg(1 + index).arg(entries.count()));
		label_size->setText(QString("%1 Byte").arg(entries.at(index++).size()));
	})
	.setFunc([&](int prog)
	{
		progressBar_compress->setValue(prog);
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

	if(canceled)
	{
		timer.stop();

		if(!finished)
		{
			QMessageBox::warning(this, APPNAME, tr("Packaging aborted by user!"));
		}
		else
		{
			QMessageBox::critical(this, APPNAME, tr("Packaging error!\n\n%1\n\n%2").arg(((MainWindow*)parent())->qarchive_error_strings.at(err_code)).arg(err_msg));
		}

		close();

		return;
	}

	if(file.open(QIODevice::ReadOnly) && file.size())
	{
		tgz = file.readAll();

		file.close();
	}
	else
	{
		QMessageBox::warning(this, APPNAME, tr("Creating voice package failed!"));

		return;
	}

	pkg.resize(32 + tgz.size());

	ccrypt.next_in = tgz.data();
	ccrypt.avail_in = static_cast<uint>(tgz.size());
	ccrypt.next_out = pkg.data();
	ccrypt.avail_out = static_cast<uint>(pkg.size());

	progressBar_encrypt->setValue(50);

	ccencrypt_init(&ccrypt, PKG_KEY);
	ccencrypt(&ccrypt);
	ccencrypt_end(&ccrypt);

	progressBar_encrypt->setValue(100);

	QCoreApplication::processEvents();

	timer.stop();

	file.setFileName(PKG);
	file.remove();

	if(file.open(QIODevice::WriteOnly) && pkg.size() > 32)
	{
		file.write(pkg);

		file.close();

		if(QMessageBox::question(this, APPNAME, tr("Install created voice package now?"), QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) ==  QMessageBox::Yes)
		{
			hide();

			file.open(QIODevice::ReadOnly);

			uploadDialog((QWidget*)parent(), &file).exec();
		}
		else
		{
			QFile save(QFileDialog::getSaveFileName(this, tr("Select file to save voice package"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/rrcc-voice.pkg", "*.pkg", nullptr, QFileDialog::DontUseNativeDialog));

			if(!save.fileName().isEmpty())
			{
				if(save.exists())
				{
					save.remove();
				}

				file.copy(save.fileName());
			}
		}
	}
	else
	{
		QMessageBox::warning(this, APPNAME, tr("Crypting voice file failed!"));
	}

	close();
}

void packagerDialog::timer_refreshTime()
{
	QTime elapsed = QTime::fromMSecsSinceStartOfDay(time.elapsed());

	label_time->setText(QString("%1h:%2m:%3s").arg(elapsed.hour(), 2, 10, QChar('0')).arg(elapsed.minute(), 2, 10, QChar('0')).arg(elapsed.second(), 2, 10, QChar('0')));
}

void packagerDialog::reject()
{
	if(!finished && !canceled)
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort packaging?"), QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) ==  QMessageBox::Yes)
		{
			compressor.cancel();
		}
	}
	else
	{
		accept();
	}
}

void packagerDialog::on_buttonBox_clicked(__attribute__((unused)) QAbstractButton *button)
{
	close();
}
