#include "history.h"

historyDialog::historyDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	foreach(int id, ((MainWindow*)parent)->robo.cleansummary.id)
	{
		record = QDateTime::fromTime_t(id);

		comboBox->addItem(record.toString("dddd, dd.MM.yyyy - hh:mm"), id);
	}
}

void historyDialog::on_comboBox_currentIndexChanged(int index)
{
	if(((MainWindow*)parent())->sendUDP(QString(MIIO_GET_CLEAN_RECORD).arg(comboBox->itemData(index).toInt()).arg("%1")))
	{
		QTime time(0, 0, 0, 0);

		time = time.addSecs(((MainWindow*)parent())->robo.cleanrecord.duration);

		label_state->setPixmap((((MainWindow*)parent())->robo.cleanrecord.complete ? QPixmap(":/png/png/complete.png") : QPixmap(":/png/png/canceled.png")));

		record = QDateTime::fromTime_t(((MainWindow*)parent())->robo.cleanrecord.begin);
		lcdNumber_start->display(record.toString("hh:mm:ss"));

		record = QDateTime::fromTime_t(((MainWindow*)parent())->robo.cleanrecord.finish);
		lcdNumber_stop->display(record.toString("hh:mm:ss"));

		lcdNumber_duration->display(time.toString("hh:mm:ss"));

		lcdNumber_area->display(QString("%1").arg(((MainWindow*)parent())->robo.cleanrecord.area / 1000000.0));

		label_error_text->setText(error_strings.at(((MainWindow*)parent())->robo.cleanrecord.error));
		label_error_icon->setPixmap((((MainWindow*)parent())->robo.cleanrecord.error ? QPixmap(":/png/png/warning.png") : QPixmap(":/png/png/info.png")));
	}
}

void historyDialog::on_buttonBox_clicked(__attribute__((unused)) QAbstractButton *button)
{
	close();
}
