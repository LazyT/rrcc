#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow win;

	win.show();

	return app.exec();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QApplication::setStyle("Fusion");
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

	if(!QLocale::system().name().startsWith("en_"))
	{
		if(appTranslator.load("rrcc_" + QLocale::system().name(), QApplication::applicationDirPath() + "/lng"))
		{
			QApplication::installTranslator(&appTranslator);

			if(baseTranslator.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
			{
				QApplication::installTranslator(&baseTranslator);
			}
			else if(baseTranslator.load("qtbase_" + QLocale::system().name(), QApplication::applicationDirPath() + "/lng"))
			{
				QApplication::installTranslator(&baseTranslator);
			}

			if(helpTranslator.load("qt_help_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
			{
				QApplication::installTranslator(&helpTranslator);
			}
			else if(helpTranslator.load("qt_help_" + QLocale::system().name(), QApplication::applicationDirPath() + "/lng"))
			{
				QApplication::installTranslator(&helpTranslator);
			}
		}
	}

	setupUi(this);

	setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	layout()->setSizeConstraint(QLayout::SetFixedSize);

	getConfig();

	scene = new QGraphicsScene(0, 0, 4095, 4095, this);
	graphicsView->setScene(scene);

	rubberBand = new QRubberBand(QRubberBand::Rectangle, graphicsView);

	menu_map = new QMenu(this);
	menu_map_zones = new QMenu(tr("Zone Cleaning"), this);
	menu_map_rotation = new QMenu(tr("Rotation"), this);
	menu_map_flipping = new QMenu(tr("Flipping"), this);
	menu_map_swapping = new QMenu(tr("Swapping"), this);
	menu_map_zones->setIcon(QIcon(":/png/png/zone.png"));
	menu_map_rotation->setIcon(QIcon(":/png/png/rotate.png"));
	menu_map_flipping->setIcon(QIcon(":/png/png/flip.png"));
	menu_map_swapping->setIcon(QIcon(":/png/png/swap.png"));

	connect(menu_map_zones, SIGNAL(hovered(QAction*)), this, SLOT(hovered(QAction*)));
	connect(menu_map_zones, SIGNAL(aboutToHide()), this, SLOT(aboutToHide()));

	foreach(CLEANZONE zone, cfg.zones)
	{
		QAction *action = new QAction(QIcon(":/png/png/zone.png"), zone.label, this);

		action->setStatusTip(tr("Clean Zone %1").arg(zone.label));

		menu_map_zones->addAction(action);
	}

	menu_map_rotation->addAction(actionMenu_Map_Rotate0);
	menu_map_rotation->addAction(actionMenu_Map_Rotate90);
	menu_map_rotation->addAction(actionMenu_Map_Rotate180);
	menu_map_rotation->addAction(actionMenu_Map_Rotate270);
	menu_map_flipping->addAction(actionMenu_Map_FlipH);
	menu_map_flipping->addAction(actionMenu_Map_FlipV);
	menu_map_swapping->addAction(actionMenu_Map_SwapX);
	menu_map_swapping->addAction(actionMenu_Map_SwapY);

	menu_map->addAction(actionMenu_Map_Reset);
	menu_map->addSeparator();
	menu_map->addAction(actionMenu_Map_Goto);
	menu_map->addSeparator();
	menu_map->addMenu(menu_map_zones);
	menu_map->addSeparator();
	menu_map->addMenu(menu_map_rotation);
	menu_map->addSeparator();
	menu_map->addMenu(menu_map_flipping);
	menu_map->addSeparator();
	menu_map->addMenu(menu_map_swapping);

	group_map = new QActionGroup(this);
	group_map->addAction(actionMenu_Map_Rotate0);
	group_map->addAction(actionMenu_Map_Rotate90);
	group_map->addAction(actionMenu_Map_Rotate180);
	group_map->addAction(actionMenu_Map_Rotate270);

	actionMenu_Map_FlipH->setChecked(cfg.flip_h);
	actionMenu_Map_FlipV->setChecked(cfg.flip_v);
	actionMenu_Map_SwapX->setChecked(cfg.swap_x);
	actionMenu_Map_SwapY->setChecked(cfg.swap_y);

	if(cfg.rotate == 270)
	{
		actionMenu_Map_Rotate270->setChecked(true);
	}
	else if(cfg.rotate == 180)
	{
		actionMenu_Map_Rotate180->setChecked(true);
	}
	else if(cfg.rotate == 90)
	{
		actionMenu_Map_Rotate90->setChecked(true);
	}
	else
	{
		actionMenu_Map_Rotate0->setChecked(true);
	}

	logger = new loggerDialog(this);

	connect(&timerMap, SIGNAL(timeout()), this, SLOT(timer_refreshMap()));
	connect(&timerFanspeed, SIGNAL(timeout()), this, SLOT(timer_setFanspeed()));

	if(cfg.token == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" && cfg.ip == "255.255.255.255")
	{
		QMessageBox::information(NULL, APPNAME, tr("Please configure your device:\n\nIf your device is not provisioned yet connect to the wifi hotspot (\"roborock-vacuum...\") and then click OK.\n\nIf you have your token already simply click OK now."));
	}

	findIP();

	if(sendUDP(NULL))
	{
		if(provisioning || cfg.token == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
		{
			setupDialog(this).exec();

			findIP();

			getStatus();
		}
		else
		{
			getStatus();
		}
	}

	netmgr = new QNetworkAccessManager(this);
	connect(netmgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpFinished(QNetworkReply*)));

	websocket = new QWebSocket();
	connect(websocket, SIGNAL(textMessageReceived(QString)), this, SLOT(websocketTextMessageReceived(QString)));
	connect(websocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(websocketError(QAbstractSocket::SocketError)));

	if(cfg.map)
	{
		actionMap->setChecked(true);
	}
	else
	{
		groupBox_map->hide();
	}
}

void MainWindow::getConfig()
{
	QSettings ini(QFile::exists(CFG_P) ? CFG_P : CFG_H, QSettings::IniFormat);
	int count;
	CLEANZONE zone;

	ini.setIniCodec("UTF-8");

	cfg.ip = ini.value("IP-Address", "255.255.255.255").toString();
	cfg.token = ini.value("AES-Token", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF").toString();
	cfg.msgid = ini.value("Message-ID", 1).toLongLong();
	cfg.map = ini.value("Map", false).toBool();
	cfg.websocket = ini.value("WebSocket", false).toBool();

	ini.beginGroup("SSH");
	cfg.ssh_user = ini.value("User", "root").toString();
	cfg.ssh_pass = ini.value("Pass", "").toString();
	cfg.ssh_pkey = ini.value("PKey", "").toString();
	cfg.ssh_pkpp = ini.value("PKPP", "").toString();
	cfg.ssh_auth = ini.value("Auth", "PKey").toString();
	ini.endGroup();

	ini.beginGroup("MAP");
	cfg.flip_h = ini.value("Flip-H", false).toBool();
	cfg.flip_v = ini.value("Flip-V", false).toBool();
	cfg.rotate = ini.value("Rotate", 0).toInt();
	cfg.swap_x = ini.value("Swap-X", false).toBool();
	cfg.swap_y = ini.value("Swap-Y", false).toBool();
	ini.endGroup();

	ini.beginGroup("ZONES");
	count = ini.beginReadArray("Data");
	for(int i = 0; i < count; ++i)
	{
		ini.setArrayIndex(i);
		zone.label = ini.value("Label").toString();
		zone.x1 = ini.value("PosX1").toInt();
		zone.y1 = ini.value("PosY1").toInt();
		zone.x2 = ini.value("PosX2").toInt();
		zone.y2 = ini.value("PosY2").toInt();
		zone.times = ini.value("Times").toInt();
		cfg.zones.append(zone);
	}
	ini.endArray();
	ini.endGroup();
}

void MainWindow::setConfig()
{
	QSettings ini(QFile::exists(CFG_P) ? CFG_P : CFG_H, QSettings::IniFormat);

	ini.setIniCodec("UTF-8");

	ini.setValue("IP-Address", cfg.ip);
	ini.setValue("AES-Token", cfg.token);
	ini.setValue("Message-ID", cfg.msgid);
	ini.setValue("Map", cfg.map);
	ini.setValue("WebSocket", cfg.websocket);

	ini.beginGroup("SSH");
	ini.setValue("User", cfg.ssh_user);
	ini.setValue("Pass", cfg.ssh_pass);
	ini.setValue("PKey", cfg.ssh_pkey);
	ini.setValue("PKPP", cfg.ssh_pkpp);
	ini.setValue("Auth", cfg.ssh_auth);
	ini.endGroup();

	ini.beginGroup("MAP");
	ini.setValue("Flip-H", cfg.flip_h);
	ini.setValue("Flip-V", cfg.flip_v);
	ini.setValue("Rotate", cfg.rotate);
	ini.setValue("Swap-X", cfg.swap_x);
	ini.setValue("Swap-Y", cfg.swap_y);
	ini.endGroup();

	ini.remove("ZONES");
	if(cfg.zones.count())
	{
		ini.beginGroup("ZONES");
		ini.beginWriteArray("Data");
		for(int i = 0; i < cfg.zones.count(); ++i)
		{
			ini.setArrayIndex(i);
			ini.setValue("Label", cfg.zones.at(i).label);
			ini.setValue("PosX1", cfg.zones.at(i).x1);
			ini.setValue("PosY1", cfg.zones.at(i).y1);
			ini.setValue("PosX2", cfg.zones.at(i).x2);
			ini.setValue("PosY2", cfg.zones.at(i).y2);
			ini.setValue("Times", cfg.zones.at(i).times);
		}
		ini.endArray();
		ini.endGroup();
	}

	ini.sync();
}

void MainWindow::findIP()
{
	QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();

	foreach(QNetworkInterface nwif, allInterfaces)
	{
		QList<QNetworkAddressEntry> addressEntries = nwif.addressEntries();

		foreach(QNetworkAddressEntry entry, addressEntries)
		{
			if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback() && !entry.ip().isLinkLocal())
			{
				if(entry.ip().isInSubnet(QHostAddress(cfg.ip), QHostAddress::parseSubnet(QString("%1/%2").arg(entry.ip().toString()).arg(entry.netmask().toString())).second))
				{
					src_ip = entry.ip().toString();

					break;
				}
				else if(cfg.ip == "255.255.255.255" && nwif.type() == QNetworkInterface::Wifi)
				{
					src_ip = entry.ip().toString();

					break;
				}
			}
		}
	}
}

void MainWindow::getStatus()
{
	if(sendUDP(MIIO_GET_SERIAL_NUMBER))
	{
		label_Serial->setText(robo.serial_number);
	}
	else
	{
		return;
	}

	if(sendUDP(MIIO_GET_CONSUMABLE))
	{
		progressBar_Mainbrush->setValue(100 - (100 * robo.consumable.main_brush_work_time) / WORKTIME_MAINBRUSH);
		progressBar_Mainbrush->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_MAINBRUSH - robo.consumable.main_brush_work_time) / 3600.0, 'f', 2)));

		progressBar_Sidebrush->setValue(100 - (100 * robo.consumable.side_brush_work_time) / WORKTIME_SIDEBRUSH);
		progressBar_Sidebrush->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_SIDEBRUSH - robo.consumable.side_brush_work_time) / 3600.0, 'f', 2)));

		progressBar_Filter->setValue(100 - (100 * robo.consumable.filter_work_time) / WORKTIME_FILTER);
		progressBar_Filter->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_FILTER - robo.consumable.filter_work_time) / 3600.0, 'f', 2)));

		progressBar_Sensors->setValue(100 - (100 * robo.consumable.sensor_dirty_time) / WORKTIME_SENSOR);
		progressBar_Sensors->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_SENSOR - robo.consumable.sensor_dirty_time) / 3600.0, 'f', 2)));
	}
	else
	{
		return;
	}

	if(sendUDP(MIIO_GET_STATUS))
	{
		progressBar_Battery->setValue(robo.status.battery);
		progressBar_Battery->setFormat(QString("%p% [%1]").arg(QCoreApplication::translate("MainWindow", state_strings.at(robo.status.state - 1).toUtf8())));

		dial->blockSignals(true);
		dial->setValue(robo.status.fan_power);
		dial->blockSignals(false);

		label_Fanspeed->setText(QString::number(robo.status.fan_power));
	}
	else
	{
		return;
	}

	if(sendUDP(MIIO_GET_CLEAN_SUMMARY))
	{
		lcdNumber_Area->display(QString::number(robo.cleansummary.area / 1000000.0, 'f', 2));
		lcdNumber_Hours->display(QString::number(robo.cleansummary.time / 3600.0, 'f', 2));
		lcdNumber_Count->display(robo.cleansummary.count);
	}
	else
	{
		return;
	}

	if(!sendUDP(MIIO_GET_DND_TIMER))
	{
		return;
	}

	if(!sendUDP(MIIO_GET_SOUND_VOLUME))
	{
		return;
	}

	if(!sendUDP(MIIO_GET_CARPET_MODE))
	{
		return;
	}
}

