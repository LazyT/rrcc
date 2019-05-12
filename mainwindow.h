#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define APPNAME "RoboRock Control Center"
#define APPVERS "0.8.4"
#define APPDATE "22.04.2019"

#include <QApplication>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>
#include <QMessageBox>
#include <QCloseEvent>
#include <QCryptographicHash>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWebSocket>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTimer>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFileDialog>
#include <QStandardPaths>
#include <QClipboard>
#include <QDesktopServices>
#include <QGraphicsPixmapItem>
#include <QScrollBar>
#include <QToolTip>
#include <QDebug>

#include "ccryptlib.h"
#include "qaesencryption.h"
#include "qarchive.h"
#include "qsshsocket.h"

#include "ui_mainwindow.h"
#include "logger.h"
#include "setup.h"
#include "history.h"
#include "timer.h"
#include "upload.h"
#include "update.h"
#include "packager.h"
#include "unpackager.h"
#include "converter.h"
#include "about.h"
#include "zones.h"
#include "installer.h"
#include "uninstaller.h"
#include "download.h"
#include "search.h"

#define CFG_H	QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.rrcc/rrcc.cfg"
#define CFG_P	QCoreApplication::applicationDirPath() + "/rrcc.cfg"
#define LOG		QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/rrcc.log"
#define TGZ		QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/rrcc.tgz"
#define PKG		QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/rrcc.pkg"

#define PKG_KEY "r0ckrobo#23456"

#define TIMEOUT 2500

#define MAPSIZE		51200			// 51200 x 51200
#define MAPFACTOR	12.5			// MAPSIZE : 4096

#define WORKTIME_MAINBRUSH	1080000	// 300h
#define WORKTIME_SIDEBRUSH	 720000	// 200h
#define WORKTIME_FILTER		 540000	// 150h
#define WORKTIME_SENSOR		 108000	// 30h

#define MIIO_HELLO					"21310020ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
#define MIIO_FIND_ME				"{'id':%1,'method':'find_me'}"
#define MIIO_APP_START				"{'id':%1,'method':'app_start'}"
#define MIIO_APP_STOP				"{'id':%1,'method':'app_stop'}"
#define MIIO_APP_PAUSE				"{'id':%1,'method':'app_pause'}"
#define MIIO_APP_CHARGE				"{'id':%1,'method':'app_charge'}"
#define MIIO_APP_SPOT				"{'id':%1,'method':'app_spot'}"
#define MIIO_APP_ZONED_CLEAN		"{'id':%6,'method':'app_zoned_clean','params':[[%1,%2,%3,%4,%5]]}"
#define MIIO_GET_SERIAL_NUMBER		"{'id':%1,'method':'get_serial_number'}"
#define MIIO_GET_CONSUMABLE			"{'id':%1,'method':'get_consumable'}"
#define MIIO_RESET_CONSUMABLE		"{'id':%2,'method':'reset_consumable','params':['%1']}"
#define MIIO_GET_STATUS				"{'id':%1,'method':'get_status'}"
#define MIIO_GET_CLEAN_SUMMARY		"{'id':%1,'method':'get_clean_summary'}"
#define MIIO_GET_CLEAN_RECORD		"{'id':%2,'method':'get_clean_record','params':[%1]}"
#define MIIO_SET_CUSTOM_MODE		"{'id':%2,'method':'set_custom_mode','params':[%1]}"
#define MIIO_GET_TIMER				"{'id':%1,'method':'get_timer'}"
#define MIIO_SET_TIMER				"{'id':%4,'method':'set_timer','params':[['%1',['%2',['start_clean',%3]]]]}"
#define MIIO_UPD_TIMER				"{'id':%3,'method':'upd_timer','params':['%1','%2']}"
#define MIIO_DEL_TIMER				"{'id':%2,'method':'del_timer','params':['%1']}"
#define MIIO_GET_DND_TIMER			"{'id':%1,'method':'get_dnd_timer'}"
#define MIIO_SET_DND_TIMER			"{'id':%2,'method':'set_dnd_timer','params':[%1]}"
#define MIIO_CLOSE_DND_TIMER		"{'id':%1,'method':'close_dnd_timer'}"
#define MIIO_GET_SOUND_VOLUME		"{'id':%1,'method':'get_sound_volume'}"
#define MIIO_CHANGE_SOUND_VOLUME	"{'id':%2,'method':'change_sound_volume','params':[%1]}"
#define MIIO_TEST_SOUND_VOLUME		"{'id':%1,'method':'test_sound_volume'}"
#define MIIO_GET_CURRENT_SOUND		"{'id':%1,'method':'get_current_sound'}"
#define MIIO_DNLD_INSTALL_SOUND		"{'id':%4,'method':'dnld_install_sound','params':{'url':'%1','md5':'%2','sid':%3}}"
#define MIIO_GET_SOUND_PROGRESS		"{'id':%1,'method':'get_sound_progress'}"
#define MIIO_GET_CARPET_MODE		"{'id':%1,'method':'get_carpet_mode'}"
#define MIIO_SET_CARPET_MODE		"{'id':%6,'method':'set_carpet_mode','params':[{'enable':%1,'current_integral':%2,'current_high':%3,'current_low':%4,'stall_time':%5}]}"
#define MIIO_APP_GOTO_TARGET		"{'id':%3,'method':'app_goto_target','params':[%1,%2]}"
#define MIIO_SET_LAB_STATUS			"{'id':%2,'method':'set_lab_status','params':[%1]}"
#define MIIO_SAVE_MAP				"{'id':%2,'method':'save_map','params':[%1]}"
#define MIIO_CONFIG_ROUTER			"{'id':%3,'method':'miIO.config_router','params':{'ssid':'%1','passwd':'%2','uid':null}}"
#define MIIO_OTA					"{'id':%3,'method':'miIO.ota','params':{'mode':'normal','install':'1','app_url':'%1','file_md5':'%2','proc':'dnld install'}}"
#define MIIO_GET_OTA_STATE			"{'id':%1,'method':'miIO.get_ota_state'}"
#define MIIO_GET_OTA_PROGRESS		"{'id':%1,'method':'miIO.get_ota_progress'}"

