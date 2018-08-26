#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include "ui_about.h"
#include "mainwindow.h"

class aboutDialog : public QDialog, private Ui::Dialog_About
{
	Q_OBJECT

public:

	aboutDialog(QWidget*);

private slots:

	void on_label_qsshsocket_linkActivated(QString);
	void on_label_qaes_linkActivated(QString);
	void on_label_qarchive_linkActivated(QString);
	void on_label_ccrypt_linkActivated(QString);
	void on_label_libarchive_linkActivated(QString);
	void on_label_zlib_linkActivated(QString);
	void on_label_libssh_linkActivated(QString);
	void on_label_openssl_linkActivated(QString);
	void on_label_qt_linkActivated(QString);

	void mouseReleaseEvent(QMouseEvent*);
};

#endif // ABOUTDLG_H
