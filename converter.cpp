#include "converter.h"

converterDialog::converterDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	lineEdit_96->setValidator(new QRegExpValidator(QRegExp("[0-9a-fA-F]{96}")));
}

void converterDialog::on_lineEdit_96_textChanged(QString text)
{
	lineEdit_96->setText(lineEdit_96->text().toUpper());

	lineEdit_32->setText(QAESEncryption::Decrypt(QAESEncryption::AES_128, QAESEncryption::ECB, QByteArray::fromHex(text.left(64).toUtf8()), QByteArray::fromHex("00000000000000000000000000000000")).toUpper());
}

void converterDialog::on_lineEdit_16_textChanged(QString text)
{
	lineEdit_32->setText(text.toUtf8().toHex().toUpper());
}

void converterDialog::on_buttonBox_clicked(__attribute__((unused)) QAbstractButton *button)
{
	close();
}