QByteArray MainWindow::AESPayload(bool mode, QByteArray data)
{
	QByteArray payload;
	QByteArray key = QCryptographicHash::hash(QByteArray::fromHex(cfg.token.toUtf8()), QCryptographicHash::Md5);
	QByteArray iv = QCryptographicHash::hash(key + QByteArray::fromHex(cfg.token.toUtf8()), QCryptographicHash::Md5);

	if(mode == AES_ENCRYPT)
	{
		payload = QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::CBC, data.append(1, 0x00), key, iv, QAESEncryption::PKCS7);
	}
	else
	{
		payload = QAESEncryption::RemovePadding(QAESEncryption::Decrypt(QAESEncryption::AES_128, QAESEncryption::CBC, data, key, iv, QAESEncryption::PKCS7), QAESEncryption::PKCS7);

		payload.remove(payload.length() - 1, 1);
	}

	return payload;
}

bool MainWindow::sendUDP(QString data)
{
	QUdpSocket *socket = new QUdpSocket(this);
	QHostAddress address;
	QByteArray send;
	QByteArray recv;
	QByteArray payload;
	quint64 bytes;
	QTime timer;
	int time;
	bool retry = false;

	if(did.isEmpty() && !data.isEmpty())
	{
		if(!sendUDP(NULL))
		{
			return false;
		}
	}

retry_once:

	send.clear();

	if(data.isEmpty())
	{
		send = QByteArray::fromHex(MIIO_HELLO);
	}
	else
	{
		if(cfg.token == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
		{
			QMessageBox::warning(this, APPNAME, tr("Token not set, please configure your device token first!"));

			return false;
		}
		else
		{
			send.append(QByteArray::fromHex("2131000000000000"));
			send.append(did);
			send.append(QByteArray::fromHex(QString("%1").arg(QDateTime::currentDateTime().toTime_t() - timediff, 8, 16, QChar('0')).toUtf8()));
			send.append(QByteArray::fromHex(cfg.token.toUtf8()));
			send.append(AESPayload(AES_ENCRYPT, data.arg(cfg.msgid++).toUtf8()));
			send.replace(2, 2, QByteArray::fromHex(QString("%1").arg(send.length(), 4, 16, QChar('0')).toUtf8()));
			send.replace(16, 16, QCryptographicHash::hash(send, QCryptographicHash::Md5));
		}
	}

	if(!retry)
	{
		socket->bind(QHostAddress(src_ip));
	}

	bytes = socket->writeDatagram(send, QHostAddress(cfg.ip), 54321);

	((loggerDialog*)logger)->log(src_ip, cfg.ip, QString::number(bytes), QString("1 ms"), data.isEmpty() ? "{'HELLO?'}" : data.arg(cfg.msgid - 1), QString(send.toHex().toUpper()));

	timer.start();

	while(!socket->hasPendingDatagrams())
	{
		if(timer.elapsed() >= TIMEOUT)
		{
			((loggerDialog*)logger)->log(cfg.ip, src_ip, QString("0"), QString("%1 ms").arg(timer.elapsed()), retry ? QString("{'TIMEOUT!'}") : QString("{'TIMEOUT, RETRY...'}"), QString());

			if(!retry)
			{
				QApplication::processEvents();

				retry = true;

				cfg.msgid += 10;

				goto retry_once;
			}

			if(!data.contains("get_ota_state"))
			{
				QMessageBox::critical(isHidden() ? NULL : this, APPNAME, tr("Network connection timed out!\n\nPlease check ip / token / msgid..."));
			}

			return false;
		}

		QThread::msleep(1);
	}

	recv.resize(socket->pendingDatagramSize());

	bytes = socket->readDatagram(recv.data(), recv.size(), &address, NULL);

	time = timer.elapsed();

	if(recv.mid(2, 2).toHex() == "0020")
	{
		did = recv.mid(8, 4);
		cnt = recv.mid(12, 4);

		timediff = QDateTime::currentDateTime().toTime_t() - cnt.toHex().toUInt(NULL, 16);

		if(recv.mid(16).toHex().toUpper() != "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
		{
			provisioning = true;

			cfg.token = recv.mid(16).toHex();
		}

		if(cfg.ip == "255.255.255.255")
		{
			cfg.ip = QHostAddress(address.toIPv4Address()).toString();
		}

		((loggerDialog*)logger)->log(QHostAddress(address.toIPv4Address()).toString(), src_ip, QString::number(bytes), QString("%1 ms").arg(time), QString("{device = %1, stamp = %2, token = %3}").arg(did.toHex(':').toUpper().data()).arg(cnt.toHex(':').toUpper().data()).arg(recv.mid(16).toHex(':').toUpper().data()), QString(recv.toHex().toUpper()));
	}
	else
	{
		payload = AESPayload(AES_DECRYPT, recv.mid(32));

		((loggerDialog*)logger)->log(QHostAddress(address.toIPv4Address()).toString(), src_ip, QString::number(bytes), QString("%1 ms").arg(time), payload, QString(recv.toHex().toUpper()));

		if(data.contains("find_me"))
		{
			parseJSON(MIIO_ID_FIND_ME, payload);
		}
		else if(data.contains("app_start"))
		{
			parseJSON(MIIO_ID_APP_START, payload);
		}
		else if(data.contains("app_stop"))
		{
			parseJSON(MIIO_ID_APP_STOP, payload);
		}
		else if(data.contains("app_pause"))
		{
			parseJSON(MIIO_ID_APP_PAUSE, payload);
		}
		else if(data.contains("app_charge"))
		{
			parseJSON(MIIO_ID_APP_CHARGE, payload);
		}
		else if(data.contains("app_spot"))
		{
			parseJSON(MIIO_ID_APP_SPOT, payload);
		}
		else if(data.contains("app_zoned_clean"))
		{
			parseJSON(MIIO_ID_APP_ZONED_CLEAN, payload);
		}
		else if(data.contains("get_serial_number"))
		{
			parseJSON(MIIO_ID_GET_SERIAL_NUMBER, payload);
		}
		else if(data.contains("get_consumable"))
		{
			parseJSON(MIIO_ID_GET_CONSUMABLE, payload);
		}
		else if(data.contains("reset_consumable"))
		{
			parseJSON(MIIO_ID_RESET_CONSUMABLE, payload);
		}
		else if(data.contains("get_status"))
		{
			parseJSON(MIIO_ID_GET_STATUS, payload);
		}
		else if(data.contains("get_clean_summary"))
		{
			parseJSON(MIIO_ID_GET_CLEAN_SUMMARY, payload);
		}
		else if(data.contains("get_clean_record"))
		{
			parseJSON(MIIO_ID_GET_CLEAN_RECORD, payload);
		}
		else if(data.contains("set_custom_mode"))
		{
			parseJSON(MIIO_ID_SET_CUTOM_MODE, payload);
		}
		else if(data.contains("get_timer"))
		{
			parseJSON(MIIO_ID_GET_TIMER, payload);
		}
		else if(data.contains("set_timer"))
		{
			parseJSON(MIIO_ID_SET_TIMER, payload);
		}
		else if(data.contains("upd_timer"))
		{
			parseJSON(MIIO_ID_UPD_TIMER, payload);
		}
		else if(data.contains("del_timer"))
		{
			parseJSON(MIIO_ID_DEL_TIMER, payload);
		}
		else if(data.contains("get_dnd_timer"))
		{
			parseJSON(MIIO_ID_GET_DND_TIMER, payload);
		}
		else if(data.contains("set_dnd_timer"))
		{
			parseJSON(MIIO_ID_SET_DND_TIMER, payload);
		}
		else if(data.contains("close_dnd_timer"))
		{
			parseJSON(MIIO_ID_CLOSE_DND_TIMER, payload);
		}
		else if(data.contains("get_sound_volume"))
		{
			parseJSON(MIIO_ID_GET_SOUND_VOLUME, payload);
		}
		else if(data.contains("change_sound_volume"))
		{
			parseJSON(MIIO_ID_CHANGE_SOUND_VOLUME, payload);
		}
		else if(data.contains("test_sound_volume"))
		{
			parseJSON(MIIO_ID_TEST_SOUND_VOLUME, payload);
		}
		else if(data.contains("get_current_sound"))
		{
			parseJSON(MIIO_ID_GET_CURRENT_SOUND, payload);
		}
		else if(data.contains("dnld_install_sound"))
		{
			parseJSON(MIIO_ID_DNLD_INSTALL_SOUND, payload);
		}
		else if(data.contains("get_sound_progress"))
		{
			parseJSON(MIIO_ID_GET_SOUND_PROGRESS, payload);
		}
		else if(data.contains("get_carpet_mode"))
		{
			parseJSON(MIIO_ID_GET_CARPET_MODE, payload);
		}
		else if(data.contains("set_carpet_mode"))
		{
			parseJSON(MIIO_ID_SET_CARPET_MODE, payload);
		}
		else if(data.contains("app_goto_target"))
		{
			parseJSON(MIIO_ID_APP_GOTO_TARGET, payload);
		}
		else if(data.contains("config_router"))
		{
			parseJSON(MIIO_ID_CONFIG_ROUTER, payload);
		}
		else if(data.contains(".ota"))
		{
			parseJSON(MIIO_ID_OTA, payload);
		}
		else if(data.contains("get_ota_state"))
		{
			parseJSON(MIIO_ID_GET_OTA_STATE, payload);
		}
		else if(data.contains("get_ota_progress"))
		{
			parseJSON(MIIO_ID_GET_OTA_PROGRESS, payload);
		}
	}

	return true;
}

void MainWindow::parseJSON(int mid, QByteArray data)
{
	QJsonDocument doc = QJsonDocument::fromJson(data);
	QJsonObject obj = doc.object();
	QJsonArray arr = obj.value("result").toArray();
	QJsonArray sub1, sub2, sub3;

	if(mid == MIIO_ID_FIND_ME)
	{
	}
	else if(mid == MIIO_ID_APP_START)
	{
	}
	else if(mid == MIIO_ID_APP_STOP)
	{
	}
	else if(mid == MIIO_ID_APP_PAUSE)
	{
	}
	else if(mid == MIIO_ID_APP_CHARGE)
	{
	}
	else if(mid == MIIO_ID_APP_SPOT)
	{
	}
	else if(mid == MIIO_ID_APP_ZONED_CLEAN)
	{
	}
	else if(mid == MIIO_ID_GET_SERIAL_NUMBER)
	{
		if(arr[0].toObject().contains("serial_number"))
		{
			robo.serial_number = arr[0].toObject().value("serial_number").toString();
		}
	}
	else if(mid == MIIO_ID_GET_CONSUMABLE)
	{
		if(arr[0].toObject().contains("main_brush_work_time"))
		{
			robo.consumable.main_brush_work_time = arr[0].toObject().value("main_brush_work_time").toInt();
		}
		if(arr[0].toObject().contains("side_brush_work_time"))
		{
			robo.consumable.side_brush_work_time = arr[0].toObject().value("side_brush_work_time").toInt();
		}
		if(arr[0].toObject().contains("filter_work_time"))
		{
			robo.consumable.filter_work_time = arr[0].toObject().value("filter_work_time").toInt();
		}
		if(arr[0].toObject().contains("sensor_dirty_time"))
		{
			robo.consumable.sensor_dirty_time = arr[0].toObject().value("sensor_dirty_time").toInt();
		}
	}
	else if(mid == MIIO_ID_RESET_CONSUMABLE)
	{
	}
	else if(mid == MIIO_ID_GET_STATUS)
	{
		if(arr[0].toObject().contains("msg_ver"))
		{
			robo.status.msg_ver = arr[0].toObject().value("msg_ver").toInt();
		}
		if(arr[0].toObject().contains("msg_seq"))
		{
			robo.status.msg_seq = arr[0].toObject().value("msg_seq").toInt();
		}
		if(arr[0].toObject().contains("state"))
		{
			robo.status.state = arr[0].toObject().value("state").toInt();
		}
		if(arr[0].toObject().contains("battery"))
		{
			robo.status.battery = arr[0].toObject().value("battery").toInt();
		}
		if(arr[0].toObject().contains("clean_time"))
		{
			robo.status.clean_time = arr[0].toObject().value("clean_time").toInt();
		}
		if(arr[0].toObject().contains("clean_area"))
		{
			robo.status.clean_area = arr[0].toObject().value("clean_area").toInt();
		}
		if(arr[0].toObject().contains("error_code"))
		{
			robo.status.error_code = arr[0].toObject().value("error_code").toInt();
		}
		if(arr[0].toObject().contains("map_present"))
		{
			robo.status.map_present = arr[0].toObject().value("map_present").toInt();
		}
		if(arr[0].toObject().contains("in_cleaning"))
		{
			robo.status.in_cleaning = arr[0].toObject().value("in_cleaning").toInt();
		}
		if(arr[0].toObject().contains("fan_power"))
		{
			robo.status.fan_power = arr[0].toObject().value("fan_power").toInt();
		}
		if(arr[0].toObject().contains("dnd_enabled"))
		{
			robo.status.dnd_enabled = arr[0].toObject().value("dnd_enabled").toInt();
		}
	}
	else if(mid == MIIO_ID_GET_CLEAN_SUMMARY)
	{
		sub1 = arr[3].toArray();

		robo.cleansummary.time = arr[0].toInt();
		robo.cleansummary.area = arr[1].toInt();
		robo.cleansummary.count = arr[2].toInt();

		robo.cleansummary.id.clear();

		foreach(QJsonValue val, sub1)
		{
			robo.cleansummary.id.append(val.toInt());
		}
	}
	else if(mid == MIIO_ID_GET_CLEAN_RECORD)
	{
		sub1 = arr[0].toArray();

		robo.cleanrecord.begin = sub1[0].toInt();
		robo.cleanrecord.finish = sub1[1].toInt();
		robo.cleanrecord.duration = sub1[2].toInt();
		robo.cleanrecord.area = sub1[3].toInt();
		robo.cleanrecord.error = sub1[4].toInt();
		robo.cleanrecord.complete = sub1[5].toInt();
	}
	else if(mid == MIIO_ID_SET_CUTOM_MODE)
	{
	}
	else if(mid == MIIO_ID_GET_TIMER)
	{
		int entries = arr.size();

		if(entries)
		{
			TIMER timer;

			robo.timers.clear();

			foreach(QJsonValue val, arr)
			{
				sub1 = val.toArray();
				sub2 = sub1[2].toArray();
				sub3 = sub2[1].toArray();

				timer.id = sub1[0].toString();
				timer.state = sub1[1].toString();
				timer.crontab = sub2[0].toString();
				timer.command = sub3[0].toString();
				timer.fanspeed = sub3[1].toInt();

				robo.timers.append(timer);
			}
		}
	}
	else if(mid == MIIO_ID_SET_TIMER)
	{
	}
	else if(mid == MIIO_ID_UPD_TIMER)
	{
	}
	else if(mid == MIIO_ID_DEL_TIMER)
	{
	}
	else if(mid == MIIO_ID_GET_DND_TIMER)
	{
		if(arr[0].toObject().contains("enabled"))
		{
			robo.dnd.enabled = arr[0].toObject().value("enabled").toInt();
		}
		if(arr[0].toObject().contains("start_hour"))
		{
			robo.dnd.start_hour = arr[0].toObject().value("start_hour").toInt();
		}
		if(arr[0].toObject().contains("start_minute"))
		{
			robo.dnd.start_minute = arr[0].toObject().value("start_minute").toInt();
		}
		if(arr[0].toObject().contains("end_hour"))
		{
			robo.dnd.end_hour = arr[0].toObject().value("end_hour").toInt();
		}
		if(arr[0].toObject().contains("end_minute"))
		{
			robo.dnd.end_minute = arr[0].toObject().value("end_minute").toInt();
		}
	}
	else if(mid == MIIO_ID_SET_DND_TIMER)
	{
	}
	else if(mid == MIIO_ID_CLOSE_DND_TIMER)
	{
	}
	else if(mid == MIIO_ID_GET_SOUND_VOLUME)
	{
		robo.volume = arr[0].toInt();
	}
	else if(mid == MIIO_ID_CHANGE_SOUND_VOLUME)
	{
	}
	else if(mid == MIIO_ID_TEST_SOUND_VOLUME)
	{
	}
	else if(mid == MIIO_ID_GET_CURRENT_SOUND)
	{
		robo.currentsound.bom = arr[0].toObject().value("bom").toString();
		robo.currentsound.language = arr[0].toObject().value("language").toString();
		robo.currentsound.location = arr[0].toObject().value("location").toString();
		robo.currentsound.msg_ver = arr[0].toObject().value("msg_ver").toInt();
		robo.currentsound.sid_in_progress = arr[0].toObject().value("sid_in_progress").toInt();
		robo.currentsound.sid_in_use = arr[0].toObject().value("sid_in_use").toInt();
		robo.currentsound.sid_version = arr[0].toObject().value("sid_version").toInt();
	}
	else if(mid == MIIO_ID_DNLD_INSTALL_SOUND)
	{
	}
	else if(mid == MIIO_ID_GET_SOUND_PROGRESS)
	{
		robo.soundprogress.error = arr[0].toObject().value("error").toInt();
		robo.soundprogress.progress = arr[0].toObject().value("progress").toInt();
		robo.soundprogress.sid_in_progress = arr[0].toObject().value("sid_in_progress").toInt();
		robo.soundprogress.state = arr[0].toObject().value("state").toInt();
	}
	else if(mid == MIIO_ID_GET_CARPET_MODE)
	{
		robo.carpetmode.enable = arr[0].toObject().value("enable").toInt();
		robo.carpetmode.current_integral = arr[0].toObject().value("current_integral").toInt();
		robo.carpetmode.current_high = arr[0].toObject().value("current_high").toInt();
		robo.carpetmode.current_low = arr[0].toObject().value("current_low").toInt();
		robo.carpetmode.stall_time = arr[0].toObject().value("stall_time").toInt();
	}
	else if(mid == MIIO_ID_SET_CARPET_MODE)
	{
	}
	else if(mid == MIIO_ID_APP_GOTO_TARGET)
	{
	}
	else if(mid == MIIO_ID_CONFIG_ROUTER)
	{
	}
	else if(mid == MIIO_ID_OTA)
	{
	}
	else if(mid == MIIO_ID_GET_OTA_STATE)
	{
		robo.ota.state = arr[0].toString();
	}
	else if(mid == MIIO_ID_GET_OTA_PROGRESS)
	{
		robo.ota.progress = arr[0].toInt();
	}
}

void MainWindow::ssh_connected()
{
	if(cfg.ssh_auth == "PKey")
	{
		QFile file_key(cfg.ssh_pkey);

		if(file_key.open(QIODevice::ReadOnly))
		{
			QByteArray key = file_key.readAll();

			file_key.close();

			ssh->loginKey(cfg.ssh_user, key, cfg.ssh_pkpp);
		}
	}
	else
	{
		ssh->login(cfg.ssh_user, cfg.ssh_pass);
	}
}

void MainWindow::ssh_loginSuccessful()
{
	ssh->executeCommand(ssh_cmd);
}

void MainWindow::ssh_commandExecuted(QString command, QString response)
{
	QByteArray version = response.toUtf8();

	ssh->disconnectFromHost();

	if(command == SSH_GET_FIRMWARE_VERSION)
	{
		if(version.contains("ROBOROCK_VERSION"))
		{
			QMessageBox::information(this, APPNAME, tr("Firmware %1 installed.").arg(QString(version.split('=').at(1)).replace("_", " Build ")));
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Firmware detection failed:\n\n%1").arg(response.isEmpty() ? tr("got empty response") : response));
		}
	}
	else if(command == SSH_GET_VALETUDO_VERSION)
	{
                if(version.contains("\"valetudo\""))
		{
			QMessageBox::information(this, APPNAME, tr("Valetudo %1 installed.").arg(QString(version.split(',').at(1).split('\"').at(3))));
		}
		else
		{
			QMessageBox::warning(this, APPNAME, tr("Valetudo detection failed:\n\n%1").arg(response.isEmpty() ? tr("got empty response") : response));
		}
	}
}

void MainWindow::ssh_disconnected()
{
	delete ssh;
}

void MainWindow::ssh_error(QSshSocket::SshError error)
{
	ssh->disconnectFromHost();

	QMessageBox::warning(this, APPNAME, tr("SSH connection error!\n\n%1").arg(ssh_error_strings.at(error)));
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::on_actionRefresh_triggered()
{
	getStatus();
}

void MainWindow::on_actionMap_toggled(bool checked)
{
	if(checked)
	{
		groupBox_map->show();

		setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
		layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

		if(cfg.websocket)
		{
			websocket->open(QUrl(QString("ws://%1").arg(cfg.ip)));
		}
		else
		{
			emit timer_refreshMap();
		}
	}
	else
	{
		showNormal();

		groupBox_map->hide();

		setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
		layout()->setSizeConstraint(QLayout::SetFixedSize);

		if(cfg.websocket)
		{
			websocket->close();
		}
		else
		{
			timerMap.stop();
		}
	}

	adjustSize();
	show();

	QCoreApplication::processEvents();

	move(qApp->desktop()->availableGeometry().center() - rect().center());
}

void MainWindow::on_actionSetup_triggered()
{
	setupDialog(this).exec();
}

void MainWindow::on_actionLogger_triggered()
{
	logger->isVisible() ? logger->hide() : logger->show();
}

void MainWindow::on_actionTimer_triggered()
{
	if(sendUDP(MIIO_GET_TIMER))
	{
		if(!robo.timers.size())
		{
			if(QMessageBox::question(this, APPNAME, tr("No timers found, create new?"), QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) ==  QMessageBox::No)
			{
				return;
			}
		}

		timerDialog(this).exec();
	}
}

void MainWindow::on_actionHistory_triggered()
{
	if(robo.cleansummary.id.size())
	{
		historyDialog(this).exec();
	}
	else
	{
		QMessageBox::information(this, APPNAME, tr("No cleaning history found."));
	}
}

void MainWindow::on_actionZones_triggered()
{
	if(cfg.zones.count())
	{
		zonesDialog(this).exec();
	}
	else
	{
		QMessageBox::information(this, APPNAME, tr("No cleaning zones defined yet."));
	}
}

void MainWindow::on_actionValetudoVersion_triggered()
{
	if(cfg.ssh_pass.isEmpty() && cfg.ssh_pkey.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please setup your ssh settings first!"));
	}
	else
	{
		ssh = new QSshSocket(this);

		connect(ssh, SIGNAL(connected()), this, SLOT(ssh_connected()));
		connect(ssh, SIGNAL(disconnected()), this, SLOT(ssh_disconnected()));
		connect(ssh, SIGNAL(error(QSshSocket::SshError)), this, SLOT(ssh_error(QSshSocket::SshError)));
		connect(ssh, SIGNAL(loginSuccessful()), this, SLOT(ssh_loginSuccessful()));
		connect(ssh, SIGNAL(commandExecuted(QString, QString)), this, SLOT(ssh_commandExecuted(QString, QString)));

		ssh_cmd = SSH_GET_VALETUDO_VERSION;

		ssh->connectToHost(cfg.ip);
	}

}

void MainWindow::on_actionValetudoInstall_triggered()
{
	if(cfg.ssh_pass.isEmpty() && cfg.ssh_pkey.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please setup your ssh settings first!"));
	}
	else
	{
		if(actionMap->isChecked())
		{
			actionMap->setChecked(false);
		}

		installerDialog(this).exec();
	}
}

void MainWindow::on_actionValetudoUninstall_triggered()
{
	if(cfg.ssh_pass.isEmpty() && cfg.ssh_pkey.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please setup your ssh settings first!"));
	}
	else
	{
		if(actionMap->isChecked())
		{
			actionMap->setChecked(false);
		}

		uninstallerDialog(this).exec();
	}
}

void MainWindow::on_actionCheckFirmware_triggered()
{
	if(cfg.ssh_pass.isEmpty() && cfg.ssh_pkey.isEmpty())
	{
		QMessageBox::warning(this, APPNAME, tr("Please setup your ssh settings first!"));
	}
	else
	{
		ssh = new QSshSocket(this);

		connect(ssh, SIGNAL(connected()), this, SLOT(ssh_connected()));
		connect(ssh, SIGNAL(disconnected()), this, SLOT(ssh_disconnected()));
		connect(ssh, SIGNAL(error(QSshSocket::SshError)), this, SLOT(ssh_error(QSshSocket::SshError)));
		connect(ssh, SIGNAL(loginSuccessful()), this, SLOT(ssh_loginSuccessful()));
		connect(ssh, SIGNAL(commandExecuted(QString, QString)), this, SLOT(ssh_commandExecuted(QString, QString)));

		ssh_cmd = SSH_GET_FIRMWARE_VERSION;

		ssh->connectToHost(cfg.ip);
	}
}

void MainWindow::on_actionSearchFirmware_triggered()
{
	searchDialog(this).exec();
}

void MainWindow::on_actionDownloadFirmware_triggered()
{
	downloadDialog(this, 0, 0, "").exec();
}

void MainWindow::on_actionUpdateFirmware_triggered()
{
	if(QMessageBox::question(this, APPNAME, tr("Are you really sure you want to install a firmware update?\n\nPlease choose the correct version for your model or you will brick your device!"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
		if(sendUDP(MIIO_GET_STATUS))
		{
			if(robo.status.battery < 20)
			{
				QMessageBox::warning(this, APPNAME, tr("Charge battery to at least 20% first!"));

				return;
			}
			else if(robo.status.state != 8)
			{
				QMessageBox::warning(this, APPNAME, tr("Send robot to docking station first!"));

				return;
			}
try_again:
			QFile file(QFileDialog::getOpenFileName(this, tr("Select firmware package to install"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "*.pkg", 0, QFileDialog::DontUseNativeDialog));

			if(!file.fileName().isEmpty())
			{
				if(file.open(QIODevice::ReadOnly))
				{
					if(file.size())
					{
						if(actionMap->isChecked())
						{
							actionMap->setChecked(false);
						}

						updateDialog(this, &file).exec();
					}
					else
					{
						QMessageBox::warning(this, APPNAME, tr("Selected firmware package is empty!"));

						goto try_again;
					}
				}
				else
				{
					QMessageBox::warning(this, APPNAME, tr("Could not open firmware package!\n\n%1").arg(file.errorString()));

					goto try_again;
				}
			}
		}
	}
}

void MainWindow::on_actionHelp_triggered()
{
	QMessageBox::information(this, APPNAME, tr("Help not implemented yet, sorry..."));
}

void MainWindow::on_actionAbout_triggered()
{
	aboutDialog(this).exec();
}

void MainWindow::on_toolButton_Dock_clicked()
{
	sendUDP(MIIO_APP_CHARGE);
}

void MainWindow::on_toolButton_Start_clicked()
{
	sendUDP(MIIO_APP_START);
}

void MainWindow::on_toolButton_Spot_clicked()
{
	sendUDP(MIIO_APP_SPOT);
}

void MainWindow::on_toolButton_Pause_clicked()
{
	sendUDP(MIIO_APP_PAUSE);
}

void MainWindow::on_toolButton_Stop_clicked()
{
	sendUDP(MIIO_APP_STOP);
}

void MainWindow::on_toolButton_Find_clicked()
{
	sendUDP(MIIO_FIND_ME);
}

void MainWindow::on_dial_valueChanged(int value)
{
	label_Fanspeed->setText(QString::number(value));

	timerFanspeed.setSingleShot(true);
	timerFanspeed.start(1000);
}

void MainWindow::timer_refreshMap()
{
	progressBar_MapRefresh->setValue(0);

	netmgr->get(QNetworkRequest(QUrl(QString("http://%1/api/map/latest").arg(cfg.ip))));
}

void MainWindow::timer_setFanspeed()
{
	sendUDP(QString(MIIO_SET_CUSTOM_MODE).arg(dial->value()).arg("%1"));
}

void MainWindow::on_toolButton_Fanspeed1_clicked()
{
	dial->setValue(FANSPEED_QUIET);
}

void MainWindow::on_toolButton_Fanspeed2_clicked()
{
	dial->setValue(FANSPEED_BALANCED);
}

void MainWindow::on_toolButton_Fanspeed3_clicked()
{
	dial->setValue(FANSPEED_TURBO);
}

void MainWindow::on_toolButton_Fanspeed4_clicked()
{
	dial->setValue(FANSPEED_MAXIMUM);
}

void MainWindow::on_toolButton_Mainbrush_clicked()
{
	if(sendUDP(QString(MIIO_RESET_CONSUMABLE).arg("main_brush_work_time").arg("%1")))
	{
		if(sendUDP(MIIO_GET_CONSUMABLE))
		{
			progressBar_Mainbrush->setValue(100 - (100 * robo.consumable.main_brush_work_time) / WORKTIME_MAINBRUSH);
			progressBar_Mainbrush->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_MAINBRUSH - robo.consumable.main_brush_work_time) / 3600.0, 'f', 2)));
		}
	}
}

void MainWindow::on_toolButton_Sidebrush_clicked()
{
	if(sendUDP(QString(MIIO_RESET_CONSUMABLE).arg("side_brush_work_time").arg("%1")))
	{
		if(sendUDP(MIIO_GET_CONSUMABLE))
		{
			progressBar_Sidebrush->setValue(100 - (100 * robo.consumable.side_brush_work_time) / WORKTIME_SIDEBRUSH);
			progressBar_Sidebrush->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_SIDEBRUSH - robo.consumable.side_brush_work_time) / 3600.0, 'f', 2)));
		}
	}
}

