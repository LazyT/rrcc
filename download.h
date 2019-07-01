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

	struct
	{
		QString names[3] = { FW_NAME_G1, FW_NAME_G2, FW_NAME_G3 };
		QString dirs[3] = { FW_DIR_G1, FW_DIR_G2, FW_DIR_G3 };
		int versions[3] = { FW_VER_G1, FW_VER_G2, FW_VER_G3 };

	}fw;

	bool Download(QString);

	QNetworkReply *reply = nullptr;
	QByteArray download;

	bool finish, failed;

private slots:

	void startDownload();

	void downloadProgress(qint64, qint64);
	void finished(QNetworkReply*);

	void on_comboBox_model_currentIndexChanged(int);
	void reject();
	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // DOWNLOADDLG_H
