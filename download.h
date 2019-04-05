#ifndef DOWNLOADDLG_H
#define DOWNLOADDLG_H

#include "ui_download.h"
#include "mainwindow.h"

class downloadDialog : public QDialog, private Ui::Dialog_Download
{
	Q_OBJECT

public:

	downloadDialog(QWidget*, int, int, QString);

private:

	bool Download(QString);

	QNetworkReply *reply = NULL;
	QByteArray download;

	bool finish, failed;

private slots:

	void startDownload();

	void downloadProgress(qint64, qint64);
	void finished(QNetworkReply*);

	void reject();

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // DOWNLOADDLG_H
