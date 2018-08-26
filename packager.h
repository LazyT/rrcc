#ifndef PACKAGERDLG_H
#define PACKAGERDLG_H

#include "ui_packager.h"
#include "mainwindow.h"

class packagerDialog : public QDialog, private Ui::Dialog_Packager
{
	Q_OBJECT

public:

	packagerDialog(QWidget*, QFile*);

private:

	QArchive::Compressor compressor;
	QTime time;
	QTimer timer;
	QFile *dir;
	bool finished;
	bool canceled;

private slots:

	void startPackaging();
	void timer_refreshTime();

	void on_buttonBox_clicked(QAbstractButton*);

	void reject();
};

#endif // PACKAGERDLG_H
