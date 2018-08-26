#ifndef LOGGERDLG_H
#define LOGGERDLG_H

#include "ui_logger.h"
#include "mainwindow.h"

class loggerDialog : public QDialog, private Ui::Dialog_Logger
{
	Q_OBJECT

public:

	loggerDialog(QWidget*);

	void log(QString, QString, QString, QString, QString, QString);

private slots:

	void calcHeight();

	void on_customContextMenuRequested(QPoint);

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // LOGGERDLG_H
