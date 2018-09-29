#ifndef UPDATEDLG_H
#define UPDATEDLG_H

#include "ui_update.h"
#include "mainwindow.h"

class updateDialog : public QDialog, private Ui::Dialog_Update
{
	Q_OBJECT

public:

	updateDialog(QWidget*, QFile*);

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
	bool finished = false;
	bool canceled = false;
	bool requested = false;

private slots:

	void newConnection();
	void readyRead();
	void bytesWritten(qint64);
	void disconnected();

	void startUpdating();
	void timer_refreshTime();

	void on_buttonBox_clicked(QAbstractButton*);

	void reject();
};

#endif // UPDATEDLG_H
