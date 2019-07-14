#ifndef ZONESDLG_H
#define ZONESDLG_H

#include "ui_zones.h"
#include "mainwindow.h"

class zonesDialog : public QDialog, private Ui::Dialog_Zones
{
	Q_OBJECT

public:

	zonesDialog(QWidget*);

private slots:

	void on_tabWidget_currentChanged(int);

	void on_comboBox_zone_currentIndexChanged(int);
	void on_comboBox_goto_currentIndexChanged(int);

	void zonePositionChanged();
	void on_spinBox_x1_editingFinished();
	void on_spinBox_y1_editingFinished();
	void on_spinBox_x2_editingFinished();
	void on_spinBox_y2_editingFinished();
	void on_spinBox_x1_valueChanged(int);
	void on_spinBox_y1_valueChanged(int);
	void on_spinBox_x2_valueChanged(int);
	void on_spinBox_y2_valueChanged(int);

	void gotoPositionChanged();
	void on_spinBox_x_editingFinished();
	void on_spinBox_y_editingFinished();
	void on_spinBox_x_valueChanged(int);
	void on_spinBox_y_valueChanged(int);

	void on_buttonBox_clicked(QAbstractButton*);

	void reject();
};

#endif // ZONESDLG_H
