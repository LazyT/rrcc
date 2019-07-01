#include "search.h"

searchDialog::searchDialog(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Search"));
	buttonBox->button(QDialogButtonBox::Apply)->setDefault(true);

	treeWidget->resizeColumnToContents(0);
	treeWidget->resizeColumnToContents(1);
	treeWidget->resizeColumnToContents(2);
	treeWidget->resizeColumnToContents(3);

	comboBox_model->currentIndexChanged(0);

	QTimer::singleShot(1, this, SLOT(calcHeight()));
}

void searchDialog::calcHeight()
{
	treeWidget->setMinimumHeight(10 * treeWidget->visualItemRect(treeWidget->topLevelItem(0)).height() + treeWidget->header()->height() + /*style()->pixelMetric(QStyle::PM_ScrollBarExtent)*/ + 2);

	treeWidget->clear();
}

void searchDialog::metaDataChanged()
{
	if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200)
	{
		reply->abort();

		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString("yyyy-MM-dd") << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString("HH:mm:ss") << reply->header(QNetworkRequest::ContentLengthHeader).toString() << request.url().fileName());

		item->setTextAlignment(0, Qt::AlignHCenter);
		item->setTextAlignment(1, Qt::AlignHCenter);
		item->setTextAlignment(2, Qt::AlignHCenter);
		item->setTextAlignment(3, Qt::AlignHCenter);

		treeWidget->addTopLevelItem(item);
	}

	progressBar->setFormat(QString("%1 | %2 / %3 | %p%").arg(QString::number(version).rightJustified(4, '0')).arg(version - spinBox_Start->value() + 1).arg(counter));
	progressBar->setValue((version - spinBox_Start->value() + 1)*100 / counter);
}

void searchDialog::on_comboBox_model_currentIndexChanged(int index)
{
	spinBox_Start->setValue(fw.versions[index]);
	spinBox_Stop->setValue(spinBox_Start->value() + 99);

	lineEdit->setText(fw.names[index]);
}

void searchDialog::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, __attribute__((unused)) int column)
{
	downloadDialog(this, comboBox_model->currentIndex(), comboBox_server->currentIndex(), item->text(3)).exec();
}

void searchDialog::reject()
{
	if(reply && reply->isRunning())
	{
		if(QMessageBox::question(this, APPNAME, tr("Really abort search?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			abort = true;
		}

		return;
	}

	accept();
}

void searchDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(buttonBox->standardButton(button) == QDialogButtonBox::Apply)
	{
		if(!lineEdit->text().contains("????"))
		{
			QMessageBox::warning(this, APPNAME, tr("Filename must contain \"????\"!"));

			return;
		}

		if(spinBox_Start->value() > spinBox_Stop->value())
		{
			int val = spinBox_Start->value();

			spinBox_Start->setValue(spinBox_Stop->value());
			spinBox_Stop->setValue(val);
		}

		counter = spinBox_Stop->value() - spinBox_Start->value() + 1;

		if(counter > 100)
		{
			if(QMessageBox::warning(this, APPNAME, tr("Scanning more than 100 versions takes a long time and is not recommended!\n\nContinue anyway?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
			{
				return;
			}
		}

		version = spinBox_Start->value();

		buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

		treeWidget->clear();
		progressBar->setValue(0);

		QNetworkAccessManager *netmgr = new QNetworkAccessManager(this);
		request = QNetworkRequest(QUrl(QString("http://%1/%2/%3").arg(comboBox_server->currentText()).arg(fw.dirs[comboBox_model->currentIndex()]).arg(lineEdit->text().replace("????", QString::number(version).rightJustified(4, '0')))));

		abort = false;

		do
		{
			reply = netmgr->get(request);

			connect(reply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));

			while(reply->isRunning())
			{
				QCoreApplication::processEvents();
			}

			version++;

			request.setUrl(QUrl(QString("http://%1/%2/%3").arg(comboBox_server->currentText()).arg(fw.dirs[comboBox_model->currentIndex()]).arg(lineEdit->text().replace("????", QString::number(version).rightJustified(4, '0')))));

		}while(!abort && version <= spinBox_Stop->value());

		buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);

		reply = nullptr;

		QMessageBox::information(this, APPNAME, (abort ? tr("Search aborted, %1 firmware versions found so far.") : tr("%1 firmware versions found.")).arg(treeWidget->topLevelItemCount()));
	}
	else if(buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
	{
		if(reply && reply->isRunning())
		{
			if(QMessageBox::question(this, APPNAME, tr("Really abort search?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
			{
				abort = true;
			}
		}
		else
		{
			close();
		}
	}
}