#define SSH_GET_FIRMWARE_VERSION "fgrep ROBOROCK_VERSION /etc/os-release"
#define SSH_GET_VALETUDO_VERSION "fgrep -a -m1 -A1 '\"name\": \"valetudo\"' /usr/local/bin/valetudo"

enum {MIIO_ID_HELLO, MIIO_ID_APP_START, MIIO_ID_APP_STOP, MIIO_ID_APP_PAUSE, MIIO_ID_APP_CHARGE, MIIO_ID_APP_SPOT, MIIO_ID_APP_ZONED_CLEAN, MIIO_ID_FIND_ME, MIIO_ID_GET_SERIAL_NUMBER, MIIO_ID_GET_CONSUMABLE, MIIO_ID_RESET_CONSUMABLE, MIIO_ID_GET_STATUS, MIIO_ID_GET_CLEAN_SUMMARY, MIIO_ID_GET_CLEAN_RECORD, MIIO_ID_SET_CUTOM_MODE, MIIO_ID_GET_TIMER, MIIO_ID_SET_TIMER, MIIO_ID_UPD_TIMER, MIIO_ID_DEL_TIMER, MIIO_ID_GET_DND_TIMER, MIIO_ID_SET_DND_TIMER, MIIO_ID_CLOSE_DND_TIMER, MIIO_ID_GET_SOUND_VOLUME, MIIO_ID_CHANGE_SOUND_VOLUME, MIIO_ID_TEST_SOUND_VOLUME, MIIO_ID_GET_CURRENT_SOUND, MIIO_ID_DNLD_INSTALL_SOUND, MIIO_ID_GET_SOUND_PROGRESS, MIIO_ID_GET_CARPET_MODE, MIIO_ID_SET_CARPET_MODE, MIIO_ID_APP_GOTO_TARGET, MIIO_ID_SET_LAB_STATUS, MIIO_ID_SAVE_MAP, MIIO_ID_CONFIG_ROUTER, MIIO_ID_OTA, MIIO_ID_GET_OTA_STATE, MIIO_ID_GET_OTA_PROGRESS};
enum {AES_ENCRYPT, AES_DECRYPT};
enum {FANSPEED_QUIET = 38, FANSPEED_BALANCED = 60, FANSPEED_TURBO = 77, FANSPEED_MAXIMUM = 90};

