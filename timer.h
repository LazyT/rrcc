#ifndef TIMERDLG_H
#define TIMERDLG_H

#include "ui_timer.h"
#include "mainwindow.h"

class timerDialog : public QDialog, private Ui::Dialog_Timer
{
	Q_OBJECT

public:

	timerDialog(QWidget*);

private:

	void addTimer();
	void delTimer();

private slots:

	void on_comboBox_currentIndexChanged(int);

	void on_dial_hour_valueChanged(int);
	void on_dial_minute_valueChanged(int);
	void on_dial_day_valueChanged(int);
	void on_dial_month_valueChanged(int);

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // TIMERDLG_H