void MainWindow::on_toolButton_Filter_clicked()
{
	if(sendUDP(QString(MIIO_RESET_CONSUMABLE).arg("filter_work_time").arg("%1")))
	{
		if(sendUDP(MIIO_GET_CONSUMABLE))
		{
			progressBar_Filter->setValue(100 - (100 * robo.consumable.filter_work_time) / WORKTIME_FILTER);
			progressBar_Filter->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_FILTER - robo.consumable.filter_work_time) / 3600.0, 'f', 2)));
		}
	}
}

void MainWindow::on_toolButton_Sensors_clicked()
{
	if(sendUDP(QString(MIIO_RESET_CONSUMABLE).arg("sensor_dirty_time").arg("%1")))
	{
		if(sendUDP(MIIO_GET_CONSUMABLE))
		{
			progressBar_Sensors->setValue(100 - (100 * robo.consumable.sensor_dirty_time) / WORKTIME_SENSOR);
			progressBar_Sensors->setFormat(QString("%p% [%1h]").arg(QString::number((WORKTIME_SENSOR - robo.consumable.sensor_dirty_time) / 3600.0, 'f', 2)));
		}
	}
}

void MainWindow::drawMapFromJson(QByteArray map)
{
	QJsonDocument doc = QJsonDocument::fromJson(map);
	QJsonObject obj = doc.object();
	QJsonObject image = obj.value("image").toObject();
	int image_position_top = image.value("position").toObject().value("top").toInt();
	int image_position_left = image.value("position").toObject().value("left").toInt();
	QJsonArray image_pixels_floor = image.value("pixels").toObject().value("floor").toArray();
	QJsonArray image_pixels_obstacle_weak = image.value("pixels").toObject().value("obstacle_weak").toArray();
	QJsonArray image_pixels_obstacle_strong = image.value("pixels").toObject().value("obstacle_strong").toArray();
	QJsonObject path = obj.value("path").toObject();
	QJsonArray path_points = path.value("points").toArray();
	QJsonArray val;
	QGraphicsPixmapItem *png_robo, *png_dock;
	int x1, y1, x2, y2;

	zone_preview_item = nullptr;

	if(png_flag)
	{
		pos_flag = png_flag->pos();
	}

	if(png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb)
	{
		pos_flag_lt = png_flag_lt->pos();
		pos_flag_rt = png_flag_rt->pos();
		pos_flag_lb = png_flag_lb->pos();
		pos_flag_rb = png_flag_rb->pos();
	}

	scene->clear();

	if(png_flag)
	{
		drawFlags(true, true);
	}

	if(png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb)
	{
		drawFlags(true, false);
	}

	png_dock = scene->addPixmap(QPixmap(":/png/png/dock.png"));
	png_robo = scene->addPixmap(QPixmap(":/png/png/robo.png"));
	png_dock->setZValue(1);
	png_robo->setZValue(2);
	png_dock->setOffset(-16, -16);
	png_robo->setOffset(-16, -16);

	foreach(QJsonValue val, image_pixels_floor)
	{
		val = val.toArray();

		x1 = (val[0].toInt() + image_position_left) * 4;
		y1 = (val[1].toInt() + image_position_top) * 4;

		scene->addRect(x1, y1, 1, 1, QPen(QBrush(QColor(0, 128, 255, 255)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
	}

	foreach(QJsonValue val, image_pixels_obstacle_weak)
	{
		val = val.toArray();

		x1 = (val[0].toInt() + image_position_left) * 4;
		y1 = (val[1].toInt() + image_position_top) * 4;

		scene->addRect(x1, y1, 1, 1, QPen(QBrush(QColor(255, 0, 0, 255)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
	}

	foreach(QJsonValue val, image_pixels_obstacle_strong)
	{
		val = val.toArray();

		x1 = (val[0].toInt() + image_position_left) * 4;
		y1 = (val[1].toInt() + image_position_top) * 4;

		scene->addRect(x1, y1, 1, 1, QPen(QBrush(QColor(96, 160, 255, 255)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
	}

	x1 = path_points[0].toArray().first().toInt() / MAPFACTOR;
	y1 = path_points[1].toArray().first().toInt() / MAPFACTOR;

	foreach(QJsonValue val, path_points)
	{
		val = val.toArray();

		x2 = val[0].toInt() / MAPFACTOR;
		y2 = val[1].toInt() / MAPFACTOR;

		if(!(x1 == x2 && y1 == y2))
		{
			scene->addLine(x1, y1, x2, y2, QPen(QBrush(Qt::white), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		}

		x1 = x2;
		y1 = y2;
	}

	png_dock->setPos(obj.value("charger").toArray().first().toInt() / MAPFACTOR, obj.value("charger").toArray().last().toInt() / MAPFACTOR);

	png_robo->setPos(obj.value("robot").toArray().first().toInt() / MAPFACTOR, obj.value("robot").toArray().last().toInt() / MAPFACTOR);
	png_robo->setRotation(path.value("current_angle").toDouble());

	if(png_flag && png_flag->collidesWithItem(png_robo))
	{
		drawFlags(false, true);
	}

	if((png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb) && (png_flag_lt->collidesWithItem(png_robo) || png_flag_rt->collidesWithItem(png_robo) || png_flag_lb->collidesWithItem(png_robo) || png_flag_rb->collidesWithItem(png_robo)))
	{
		drawFlags(false, false);
	}

	if(zone_preview_rect.width() && zone_preview_rect.height())
	{
		zone_preview_item = scene->addRect(zone_preview_rect, QPen(Qt::red), QBrush(QColor(255, 0, 0, 64)));
	}

	graphicsView->setSceneRect(scene->itemsBoundingRect());

	if(graphicsView->matrix().m11() == 1.0)
	{
		setMatrix();
	}
}

void MainWindow::drawMapFromJsonOld(QByteArray map)
{
	QJsonDocument doc = QJsonDocument::fromJson(map);
	QJsonObject obj = doc.object();
	QJsonArray arr_path = obj.value("path").toArray();
	QJsonArray arr_map = obj.value("map").toArray();
	QJsonArray sub;
	QGraphicsPixmapItem *png_robo, *png_dock;
	QColor col;
	int x1, y1, x2, y2;
	int angle = 0;

	zone_preview_item = nullptr;

	if(png_flag)
	{
		pos_flag = png_flag->pos();
	}

	if(png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb)
	{
		pos_flag_lt = png_flag_lt->pos();
		pos_flag_rt = png_flag_rt->pos();
		pos_flag_lb = png_flag_lb->pos();
		pos_flag_rb = png_flag_rb->pos();
	}

	scene->clear();

	if(png_flag)
	{
		drawFlags(true, true);
	}

	if(png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb)
	{
		drawFlags(true, false);
	}

	png_dock = scene->addPixmap(QPixmap(":/png/png/dock.png"));
	png_robo = scene->addPixmap(QPixmap(":/png/png/robo.png"));
	png_dock->setZValue(1);
	png_robo->setZValue(2);
	png_dock->setOffset(-16, -16);
	png_robo->setOffset(-16, -16);

	foreach(QJsonValue val, arr_map)
	{
		sub = val.toArray();

		int pos = sub[0].toInt() / 4;
		int x1 = (pos % 1024) * 4;
		int y1 = (pos / 1024) * 4;

		col = QColor(sub[1].toInt(), sub[2].toInt(), sub[3].toInt(), 255);

		if(col.name() == "#000000")
		{
			col.setRgb(96, 160, 255, 255);
		}
		else if(col.name() == "#ffffff")
		{
			col.setRgb(0, 128, 255, 255);
		}

		scene->addRect(x1, y1, 1, 1, QPen(QBrush(col), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
	}

	sub = arr_path[0].toArray();

	x1 = sub[0].toInt();
	y1 = sub[1].toInt();

	png_dock->setPos(x1, y1);

	foreach(QJsonValue val, arr_path)
	{
		sub = val.toArray();

		x2 = sub[0].toInt();
		y2 = sub[1].toInt();

		if(!(x1 == x2 && y1 == y2))
		{
			scene->addLine(x1, y1, x2, y2, QPen(QBrush(Qt::white), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

			if(qAbs(x2 - x1) > 2 || qAbs(y2 - y1) > 2)
			{
				angle = (int)(qRadiansToDegrees(qAtan2(y1 - y2, x2 - x1)) + 360) % 360;
			}
		}

		x1 = x2;
		y1 = y2;
	}

	if(zone_preview_rect.width() && zone_preview_rect.height())
	{
		zone_preview_item = scene->addRect(zone_preview_rect, QPen(Qt::red), QBrush(QColor(255, 0, 0, 64)));
	}

	png_robo->setPos(x1, y1);
	png_robo->setRotation(360 - angle);

	if(png_flag && png_flag->collidesWithItem(png_robo))
	{
		drawFlags(false, true);
	}

	if((png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb) && (png_flag_lt->collidesWithItem(png_robo) || png_flag_rt->collidesWithItem(png_robo) || png_flag_lb->collidesWithItem(png_robo) || png_flag_rb->collidesWithItem(png_robo)))
	{
		drawFlags(false, false);
	}

	graphicsView->setSceneRect(scene->itemsBoundingRect());

	if(graphicsView->matrix().m11() == 1.0)
	{
		setMatrix();
	}
}

void MainWindow::httpFinished(QNetworkReply *reply)
{
	progressBar_MapRefresh->setValue(100);

	if(!reply->error())
	{
		QByteArray data = reply->readAll();

		if(data.contains("\"image\""))
		{
			drawMapFromJson(data);
		}
		else
		{
			drawMapFromJsonOld(data);
		}

		timerMap.setSingleShot(true);
		timerMap.start(3000);
	}
	else
	{
		actionMap->setChecked(false);

		QMessageBox::warning(this, APPNAME, tr("Map function is only available on rooted devices running Valetudo!\n\n%1").arg(reply->errorString()));
	}
}

void MainWindow::websocketTextMessageReceived(QString message)
{
	if(!message.isEmpty())
	{
		drawMapFromJson(message.toUtf8());
	}
	else
	{
		progressBar_MapRefresh->setValue(0);

		QThread::msleep(250);

		progressBar_MapRefresh->setValue(100);
	}
}

void MainWindow::websocketError(QAbstractSocket::SocketError error)
{
	QMessageBox::warning(this, APPNAME, tr("Websocket error %1, closing connection!\n\n%2").arg(error).arg(websocket->errorString()));

	actionMap->setChecked(false);
}

void MainWindow::hovered(QAction *action)
{
	int index = 0;

	foreach(QAction *current, menu_map_zones->actions())
	{
		if(current == action)
		{
			break;
		}

		index++;
	}

	zone_preview_rect = QRect((cfg.swap_x ? cfg.zones.at(index).x1 : MAPSIZE - cfg.zones.at(index).x2) / MAPFACTOR, (cfg.swap_y ? cfg.zones.at(index).y1 : MAPSIZE - cfg.zones.at(index).y2) / MAPFACTOR, qAbs(cfg.zones.at(index).x1 - cfg.zones.at(index).x2) / MAPFACTOR, qAbs(cfg.zones.at(index).y1 - cfg.zones.at(index).y2) / MAPFACTOR);

	if(zone_preview_item)
	{
		scene->removeItem(zone_preview_item);
	}

	zone_preview_item = scene->addRect(zone_preview_rect, QPen(Qt::red), QBrush(QColor(255, 0, 0, 64)));
}

void MainWindow::aboutToHide()
{
	zone_preview_rect = QRect();

	if(zone_preview_item)
	{
		scene->removeItem(zone_preview_item);

		zone_preview_item = NULL;
	}
}

void MainWindow::drawFlags(bool show, bool single)
{
	if(show)
	{
		if(single)
		{
			png_flag = scene->addPixmap(QPixmap(":/png/png/flag.png"));
			png_flag->setPos(pos_flag);
			png_flag->setZValue(3);
			png_flag->setOffset(-16, -16);
		}
		else
		{
			png_flag_lt = scene->addPixmap(QPixmap(":/png/png/flag.png"));
			png_flag_lt->setZValue(3);
			png_flag_lt->setOffset(-16, -16);
			png_flag_lt->setPos(pos_flag_lt);

			png_flag_rt = scene->addPixmap(QPixmap(":/png/png/flag.png"));
			png_flag_rt->setZValue(3);
			png_flag_rt->setOffset(-16, -16);
			png_flag_rt->setPos(pos_flag_rt);

			png_flag_lb = scene->addPixmap(QPixmap(":/png/png/flag.png"));
			png_flag_lb->setZValue(3);
			png_flag_lb->setOffset(-16, -16);
			png_flag_lb->setPos(pos_flag_lb);

			png_flag_rb = scene->addPixmap(QPixmap(":/png/png/flag.png"));
			png_flag_rb->setZValue(3);
			png_flag_rb->setOffset(-16, -16);
			png_flag_rb->setPos(pos_flag_rb);
		}
	}
	else
	{
		if(single)
		{
			scene->removeItem(png_flag);

			png_flag = nullptr;
		}
		else
		{
			scene->removeItem(png_flag_lt);
			scene->removeItem(png_flag_rt);
			scene->removeItem(png_flag_lb);
			scene->removeItem(png_flag_rb);

			png_flag_lt = nullptr;
			png_flag_rt = nullptr;
			png_flag_lb = nullptr;
			png_flag_rb = nullptr;
		}
	}
}

void MainWindow::getScale()
{
	QMatrix matrix = graphicsView->matrix();

	if(matrix.m11() != 0)
	{
		scale = qAbs(matrix.m11());
	}
	else if(matrix.m12() != 0)
	{
		scale = qAbs(matrix.m12());
	}
}

void MainWindow::setMatrix()
{
	qreal m11 = 0, m12 = 0, m21 = 0, m22 = 0;

	getScale();

	if(cfg.rotate == 0)
	{
		m11 = scale;
		m12 = 0;
		m21 = 0;
		m22 = scale;

		if(cfg.flip_h)
		{
			m11 *= -1;
		}

		if(cfg.flip_v)
		{
			m22 *= -1;
		}
	}
	else if(cfg.rotate == 90)
	{
		m11 = 0;
		m12 = scale;
		m21 = -scale;
		m22 = 0;

		if(cfg.flip_h)
		{
			m21 *= -1;
		}

		if(cfg.flip_v)
		{
			m12 *= -1;
		}
	}
	else if(cfg.rotate == 180)
	{
		m11 = -scale;
		m12 = 0;
		m21 = 0;
		m22 = -scale;

		if(cfg.flip_h)
		{
			m11 *= -1;
		}

		if(cfg.flip_v)
		{
			m22 *= -1;
		}
	}
	else if(cfg.rotate == 270)
	{
		m11 = 0;
		m12 = -scale;
		m21 = scale;
		m22 = 0;

		if(cfg.flip_h)
		{
			m21 *= -1;
		}

		if(cfg.flip_v)
		{
			m12 *= -1;
		}
	}

	graphicsView->setMatrix(QMatrix(m11, m12, m21, m22, 0, 0));

	emit resizeEvent(NULL);
}

void MainWindow::resizeEvent(QResizeEvent*)
{
	graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

	getScale();

	groupBox_map->setTitle(tr("Map [ Zoom Factor = %1 ]").arg(QString::number(scale, 'f', 2)));
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	if(childAt(event->pos())->parent() == graphicsView)
	{
		if(event->button() == Qt::LeftButton)
		{
			rubber_pos = graphicsView->mapFromGlobal(event->globalPos());

			rubberBand->setGeometry(QRect(rubber_pos, QSize()));
			rubberBand->show();
		}
		else if(event->button() == Qt::MiddleButton)
		{
			drag_pos = event->pos();

			setCursor(QCursor(Qt::ClosedHandCursor));
		}
	}
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	if(childAt(event->pos()) && childAt(event->pos())->parent() == graphicsView)
	{
		if(event->buttons() & Qt::LeftButton)
		{
			rubberBand->setGeometry(QRect(rubber_pos, graphicsView->mapFromGlobal(event->globalPos())).normalized());

			QToolTip::showText(event->globalPos(), QString("%1 x %2").arg(qAbs((int)((graphicsView->mapToScene(graphicsView->mapFromGlobal(event->globalPos())).x() - graphicsView->mapToScene(rubber_pos).x()) * MAPFACTOR))).arg(qAbs((int)((graphicsView->mapToScene(graphicsView->mapFromGlobal(event->globalPos())).y() - graphicsView->mapToScene(rubber_pos).y()) * MAPFACTOR))));
		}
		else if(event->buttons() & Qt::MiddleButton)
		{
			QScrollBar *h = graphicsView->horizontalScrollBar();
			QScrollBar *v = graphicsView->verticalScrollBar();

			h->setValue(h->value() - (event->pos().x() - drag_pos.x()));
			v->setValue(v->value() - (event->pos().y() - drag_pos.y()));

			drag_pos = event->pos();
		}
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if(childAt(event->pos()) && childAt(event->pos())->parent() == graphicsView)
	{
		if(event->button() == Qt::LeftButton)
		{
			QPointF src = graphicsView->mapToScene(rubber_pos) * MAPFACTOR;
			QPointF dst = graphicsView->mapToScene(graphicsView->mapFromGlobal(event->globalPos())) * MAPFACTOR;
			int x1, y1, x2, y2, swap;
			int rc;

			rubberBand->hide();

			x1 = cfg.swap_x ? src.x() : MAPSIZE - src.x();
			y1 = cfg.swap_y ? src.y() : MAPSIZE - src.y();
			x2 = cfg.swap_x ? dst.x() : MAPSIZE - dst.x();
			y2 = cfg.swap_y ? dst.y() : MAPSIZE - dst.y();

			if(x1 > x2)
			{
				swap = x1;
				x1 = x2;
				x2 = swap;
			}

			if(y1 > y2)
			{
				swap = y1;
				y1 = y2;
				y2 = swap;
			}

			if(src != dst)
			{
				if(png_flag_lt && png_flag_rt && png_flag_lb && png_flag_rb)
				{
					drawFlags(false, false);
				}

				pos_flag_lt = QPointF(src.x() / MAPFACTOR, src.y() / MAPFACTOR);
				pos_flag_rt = QPointF(dst.x() / MAPFACTOR, src.y() / MAPFACTOR);
				pos_flag_lb = QPointF(src.x() / MAPFACTOR, dst.y() / MAPFACTOR);
				pos_flag_rb = QPointF(dst.x() / MAPFACTOR, dst.y() / MAPFACTOR);

				drawFlags(true, false);

				rc = QMessageBox::question(this, APPNAME, tr("Start zone cleaning for selected region?\n\n[ %1 / %2 - %3 / %4 ]").arg(x1).arg(y1).arg(x2).arg(y2), QMessageBox::Yes | QMessageBox::No | QMessageBox::Save, QMessageBox::Yes);

				if(rc == QMessageBox::Yes)
				{
					sendUDP(QString(MIIO_APP_ZONED_CLEAN).arg(x1).arg(y1).arg(x2).arg(y2).arg(1).arg("%1"));
				}
				else if(rc == QMessageBox::Save)
				{
					CLEANZONE zone = { QString("Zone %1").arg(cfg.zones.count() + 1), x1, y1, x2, y2, 1 };

					cfg.zones.append(zone);

					menu_map_zones->addAction(QIcon(":/png/png/zone.png"), zone.label);

					if(cfg.zones.count() == 1)
					{
						QMessageBox::information(this, APPNAME, tr("You can customize all zones later with the zone editor."));
					}
				}

				if(rc != QMessageBox::Yes)
				{
					drawFlags(false, false);
				}
			}
		}
		else if(event->button() == Qt::MiddleButton)
		{
			setCursor(QCursor(Qt::ArrowCursor));
		}
	}
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
	if(childAt(event->pos())->parent() == graphicsView)
	{
		qreal scaleFactor = (event->delta() > 0 ? 1.1 : 0.9);

		getScale();

		if(scaleFactor > 1 && scale * scaleFactor >= 10.0)
		{
			scaleFactor = 10.0 / scale;
		}
		else if(scaleFactor < 1 && scale * scaleFactor <= 0.01)
		{
			scaleFactor = 0.01 / scale;
		}

		graphicsView->scale(scaleFactor, scaleFactor);

		graphicsView->centerOn(graphicsView->mapToScene(graphicsView->mapFromGlobal(event->globalPos())));

		groupBox_map->setTitle(tr("Map [ Zoom Factor = %1 ]").arg(QString::number(scale, 'f', 2)));
	}
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
	if(childAt(event->pos())->parent() == graphicsView)
	{
		QAction *selected;

		if((selected = menu_map->exec(event->globalPos())))
		{
			if(selected->objectName() == "actionMenu_Map_Reset")
			{
				emit resizeEvent(NULL);
			}
			else if(selected->objectName() == "actionMenu_Map_Goto")
			{
				if(graphicsView->itemAt(graphicsView->mapFromGlobal(event->globalPos()).x(), graphicsView->mapFromGlobal(event->globalPos()).y()))
				{
					QPointF pos = graphicsView->mapToScene(graphicsView->mapFromGlobal(event->globalPos())) * MAPFACTOR;
					int x = pos.x();
					int y = pos.y();

					if(png_flag)
					{
						drawFlags(false, true);
					}

					pos_flag = QPointF(x / MAPFACTOR, y / MAPFACTOR);

					drawFlags(true, true);

					if(QMessageBox::question(this, APPNAME, tr("Send robot to selected position?\n\n[ %1 / %2 ]").arg(cfg.swap_x ? x : MAPSIZE - x).arg(cfg.swap_y ? y : MAPSIZE - y), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
					{
						sendUDP(QString(MIIO_APP_GOTO_TARGET).arg(cfg.swap_x ? x : MAPSIZE - x).arg(cfg.swap_y ? y : MAPSIZE - y).arg("%1"));
					}
					else
					{
						drawFlags(false, true);
					}
				}
				else
				{
					QMessageBox::information(this, APPNAME, tr("Please select position inside the map!"));
				}
			}
			else if(selected->objectName() == "actionMenu_Map_FlipH")
			{
				cfg.flip_h = selected->isChecked();

				setMatrix();
			}
			else if(selected->objectName() == "actionMenu_Map_FlipV")
			{
				cfg.flip_v = selected->isChecked();

				setMatrix();
			}
			else if(selected->objectName() == "actionMenu_Map_Rotate0")
			{
				cfg.rotate = 0;

				setMatrix();
			}
			else if(selected->objectName() == "actionMenu_Map_Rotate90")
			{
				cfg.rotate = 90;

				setMatrix();
			}
			else if(selected->objectName() == "actionMenu_Map_Rotate180")
			{
				cfg.rotate = 180;

				setMatrix();
			}
			else if(selected->objectName() == "actionMenu_Map_Rotate270")
			{
				cfg.rotate = 270;

				setMatrix();
			}
			else if(selected->objectName() == "actionMenu_Map_SwapX")
			{
				cfg.swap_x = selected->isChecked();
			}
			else if(selected->objectName() == "actionMenu_Map_SwapY")
			{
				cfg.swap_y = selected->isChecked();
			}
			else
			{
				foreach(CLEANZONE zone, cfg.zones)
				{
					if(zone.label == selected->text())
					{
						if(QMessageBox::question(this, APPNAME, tr("Start cleaning for zone \"%1\"?").arg(zone.label), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
						{
							sendUDP(QString(MIIO_APP_ZONED_CLEAN).arg(zone.x1).arg(zone.y1).arg(zone.x2).arg(zone.y2).arg(zone.times).arg("%1"));
						}

						break;
					}
				}
			}
		}
	}
}

void MainWindow::keyPressEvent(QKeyEvent *ke)
{
	if(ke->key() == Qt::Key_F1)
	{
		on_actionHelp_triggered();
	}
	else if(ke->key() == Qt::Key_Escape)
	{
		close();
	}

	QMainWindow::keyPressEvent(ke);
}

void MainWindow::closeEvent(QCloseEvent *ce)
{
	if(QMessageBox::question(this, APPNAME, tr("Really exit program?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
	{
		cfg.map = actionMap->isChecked();

		setConfig();

		ce->accept();
	}
	else
	{
		ce->ignore();
	}
}