struct TIMER
{
	QString id;
	QString state;
	QString crontab;
	QString command;
	int fanspeed;
};

struct CLEANZONE
{
	QString label;
	int x1;
	int y1;
	int x2;
	int y2;
	int times;
};

struct NOGOZONE
{
	int x1;
	int y1;
	int x2;
	int y2;
	int x3;
	int y3;
	int x4;
	int y4;
};

struct VIRTWALL
{
	int x1;
	int y1;
	int x2;
	int y2;
};

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:

	explicit MainWindow(QWidget *parent = 0);

	QMenu *menu_map_zones;
	QByteArray did, cnt;
	uint timediff;
	QString src_ip;
	QByteArray AESPayload(bool, QByteArray);
	bool sendUDP(QString);
	void parseJSON(int, QByteArray);
	bool provisioning = false;

	QStringList qarchive_error_strings
	{
		tr("No error"),
		tr("Archive has bad data"),
		tr("Archive does not exist or no permission"),
		tr("Unknown error"),
		tr("Fatal error"),
		tr("Wrong password for archive"),
		tr("Empty password for archive"),
		tr("Cannot create archive"),
		tr("Cannot open added file"),
		tr("Cannot read added file"),
		tr("Cannot find extraction destination"),
		tr("File does not exists"),
		tr("Extraction destination invalid"),
		tr("Memory allocation failed"),
		tr("Cannot open existing file")
	};

	QStringList ssh_error_strings
	{
		"SocketError",
		"SessionCreationError",
		"ChannelCreationError",
		"ScpChannelCreationError",
		"ScpPullRequestError",
		"ScpPushRequestError",
		"ScpFileNotCreatedError",
		"ScpReadError",
		"ScpWriteError",
		"PasswordAuthenticationFailedError",
		"KeyAuthenticationFailedError"
	};

	struct CONFIG
	{
		QString ip;
		QString token;
		qlonglong msgid;
		bool map;
		bool websocket;
		QString ssh_user;
		QString ssh_pass;
		QString ssh_pkey;
		QString ssh_pkpp;
		QString ssh_auth;
		bool flip_h;
		bool flip_v;
		bool swap_x;
		bool swap_y;
		int rotate;
		QVector<CLEANZONE> zones;

	}cfg;

	struct ROBO
	{
		QString serial_number;

		struct CONSUMABLE
		{
			int main_brush_work_time;
			int side_brush_work_time;
			int filter_work_time;
			int sensor_dirty_time;

		}consumable;

		struct STATUS
		{
			int msg_ver;
			int msg_seq;
			int state;
			int battery;
			int clean_time;
			int clean_area;
			int error_code;
			int map_present;
			int in_cleaning;
			int in_returning;
			int in_fresh_state;
			int lab_status;
			int fan_power;
			int dnd_enabled;

		}status;

		struct CLEANSUMMARY
		{
			int time;
			int area;
			int count;
			QVector<int> id;

		}cleansummary;

		struct CLEANRECORD
		{
			int begin;
			int finish;
			int duration;
			int area;
			int error;
			int complete;

		}cleanrecord;

		QVector<TIMER> timers;

		struct DND
		{
			int enabled;
			int start_hour;
			int start_minute;
			int end_hour;
			int end_minute;

		}dnd;

		struct CARPETMODE
		{
			int enable;
			int current_integral;
			int current_high;
			int current_low;
			int stall_time;

		}carpetmode;

		int volume;

		struct CURRENTSOUND
		{
			QString bom;
			QString language;
			QString location;
			int msg_ver;
			int sid_in_progress;
			int sid_in_use;
			int sid_version;

		}currentsound;

		struct SOUNDPROGRESS
		{
			int error;
			int progress;
			int sid_in_progress;
			int state;

		}soundprogress;

		struct OTA
		{
			QString state;
			int progress;

		}ota;

		QVector<VIRTWALL> virtwalls;
		QVector<NOGOZONE> nogozones;

	}robo = {};

