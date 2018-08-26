#include "timer.h"

timerDialog::timerDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	if(((MainWindow*)parent)->robo.timers.size())
	{
		foreach(TIMER val, ((MainWindow*)parent)->robo.timers)
		{
			comboBox->addItem(QString("ID %1").arg(val.id), val.id);
		}
	}
	else
	{
		addTimer();
	}

	buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Add Timer"));
	buttonBox->button(QDialogButtonBox::No)->setText(tr("Del Timer"));
}

void timerDialog::addTimer()
{
	buttonBox->button(QDialogButtonBox::Yes)->setEnabled(false);
	buttonBox->button(QDialogButtonBox::No)->setEnabled(false);
	comboBox->setEnabled(false);

	comboBox_fanspeed->setCurrentIndex(1);

	dial_hour->setValue(10);
	dial_minute->setValue(0);
	dial_day->setValue(0);
	dial_month->setValue(0);

	lcdNumber_day->display("-");
	lcdNumber_month->display("-");
	toolButton_mon->setChecked(true);
	toolButton_tue->setChecked(false);
	toolButton_wed->setChecked(true);
	toolButton_thu->setChecked(false);
	toolButton_fri->setChecked(true);
	toolButton_sat->setChecked(false);
	toolButton_sun->setChecked(false);
	pushButton_state->setChecked(true);

	comboBox->addItem(QString("ID %1").arg(QDateTime::currentDateTime().toTime_t()), QDateTime::currentDateTime().toTime_t());
	comboBox->setCurrentIndex(comboBox->count() - 1);
}

void timerDialog::delTimer()
{
	if(((MainWindow*)parent())->sendUDP(QString(MIIO_DEL_TIMER).arg(comboBox->itemData(comboBox->currentIndex()).toLongLong()).arg("%1")))
	{
		((MainWindow*)parent())->robo.timers.remove(comboBox->currentIndex(), 1);

		comboBox->removeItem(comboBox->currentIndex());

		if(!comboBox->count())
		{
			close();
		}
	}
}

void timerDialog::on_comboBox_currentIndexChanged(int index)
{
	if(((MainWindow*)parent())->robo.timers.size() && ((MainWindow*)parent())->robo.timers.size() > index)
	{
		QStringList entries = ((MainWindow*)parent())->robo.timers.at(index).crontab.split(' ');

		if(((MainWindow*)parent())->robo.timers.at(index).fanspeed > FANSPEED_TURBO)
		{
			comboBox_fanspeed->setCurrentIndex(3);
		}
		else if(((MainWindow*)parent())->robo.timers.at(index).fanspeed > FANSPEED_BALANCED)
		{
			comboBox_fanspeed->setCurrentIndex(2);
		}
		else if(((MainWindow*)parent())->robo.timers.at(index).fanspeed > FANSPEED_QUIET)
		{
			comboBox_fanspeed->setCurrentIndex(1);
		}
		else
		{
			comboBox_fanspeed->setCurrentIndex(0);
		}

		pushButton_state->setChecked(((MainWindow*)parent())->robo.timers.at(index).state == "off" ? 0 : 1);

		dial_hour->setValue(entries.at(1) == "*" ? -1 : entries.at(1).toInt());
		dial_minute->setValue(entries.at(0) == "*" ? -1 : entries.at(0).toInt());
		dial_day->setValue(entries.at(2) == "*" ? 0 : entries.at(2).toInt());
		dial_month->setValue(entries.at(3) == "*" ? 0 : entries.at(3).toInt());

		emit on_dial_hour_valueChanged(dial_hour->value());
		emit on_dial_minute_valueChanged(dial_minute->value());
		emit on_dial_day_valueChanged(dial_day->value());
		emit on_dial_month_valueChanged(dial_month->value());

		toolButton_sun->setChecked(entries.at(4).contains("0"));
		toolButton_mon->setChecked(entries.at(4).contains("1"));
		toolButton_tue->setChecked(entries.at(4).contains("2"));
		toolButton_wed->setChecked(entries.at(4).contains("3"));
		toolButton_thu->setChecked(entries.at(4).contains("4"));
		toolButton_fri->setChecked(entries.at(4).contains("5"));
		toolButton_sat->setChecked(entries.at(4).contains("6"));
	}
}

void timerDialog::on_dial_hour_valueChanged(int value)
{
	lcdNumber_hour->display(value < 0 ? "-" : QString::number(value));
}

void timerDialog::on_dial_minute_valueChanged(int value)
{
	lcdNumber_minute->display(value < 0 ? "-" : QString::number(value));
}

void timerDialog::on_dial_day_valueChanged(int value)
{
	lcdNumber_day->display(value ? QString::number(value) : "-");
}

void timerDialog::on_dial_month_valueChanged(int value)
{
	lcdNumber_month->display(value ? QString::number(value) : "-");
}

void timerDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Apply)
	{
		int hour = dial_hour->value();
		int minute = dial_minute->value();
		int day = dial_day->value();
		int month = dial_month->value();
		QString weekdays;
		int fanspeeds[] = {FANSPEED_QUIET, FANSPEED_BALANCED, FANSPEED_TURBO, FANSPEED_MAXIMUM};

		if(toolButton_sun->isChecked())
		{
			weekdays.append("0,");
		}
		if(toolButton_mon->isChecked())
		{
			weekdays.append("1,");
		}
		if(toolButton_tue->isChecked())
		{
			weekdays.append("2,");
		}
		if(toolButton_wed->isChecked())
		{
			weekdays.append("3,");
		}
		if(toolButton_thu->isChecked())
		{
			weekdays.append("4,");
		}
		if(toolButton_fri->isChecked())
		{
			weekdays.append("5,");
		}
		if(toolButton_sat->isChecked())
		{
			weekdays.append("6,");
		}

		if(weekdays.isEmpty())
		{
			weekdays = "*";
		}
		else
		{
			weekdays.chop(1);
		}

		QString crontab = QString("%1 %2 %3 %4 %5").arg(minute < 0 ? "*" : QString::number(minute)).arg(hour < 0 ? "*" : QString::number(hour)).arg(day == 0 ? "*" : QString::number(day)).arg(month == 0 ? "*" : QString::number(month)).arg(weekdays);

		if(((MainWindow*)parent())->sendUDP(QString(MIIO_SET_TIMER).arg(comboBox->itemData(comboBox->currentIndex()).toLongLong()).arg(crontab).arg(fanspeeds[comboBox_fanspeed->currentIndex()]).arg("%1")))
		{
			if(!pushButton_state->isChecked())
			{
				((MainWindow*)parent())->sendUDP(QString(MIIO_UPD_TIMER).arg(comboBox->itemData(comboBox->currentIndex()).toLongLong()).arg("off").arg("%1"));
			}

			close();
		}
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Yes)
	{
		addTimer();
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::No)
	{
		delTimer();
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
