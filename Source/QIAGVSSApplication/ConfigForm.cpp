﻿#include "ConfigForm.h"
#include "ui_ConfigForm.h"

ConfigForm::ConfigForm(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ConfigForm)
{
	ui->setupUi(this);

	init();
}

ConfigForm::~ConfigForm()
{
	delete ui;
}

void ConfigForm::init()
{
	ui->groupBox->setTitle(QString::fromLocal8Bit("数据库服务器参数配置"));
	ui->groupBox_2->setTitle(QString::fromLocal8Bit("网络服务器参数配置"));
	ui->groupBox_3->setTitle(QString::fromLocal8Bit("WMS服务器参数配置"));
	ui->groupBox_4->setTitle(QString::fromLocal8Bit("串口参数配置"));

	// 更新串口号
	updatePort();

	// 波特率
	ui->comboBoxBaud->addItem("9600");
	ui->comboBoxBaud->addItem("19200");
	ui->comboBoxBaud->addItem("38400");
	ui->comboBoxBaud->addItem("57600");
	ui->comboBoxBaud->addItem("115200");

	QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	ui->lineEditNetSerIP->setValidator(new QRegExpValidator(rx, this));
	ui->lineEditNetSerPort->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));

	// 读取服务器配置参数
	Config config("./res/set/config.ini");
	QString serverName = config.get("DataBase", "ServerName").toString();
	QString databaseName = config.get("DataBase", "DataBaseName").toString();
	QString loginName = config.get("DataBase", "LoginName").toString();
	QString passwd = config.get("DataBase", "Passwd").toString();
	QString netip = config.get("Net", "IP").toString();
	QString netport = config.get("Net", "Port").toString();
	QString com = config.get("SerialPort", "Com").toString();
	QString num = config.get("SerialPort", "Num").toString();
	QString baud = config.get("SerialPort", "Baud").toString();

	// 填充相应文本框
	ui->lineEditSerName->setText(serverName);
	ui->lineEditDbName->setText(databaseName);
	ui->lineEditLoginName->setText(loginName);
	ui->lineEditPasswd->setText(passwd);
	ui->lineEditNetSerIP->setText(netip);
	ui->lineEditNetSerPort->setText(netport);
	ui->comboBoxComName->setCurrentText(com);
	ui->lineEditComNumber->setText(num);
	ui->comboBoxBaud->setCurrentText(baud);
}

void ConfigForm::on_pushButtonSetDb_clicked()
{
	QString serverName = ui->lineEditSerName->text();
	QString databaseName = ui->lineEditDbName->text();
	QString loginName = ui->lineEditLoginName->text();
	QString passwd = ui->lineEditPasswd->text();

	if (serverName.isEmpty() || databaseName.isEmpty() || loginName.isEmpty() || passwd.isEmpty())
	{
		MsgBoxEx* msgBox = new MsgBoxEx();
		msgBox->setMsgBoxMode(QString::fromLocal8Bit("所有数据库服务器参数都不可为空！"));
		delete msgBox;
		return;
	}

	// 设置服务器参数
	Config config("./res/set/config.ini");
	config.set("DataBase", "ServerName", QString("%1").arg(serverName));
	config.set("DataBase", "DataBaseName", QString("%1").arg(databaseName));
	config.set("DataBase", "LoginName", QString("%1").arg(loginName));
	config.set("DataBase", "Passwd", QString("%1").arg(passwd));

	MsgBoxEx* msgBox = new MsgBoxEx();
	msgBox->setMsgBoxMode(QString::fromLocal8Bit("数据库服务器参数设置成功，若要应用此设置请重启程序！"), 3000);
	delete msgBox;
}

void ConfigForm::on_pushButtonSetNet_clicked()
{
	QString ip = ui->lineEditNetSerIP->text();
	QString port = ui->lineEditNetSerPort->text();

	if (ip.isEmpty() || port.isEmpty())
	{
		MsgBoxEx* msgBox = new MsgBoxEx();
		msgBox->setMsgBoxMode(QString::fromLocal8Bit("网络服务器参数不可为空！"));
		delete msgBox;
		return;
	}

	Config config("./res/set/config.ini");
	config.set("Net", "IP", QString("%1").arg(ip));
	config.set("Net", "Port", QString("%1").arg(port));

	MsgBoxEx* msgBox = new MsgBoxEx();
	msgBox->setMsgBoxMode(QString::fromLocal8Bit("网络服务器参数设置成功，若要应用此设置请重启程序！"), 3000);
	delete msgBox;
}

void ConfigForm::onNetServerStateChange(QString strLinkDesc)
{
	ui->labelNetState->setText(strLinkDesc);
}

void ConfigForm::onSerialPortStateChange(QString strOpenDesc)
{
	ui->labelComState->setText(strOpenDesc);
}

void ConfigForm::on_pushButtonSetCom_clicked()
{
	QString com = ui->comboBoxComName->currentText();
	QString num = ui->lineEditComNumber->text();
	QString baud = ui->comboBoxBaud->currentText();
	QString details = ui->labelComDetails->text();

	if (com.isEmpty() || baud.isEmpty())
	{
		MsgBoxEx* msgBox = new MsgBoxEx();
		msgBox->setMsgBoxMode(QString::fromLocal8Bit("串口参数不可为空！"));
		delete msgBox;
		return;
	}

	Config config("./res/set/config.ini");
	config.set("SerialPort", "Com", QString("%1").arg(com));
	config.set("SerialPort", "Num", QString("%1").arg(num));
	config.set("SerialPort", "Baud", QString("%1").arg(baud));
	config.set("SerialPort", "Details", QString("%1").arg(details));

	MsgBoxEx* msgBox = new MsgBoxEx();
	msgBox->setMsgBoxMode(QString::fromLocal8Bit("串口参数设置成功，若要应用此设置请重启程序！"), 3000);
	delete msgBox;
}

void ConfigForm::on_comboBoxComName_currentTextChanged(const QString& arg1)
{
	QString strValue = m_mapSerialPort[arg1];
	QStringList listValue = strValue.split(";");

	if (listValue.count() != 2)
		return;
	ui->lineEditComNumber->setText(listValue[0]);
	ui->labelComDetails->setText(listValue[1]);
}

void ConfigForm::on_pushButtonRefresh_clicked()
{
	updatePort();
}

void ConfigForm::updatePort()
{
	m_mapSerialPort.clear();
	ui->comboBoxComName->clear();
	ui->lineEditComNumber->setText("");
	ui->comboBoxBaud->setCurrentIndex(-1);
	ui->labelComDetails->setText("");

	QSerialPort serial;
	QStringList listPort;
	const auto infos = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& info : infos)
	{
		//serial.setPort(info);
		m_mapSerialPort[info.portName()] = info.serialNumber() + ";" + info.description();
		//serial.close();
	}

	for (QMap<QString, QString>::iterator it = m_mapSerialPort.begin(); it != m_mapSerialPort.end(); it++)
	{
		if (!it.key().isEmpty())
		{
			ui->comboBoxComName->addItem(it.key());
		}
	}

	ui->comboBoxComName->setCurrentIndex(-1);
}
