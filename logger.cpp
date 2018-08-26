#include "logger.h"

loggerDialog::loggerDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

	connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));

	resize(qApp->desktop()->screen()->width(), 0);
	move(0, 0);

	QTimer::singleShot(1, this, SLOT(calcHeight()));
}

void loggerDialog::calcHeight()
{
	treeWidget->setMinimumHeight(10 * treeWidget->visualItemRect(treeWidget->topLevelItem(0)).height() + treeWidget->header()->height() + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2);
}

void loggerDialog::log(QString p1, QString p2, QString p3, QString p4, QString p5, QString p6)
{
	static qlonglong counter = 0;

	QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << QString::number(++counter) << QDateTime::currentDateTime().toString("dd.MM hh:mm:ss") << p1 << p2 << p3 << p4 << p5 << (p6.isEmpty() ? "" : p6.insert(4, '-').insert(9, '-').insert(18, '-').insert(27, '-').insert(36, '-')));

    item->setTextAlignment(0, Qt::AlignHCenter);
    item->setTextAlignment(1, Qt::AlignHCenter);
    item->setTextAlignment(2, Qt::AlignHCenter);
    item->setTextAlignment(3, Qt::AlignHCenter);
    item->setTextAlignment(4, Qt::AlignHCenter);
	item->setTextAlignment(5, Qt::AlignHCenter);

    treeWidget->addTopLevelItem(item);

    if(treeWidget->topLevelItemCount() == 1)
    {
		treeWidget->topLevelItem(treeWidget->topLevelItemCount() - 1)->setSelected(true);
    }

    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
    treeWidget->resizeColumnToContents(5);
    treeWidget->resizeColumnToContents(6);
	treeWidget->resizeColumnToContents(7);

    treeWidget->scrollToBottom();
}

void loggerDialog::on_customContextMenuRequested(QPoint pos)
{
	QMenu menu(this);
	QAction action1(QIcon(":/png/png/clipboard.png"), tr("Copy all to clipboard"), this);
	QAction action2(QIcon(":/png/png/clipboard.png"), tr("Copy line to clipboard"), this);
	QAction action3(QIcon(":/png/png/clipboard.png"), tr("Copy field to clipboard"), this);
	QAction *selected;

	menu.addAction(&action1);
	menu.addAction(&action2);
	menu.addAction(&action3);

	selected = menu.exec(treeWidget->viewport()->mapToGlobal(pos));

	if(selected)
	{
		QClipboard *clipboard = QGuiApplication::clipboard();
		QTreeWidgetItem *item = treeWidget->itemAt(pos);

		if(selected->text() == action1.text())
		{
			QString text;

			for(int i = 0; i < treeWidget->topLevelItemCount(); i++)
			{
				item = treeWidget->topLevelItem(i);

				text.append(QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\n").arg(item->text(0)).arg(item->text(1)).arg(item->text(2)).arg(item->text(3)).arg(item->text(4)).arg(item->text(5)).arg(item->text(6)).arg(item->text(7)));
			}

			clipboard->setText(text);
		}
		else if(selected->text() == action2.text())
		{
			clipboard->setText(QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\n").arg(item->text(0)).arg(item->text(1)).arg(item->text(2)).arg(item->text(3)).arg(item->text(4)).arg(item->text(5)).arg(item->text(6)).arg(item->text(7)));
		}
		else if(selected->text() == action3.text())
		{
			item = treeWidget->itemAt(pos);

			if(item)
			{
				int colums[7];

				colums[0] = treeWidget->header()->sectionSize(0);
				colums[1] = colums[0] + treeWidget->header()->sectionSize(1);
				colums[2] = colums[1] + treeWidget->header()->sectionSize(2);
				colums[3] = colums[2] + treeWidget->header()->sectionSize(3);
				colums[4] = colums[3] + treeWidget->header()->sectionSize(4);
				colums[5] = colums[4] + treeWidget->header()->sectionSize(5);
				colums[6] = colums[5] + treeWidget->header()->sectionSize(6);

				if(pos.x() > colums[6])
				{
					clipboard->setText(item->text(7));
				}
				else if(pos.x() > colums[5])
				{
					clipboard->setText(item->text(6));
				}
				else if(pos.x() > colums[4])
				{
					clipboard->setText(item->text(5));
				}
				else if(pos.x() > colums[3])
				{
					clipboard->setText(item->text(4));
				}
				else if(pos.x() > colums[2])
				{
					clipboard->setText(item->text(3));
				}
				else if(pos.x() > colums[1])
				{
					clipboard->setText(item->text(2));
				}
				else if(pos.x() > colums[0])
				{
					clipboard->setText(item->text(1));
				}
				else
				{
					clipboard->setText(item->text(0));
				}
			}
		}
	}
}

void loggerDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(buttonBox->standardButton(button) == QDialogButtonBox::Save)
    {
		QFile logfile(QFileDialog::getSaveFileName(this, tr("Select file to save log"), LOG, "*.*", 0, QFileDialog::DontUseNativeDialog));

		if(!logfile.fileName().isEmpty())
		{
			if(logfile.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				for(int i = 0; i < treeWidget->topLevelItemCount(); i++)
				{
				   QTreeWidgetItem *item = treeWidget->topLevelItem(i);

					logfile.write(QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\n").arg(item->text(0)).arg(item->text(1)).arg(item->text(2)).arg(item->text(3)).arg(item->text(4)).arg(item->text(5)).arg(item->text(6)).arg(item->text(7)).toUtf8());
				}

				logfile.close();
			}
			else
			{
				QMessageBox::warning(this, APPNAME, tr("Could not open logfile!\n\n%1 : %2").arg(logfile.fileName()).arg(logfile.errorString()));
			}
		}
    }
    else if(buttonBox->standardButton(button) == QDialogButtonBox::Close)
    {
        close();
    }
}
