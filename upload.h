#ifndef UPLOADDLG_H
#define UPLOADDLG_H

#include "ui_upload.h"
#include "mainwindow.h"

class uploadDialog : public QDialog, private Ui::Dialog_Upload
{
	Q_OBJECT

public:

	uploadDialog(QWidget*, QFile*);

private:

	QTcpServer *server;
	QTcpSocket *socket;
	QByteArray header;
	QByteArray pkg;
	QTime time;
	QTimer timer;
	QFile *file;
	int size;
	quint64 total;
	bool finished;
	bool canceled;

private slots:

	void newConnection();
	void readyRead();
	void bytesWritten(qint64);
	void disconnected();

	void startUploading();
	void timer_refreshTime();

	void on_buttonBox_clicked(QAbstractButton*);

	void reject();
};

#endif // UPLOADDLG_H