private:

	QTranslator baseTranslator, helpTranslator, appTranslator;

	QStringList state_strings
	{
		tr("Starting"),
		tr("Charger disconnected"),
		tr("Idle"),
		tr("Remote control active"),
		tr("Cleaning"),
		tr("Returning home"),
		tr("Manual mode"),
		tr("Charging"),
		tr("Charging problem"),
		tr("Paused"),
		tr("Spot cleaning"),
		tr("Error"),
		tr("Shutting down"),
		tr("Updating"),
		tr("Docking"),
		tr("Going to target"),
		tr("Zoned cleaning")
	};

	void getConfig();
	void setConfig();
	void findIP();
	void getStatus();
	void drawMapFromJson(QByteArray);
	void drawMapFromJsonOld(QByteArray);
	void getScale();
	void setMatrix();
	void drawFlags(bool, bool);

	QDialog *logger;
	QTimer timerMap, timerFanspeed;
	QNetworkAccessManager *netmgr;
	QWebSocket *websocket;
	QGraphicsScene *scene;
	QPoint drag_pos, rubber_pos;
	QRect zone_preview_rect = {0, 0, 0, 0};
	QGraphicsPixmapItem *png_flag = NULL, *png_flag_lt = NULL, *png_flag_rt = NULL, *png_flag_lb = NULL, *png_flag_rb = NULL;
	QPointF pos_flag, pos_flag_lt, pos_flag_rt, pos_flag_lb, pos_flag_rb;
	QGraphicsRectItem *zone_preview_item = NULL;
	QMenu *menu_map, *menu_map_rotation, *menu_map_flipping, *menu_map_swapping;
	QActionGroup *group_map;
	QRubberBand *rubberBand;
	qreal scale;
	QSshSocket *ssh;
	QString ssh_cmd;

private slots:

	void on_actionExit_triggered();
	void on_actionRefresh_triggered();
	void on_actionMap_toggled(bool);
	void on_actionSetup_triggered();
	void on_actionLogger_triggered();
	void on_actionTimer_triggered();
	void on_actionHistory_triggered();
	void on_actionZones_triggered();
	void on_actionValetudoVersion_triggered();
	void on_actionValetudoInstall_triggered();
	void on_actionValetudoUninstall_triggered();
	void on_actionCheckFirmware_triggered();
	void on_actionSearchFirmware_triggered();
	void on_actionDownloadFirmware_triggered();
	void on_actionUpdateFirmware_triggered();
	void on_actionHelp_triggered();
	void on_actionAbout_triggered();

	void on_toolButton_Dock_clicked();
	void on_toolButton_Start_clicked();
	void on_toolButton_Spot_clicked();
	void on_toolButton_Pause_clicked();
	void on_toolButton_Stop_clicked();
	void on_toolButton_Find_clicked();

	void on_toolButton_Fanspeed1_clicked();
	void on_toolButton_Fanspeed2_clicked();
	void on_toolButton_Fanspeed3_clicked();
	void on_toolButton_Fanspeed4_clicked();
	void on_toolButton_Mainbrush_clicked();
	void on_toolButton_Sidebrush_clicked();
	void on_toolButton_Filter_clicked();
	void on_toolButton_Sensors_clicked();

	void on_dial_valueChanged(int);

	void timer_refreshMap();
	void timer_setFanspeed();

	void httpFinished(QNetworkReply*);

	void websocketTextMessageReceived(QString);
	void websocketError(QAbstractSocket::SocketError);

	void hovered(QAction*);
	void aboutToHide();

	void ssh_connected();
	void ssh_disconnected();
	void ssh_error(QSshSocket::SshError);
	void ssh_loginSuccessful();
	void ssh_commandExecuted(QString, QString);

	void resizeEvent(QResizeEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	void contextMenuEvent(QContextMenuEvent*);
	void keyPressEvent(QKeyEvent*);
	void closeEvent(QCloseEvent*);
};

#endif // MAINWINDOW_H
