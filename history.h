#ifndef HISTORYDLG_H
#define HISTORYDLG_H

#include "ui_history.h"
#include "mainwindow.h"

class historyDialog : public QDialog, private Ui::Dialog_History
{
	Q_OBJECT

public:

	historyDialog(QWidget*);

private:

	QStringList error_strings
	{
		tr("No error"),
		tr("Laser distance sensor error"),
		tr("Collision sensor error"),
		tr("Wheels on top of void, move robot"),
		tr("Clean hovering sensors, move robot"),
		tr("Clean main brush"),
		tr("Clean side brush"),
		tr("Main wheel stuck?"),
		tr("Device stuck, clean area"),
		tr("Dust collector missing"),
		tr("Clean filter"),
		tr("Stuck in magnetic barrier"),
		tr("Low battery"),
		tr("Charging fault"),
		tr("Battery fault"),
		tr("Wall sensors dirty, wipe them"),
		tr("Place me on flat surface"),
		tr("Side brushes problem, reboot me"),
		tr("Suction fan problem"),
		tr("Unpowered charging station")
	};

	QDateTime record;

private slots:

	void on_comboBox_currentIndexChanged(int);

	void on_buttonBox_clicked(QAbstractButton *button);
};

#endif // HISTORYDLG_H
