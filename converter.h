#ifndef CONVERTERDLG_H
#define CONVERTERDLG_H

#include "ui_converter.h"
#include "mainwindow.h"

class converterDialog : public QDialog, private Ui::Dialog_Converter
{
	Q_OBJECT

public:

	converterDialog(QWidget*);

private slots:

	void on_lineEdit_96_textChanged(QString);
	void on_lineEdit_16_textChanged(QString);

	void on_buttonBox_clicked(QAbstractButton*);
};

#endif // CONVERTERDLG_H
