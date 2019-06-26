#include "about.h"

aboutDialog::aboutDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	label_Version->setText(tr("Version %1 - %2").arg(APPVERS, APPDATE));

	textEdit->setAlignment(Qt::AlignCenter);
	textEdit->textCursor().insertText(tr("This program is Freeware and may be installed and used free of charge for non-commercial use on as many computers as you like without limitations.\n\nA liability for any damages resulting from the use is excluded. Use at your own risk!"));
	textEdit->moveCursor(QTextCursor::Start);
	textEdit->setFixedHeight(5 * textEdit->fontMetrics().height() + static_cast<int>(textEdit->document()->documentMargin()) + 2);
}

void aboutDialog::mouseReleaseEvent(QMouseEvent *me)
{
	QWidget *child = QWidget::childAt(me->pos());

	if(child)
	{
		QString name = child->objectName();

		if(name == "label_Mail")
		{
			QDesktopServices::openUrl(QUrl(QByteArray::fromBase64("bWFpbHRvOkxhenlUQG1haWxib3gub3JnP3N1YmplY3Q9UlJDQyZib2R5PVdyaXRlIGluIEVuZ2xpc2ggb3IgR2VybWFuIHBsZWFzZS4uLg==")));
		}
		else if(name == "label_Forum")
		{
			QDesktopServices::openUrl(QUrl(QByteArray::fromBase64("aHR0cHM6Ly93d3cucm9ib3Rlci1mb3J1bS5jb20vaW5kZXgucGhwP3RocmVhZC8zMDMwOS1yb2Jvcm9jay1jb250cm9sLWNlbnRlci1kZXNrdG9wLWFwcC1mJUMzJUJDci13aW4tbGluLW9zeA==")));
		}
		else if(name == "label_Donation")
		{
			if(QMessageBox::warning(this, APPNAME, tr("Please note the following points:\n\n* The payment is made voluntarily without the acquisition of claims.\n* You receive no rights to the offered software.\n* Because this is not a donation in juridical sense no certificate can be issued.\n\nWould you like to support the further development of this project nevertheless?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
			{
				QDesktopServices::openUrl(QUrl(QByteArray::fromBase64("aHR0cHM6Ly93d3cucGF5cGFsLmNvbS9jZ2ktYmluL3dlYnNjcj9jbWQ9X2RvbmF0aW9ucyZidXNpbmVzcz1MYXp5VEBtYWlsYm94Lm9yZyZpdGVtX25hbWU9Um9ib1JvY2srQ29udHJvbCtDZW50ZXImYW1vdW50PTAmY3VycmVuY3lfY29kZT1FVVI=")));
			}
		}
	}
}

void aboutDialog::on_label_qsshsocket_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_qaes_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_qarchive_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_ccrypt_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_libarchive_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_zlib_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_libssh_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_openssl_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}

void aboutDialog::on_label_qt_linkActivated(QString link)
{
	QDesktopServices::openUrl(link);
}
