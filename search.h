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

	QNetworkRequest request;
	QNetworkReply *reply = NULL;
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
