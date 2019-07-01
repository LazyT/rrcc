#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include "ui_search.h"
#include "mainwindow.h"

class searchDialog : public QDialog, private Ui::Dialog_Search
{
	Q_OBJECT

public:

	searchDialog(QWidget*);

private:

	struct
	{
		QString names[3] = { FW_NAME_G1, FW_NAME_G2, FW_NAME_G3 };
		QString dirs[3] = { FW_DIR_G1, FW_DIR_G2, FW_DIR_G3 };
		int versions[3] = { FW_VER_G1, FW_VER_G2, FW_VER_G3 };

	}fw;

	QNetworkRequest request;
	QNetworkReply *reply = nullptr;
	int counter;
	int version;
	bool abort;

private slots:

	void calcHeight();
	void metaDataChanged();
	void on_comboBox_model_currentIndexChanged(int);
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem*, int);
	void reject();
	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // SEARCHDLG_H
