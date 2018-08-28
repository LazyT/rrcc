#include "zones.h"

zonesDialog::zonesDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	foreach(CLEANZONE zone, ((MainWindow*)parent)->cfg.zones)
	{
		comboBox_zone->addItem(zone.label);
	}
}

void zonesDialog::on_comboBox_zone_currentIndexChanged(int index)
{
	label_current->setText(tr("Zone %1 / %2").arg(index + 1).arg(((MainWindow*)parent())->cfg.zones.count()));

	spinBox_x1->setValue(((MainWindow*)parent())->cfg.zones.at(index).x1);
	spinBox_y1->setValue(((MainWindow*)parent())->cfg.zones.at(index).y1);
	spinBox_x2->setValue(((MainWindow*)parent())->cfg.zones.at(index).x2);
	spinBox_y2->setValue(((MainWindow*)parent())->cfg.zones.at(index).y2);
	spinBox_times->setValue(((MainWindow*)parent())->cfg.zones.at(index).times);
}

void zonesDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	int index = comboBox_zone->currentIndex();

	if(buttonBox->standardButton(button) == QDialogButtonBox::Save)
	{
		if(comboBox_zone->currentText().isEmpty())
		{
			QMessageBox::warning(this, APPNAME, tr("Please enter a label for this zone!"));
		}
		else
		{
			comboBox_zone->setItemText(index, comboBox_zone->currentText());

			((MainWindow*)parent())->menu_map_zones->actions().at(index)->setText(comboBox_zone->currentText());

			((MainWindow*)parent())->cfg.zones.data()[index].label = comboBox_zone->currentText();
			((MainWindow*)parent())->cfg.zones.data()[index].x1 = spinBox_x1->value();
			((MainWindow*)parent())->cfg.zones.data()[index].y1 = spinBox_y1->value();
			((MainWindow*)parent())->cfg.zones.data()[index].x2 = spinBox_x2->value();
			((MainWindow*)parent())->cfg.zones.data()[index].y2 = spinBox_y2->value();
			((MainWindow*)parent())->cfg.zones.data()[index].times = spinBox_times->value();
		}
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Discard)
	{
		if(QMessageBox::question(this, APPNAME, tr("Really delete zone \"%1\"?").arg(comboBox_zone->currentText()), QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) ==  QMessageBox::Yes)
		{
			((MainWindow*)parent())->cfg.zones.removeAt(index);

			if(!((MainWindow*)parent())->cfg.zones.count())
			{
				close();
			}
			else
			{
				comboBox_zone->removeItem(index);

				((MainWindow*)parent())->menu_map_zones->removeAction(((MainWindow*)parent())->menu_map_zones->actions().at(index));
			}
		}
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
	{
		close();
	}
}
