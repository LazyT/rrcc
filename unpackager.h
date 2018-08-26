#ifndef UNPACKAGERDLG_H
#define UNPACKAGERDLG_H

#include "ui_unpackager.h"
#include "mainwindow.h"

class unpackagerDialog : public QDialog, private Ui::Dialog_Unpackager
{
	Q_OBJECT

public:

	unpackagerDialog(QWidget*, QFile*, QFile*);

private:

	QArchive::Extractor extractor;
	QTime time;
	QTimer timer;
	QFile *file_pkg;
	QFile *file_dir;
	bool finished;
	bool canceled;

private slots:

	void startUnpackaging();
	void timer_refreshTime();

	void on_buttonBox_clicked(QAbstractButton*);

	void reject();
};

#endif // UNPACKAGERDLG_H
