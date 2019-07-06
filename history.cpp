#include "history.h"

historyDialog::historyDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->button(QDialogButtonBox::Save)->setText(tr("Export"));

	foreach(int id, reinterpret_cast<MainWindow*>(parent)->robo.cleansummary.id)
	{
		record = QDateTime::fromTime_t(static_cast<uint>(id));

		comboBox->addItem(record.toString("dddd, dd.MM.yyyy - hh:mm"), id);
	}
}

void historyDialog::on_comboBox_currentIndexChanged(int index)
{
	if(reinterpret_cast<MainWindow*>(parent())->sendUDP(QString(MIIO_GET_CLEAN_RECORD).arg(comboBox->itemData(index).toInt()).arg("%1")))
	{
		QTime time(0, 0, 0, 0);

		time = time.addSecs(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.duration);

		label_state->setPixmap((reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.complete ? QPixmap(":/png/png/complete.png") : QPixmap(":/png/png/canceled.png")));

		record = QDateTime::fromTime_t(static_cast<uint>(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.begin));
		lcdNumber_start->display(record.toString("hh:mm:ss"));

		record = QDateTime::fromTime_t(static_cast<uint>(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.finish));
		lcdNumber_stop->display(record.toString("hh:mm:ss"));

		lcdNumber_duration->display(time.toString("hh:mm:ss"));

		lcdNumber_area->display(QString("%1").arg(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.area / 1000000.0));

		label_error_text->setText(error_strings.at(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.error));
		label_error_icon->setPixmap((reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.error ? QPixmap(":/png/png/warning.png") : QPixmap(":/png/png/info.png")));
	}
}

void historyDialog::on_buttonBox_clicked(__attribute__((unused)) QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Save)
	{
		QString data("ID;DATE;BEGIN;FINISH;DURATION;AREA;ERROR;COMPLETE\n");

		foreach(int id, reinterpret_cast<MainWindow*>(parent())->robo.cleansummary.id)
		{
			if(reinterpret_cast<MainWindow*>(parent())->sendUDP(QString(MIIO_GET_CLEAN_RECORD).arg(id).arg("%1")))
			{
				QTime time(0, 0, 0, 0);

				time = time.addSecs(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.duration);

				data.append(QString("%1;%2;%3;%4;%5;%6;%7;%8\n").arg(id).arg(QDateTime::fromTime_t(static_cast<uint>(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.begin)).toString("dd.MM.yyyy")).arg(QDateTime::fromTime_t(static_cast<uint>(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.begin)).toString("hh:mm:ss")).arg(QDateTime::fromTime_t(static_cast<uint>(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.finish)).toString("hh:mm:ss")).arg(time.toString("hh:mm:ss")).arg(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.area / 1000000.0).arg(error_strings.at(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.error)).arg(reinterpret_cast<MainWindow*>(parent())->robo.cleanrecord.complete));
			}
			else
			{
				QMessageBox::warning(this, APPNAME, "Communication error during export!");

				break;
			}
		}

		QFile csv(QFileDialog::getSaveFileName(this, tr("Select file to export cleaning history"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "*.csv", nullptr, QFileDialog::DontUseNativeDialog));

		if(!csv.fileName().isEmpty())
		{
			if(csv.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				csv.write(data.toUtf8());
				csv.close();

				QMessageBox::information(this, APPNAME, tr("Cleaning history successfully exported."));
			}
			else
			{
				QMessageBox::warning(this, APPNAME, tr("Could not open csv export!\n\n%1 : %2").arg(csv.fileName()).arg(csv.errorString()));
			}
		}
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
