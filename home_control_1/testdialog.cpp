#include "testdialog.h"
#include "ui_testdialog.h"
#include <qstandarditemmodel.h>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QFile>
//#include <QMessageBox>
#include <QTextStream>
#include "data_types.h"
//#include "callcenterdialog.h"
#include "maindialog.h"
#include "ui_maindialog.h"
//#include "terminalhelpmaindialog.h"
//#include "helpdialog.h"
#include <QSettings>
#include "data_types.h"

/************************************************************
  该文件为对讲管理对话框主程序
************************************************************/

extern char g_strNativeName[50];
//extern char g_strLphoneName[3][50];
extern MainDialog *g_pMainDlg;

/************************************************************
描述：对讲管理对话框构造函数
参数：parent - 父窗口指针
返回：无
************************************************************/
TestDialog::TestDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TestDialog)
{
	ui->setupUi(this);

	if (strstr(QT_BUILD_KEY, "arm") != 0) //not arm-linux-gcc
	{
		setWindowFlags(Qt::FramelessWindowHint);
		hide();
	}

	/*ui->pushButtonCallCenter->setText("");
	ui->pushButtonDelMedia->setText("");
	ui->pushButtonEmergency->setText("");
	ui->pushButtonHelp->setText("");
	ui->pushButtonMediaDetail->setText("");
	ui->pushButtonNextMedia->setText("");
	ui->pushButtonPrevMedia->setText("");
	ui->pushButtonReturn->setText("");
	*/

	QStandardItemModel *model = new QStandardItemModel(0, 4);////7);

	ui->tableViewMediaList->setModel(model);
	ui->tableViewMediaList->verticalHeader()->hide();
	ui->tableViewMediaList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewMediaList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewMediaList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableViewMediaList->setSortingEnabled(false);
	ui->tableViewMediaList->setShowGrid(false);
	ui->tableViewMediaList->setFocusPolicy(Qt::NoFocus);
	ui->tableViewMediaList->setIconSize(QSize(64, 32));

	/*
	model->setHeaderData(0, Qt::Horizontal, tr("序号"));
	model->setHeaderData(1, Qt::Horizontal, tr("日期"));
	model->setHeaderData(2, Qt::Horizontal, tr("类型"));
	model->setHeaderData(3, Qt::Horizontal, tr("通话时间"));
	model->setHeaderData(4, Qt::Horizontal, tr("呼叫地址"));
	model->setHeaderData(5, Qt::Horizontal, tr("状态"));
	model->setHeaderData(6, Qt::Horizontal, tr("存盘"));
	*/

	////ui->tableViewMediaList->setColumnWidth(0, 50);
	////ui->tableViewMediaList->setColumnWidth(1, 190);
	////ui->tableViewMediaList->setColumnWidth(2, 95);
	////ui->tableViewMediaList->setColumnWidth(3, 90);
	////ui->tableViewMediaList->setColumnWidth(4, 140);
	////ui->tableViewMediaList->setColumnWidth(5, 75);
	////ui->tableViewMediaList->setColumnWidth(6, 50);
	ui->tableViewMediaList->setColumnWidth(0, 90);
	ui->tableViewMediaList->setColumnWidth(1, 200);
	ui->tableViewMediaList->setColumnWidth(2, 186);
	ui->tableViewMediaList->setColumnWidth(3, 50);

	ui->tableViewMediaList->setColumnHidden(3, true);

	ui->tableViewMediaList->horizontalHeader()->setResizeMode(0, QHeaderView::Fixed);
	ui->tableViewMediaList->horizontalHeader()->setResizeMode(1, QHeaderView::Fixed);
	ui->tableViewMediaList->horizontalHeader()->setResizeMode(2, QHeaderView::Fixed);
	ui->tableViewMediaList->horizontalHeader()->setResizeMode(3, QHeaderView::Fixed);

	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotMediaListPressed()));

	////connect(ui->tableViewMediaList->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
	////		this, SLOT(tvMediaListSortIndicatorChanged(int, Qt::SortOrder)));

	//ReloadMediaList((char *)MEDIA_LIST_FILE_PATH);

	QSettings settings_config(CONFIG_FILE, QSettings::IniFormat);
	m_newVisitorCnt = settings_config.value("info/new_visitor_cnt", 0).toInt();
	//m_newVisitorCnt = 0;
	//((MainDialog *)parent)->getUi()->labelIconVisitorCnt->setText("\n\n" + QString::number(m_newVisitorCnt));
	//emit g_pMainDlg->emitSigSetNotify(0);
	QStandardItemModel *model2 = (QStandardItemModel *)g_pMainDlg->ui->tableViewNotifier->model();
	if (m_newVisitorCnt == 0)
	{
		model2->item(2, 1)->setIcon(QIcon(":/images/main/call_none.png"));
		model2->item(2, 2)->setText(MainDialog::tr("无未接呼叫"));
	}
	else
	{
		model2->item(2, 1)->setIcon(QIcon(":/images/main/call.png"));
		model2->item(2, 2)->setText(MainDialog::tr("有") + QString::number(m_newVisitorCnt) + MainDialog::tr("个呼叫"));
	}

	m_pMediaPlayerDlg = new MediaPlayerDialog(this);

	ReloadMediaList((char *)"/mnt/disk/media/record.xml");//(char *)MEDIA_LIST_FILE_PATH);
}

/************************************************************
描述：对讲管理对话框析构函数
参数：无
返回：无
************************************************************/
TestDialog::~TestDialog()
{
	delete m_pMediaPlayerDlg;
	delete m_pTimer;

	delete ui;
}

/************************************************************
描述：获取对讲记录列表控件
参数：无
返回：对讲记录列表控件的指针
************************************************************/
QTableView *TestDialog::getTableViewMediaList()
{
	return ui->tableViewMediaList;
}

/************************************************************
描述：读取路径为pathName的xml格式文件，将读到的对讲记录加入对讲记录列
	 表控件
参数：pathName - 存对讲记录的xml文件的路径
返回：0 - 成功
	 -1 - 失败
************************************************************/
int TestDialog::ReloadMediaList(char *pathName)
{
	QFile file(pathName);
	QDomDocument doc;
	QDomElement root;
	QDomElement infoElement;
	int nRow;
	QStandardItemModel *model = (QStandardItemModel *)ui->tableViewMediaList->model();

	////model->setHeaderData(0, Qt::Horizontal, tr("序号"));
	////model->setHeaderData(1, Qt::Horizontal, tr("日期"));
	////model->setHeaderData(2, Qt::Horizontal, tr("类型"));
	////model->setHeaderData(3, Qt::Horizontal, tr("通话时间"));
	////model->setHeaderData(4, Qt::Horizontal, tr("呼叫地址"));
	////model->setHeaderData(5, Qt::Horizontal, tr("状态"));
	////model->setHeaderData(6, Qt::Horizontal, tr("存盘"));
	model->setHeaderData(0, Qt::Horizontal, "");
	model->setHeaderData(1, Qt::Horizontal, tr("号码信息"));
	model->setHeaderData(2, Qt::Horizontal, tr("对讲时间"));
	model->setHeaderData(3, Qt::Horizontal, tr("存盘"));

	model->removeRows(0, model->rowCount());

	if (!file.open(QIODevice::ReadOnly))
	{
		return -1;
	}

	if (!doc.setContent(&file))
	{
		file.close();
		return -1;
	}

	root = doc.firstChildElement("sysrecord");
	infoElement = root.lastChildElement("record");

	nRow = 0;

	while (!infoElement.isNull())// && (nRow <= ui->tableViewMediaList->model()->rowCount() - 1))
	{
		QString datetime = infoElement.attribute(QString("datetime"));
		QString type = infoElement.attribute(QString("type"));
		QString address = infoElement.attribute(QString("ipaddress"));
		QString state = infoElement.attribute(QString("state"));
		QString aviurl = infoElement.attribute(QString("aviurl"));

		if (datetime == "datetime")
		{
			break;
		}

		if (nRow >= 45)
		{
			QFile::remove(aviurl);

			infoElement = infoElement.previousSiblingElement("record");

			nRow++;

			continue;
		}

		////QString duration;
		//if (state == "LinphoneCallSuccess")
		//{
		////duration = infoElement.attribute(QString("duration"));
		////duration = duration.append(tr("秒"));
		//}
		//else
		//{
		//	duration = QString("0秒");
		//}

		QString callTypeIconFile;
		if (type == "receive")
		{
			if (state == "LinphoneCallRecord")
			{
				callTypeIconFile = ":/images/intercomman_dialog/status_callin_record.png"; //tr("留言");
			}
			else if (state == "LinphoneCallSuccess")
			{
				callTypeIconFile = ":/images/intercomman_dialog/status_callin.png"; //tr("接收成功");
			}
			else
			{
				callTypeIconFile = ":/images/intercomman_dialog/status_callin_failed.png"; //tr("接收失败");
			}
		}
		else if (type == "send")
		{
			if (state == "LinphoneCallSuccess")
			{
				callTypeIconFile = ":/images/intercomman_dialog/status_callout.png"; //tr("发送成功");
			}
			else
			{
				callTypeIconFile = ":/images/intercomman_dialog/status_callout_failed.png"; //tr("发送失败");
			}
		}

		int year, month, day, hour, minute, second;
		sscanf(datetime.toLocal8Bit().data(), "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day,
			   &hour, &minute, &second);
		datetime.sprintf("%d-%d-%d %d:%02d", year, month, day, hour, minute);

		QString have_avifile;
		QFile avi_file(aviurl);
		if (avi_file.open(QIODevice::ReadOnly))
		{
			have_avifile = "yes";
			avi_file.close();
		}
		else
		{
			have_avifile = "no";
		}

		QString strTemp = address;
		int pos = strTemp.indexOf("?");
		if (pos >= 0)
		{
			strTemp = strTemp.left(pos);
		}

		QStringList &addressList = g_pMainDlg->m_pAddressBookDlg->m_addressListNetName;
		int i;
		for (i = 0; i < addressList.count(); i++)
		{
			QString str = addressList.at(i);

			if (strTemp.length() > str.length())
			{
				if ((strTemp.left(str.length()) == str) && (strTemp.at(str.length()) == QChar('-')))
				{
					strTemp = g_pMainDlg->m_pAddressBookDlg->m_addressListName.at(i) + strTemp.mid(str.length());
					break;
				}
			}
			else if (strTemp == str)
			{
				strTemp = g_pMainDlg->m_pAddressBookDlg->m_addressListName.at(i);
				break;
			}
		}

		if (i >= addressList.count())
		{
			pos = strTemp.indexOf("-");
			if ((pos != -1) && ((pos != strTemp.length() - 2) || (strTemp.at(strTemp.length() - 1).isDigit())))
			{
				QString strTemp2 = g_strNativeName;//g_strLphoneName[0];

				int pos2;
				pos2 = strTemp2.indexOf("?");
				if (pos2 >= 0) strTemp2 = strTemp2.left(pos2);

				pos2 = strTemp2.indexOf("-");
				if ((pos2 != -1) && ((pos2 != strTemp2.length() - 2) || (strTemp2.at(strTemp2.length() - 1).isDigit())))
				{
					strTemp2 = strTemp2.left(pos2);
					if (strTemp.left(pos) == strTemp2)
					{
						strTemp = strTemp.mid(pos + strlen("-"));
					}
				}
			}
		}

		QStandardItem *newItem0 = new QStandardItem(QIcon(callTypeIconFile), "");
		QStandardItem *newItem1 = new QStandardItem(strTemp);
		QStandardItem *newItem2 = new QStandardItem(datetime);
		QStandardItem *newItem3 = new QStandardItem(have_avifile);

		model->setItem(nRow, 0, newItem0);
		model->setItem(nRow, 1, newItem1);
		model->setItem(nRow, 2, newItem2);
		model->setItem(nRow, 3, newItem3);

		newItem0->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
		newItem1->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
		newItem2->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
		newItem3->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

		ui->tableViewMediaList->setRowHeight(nRow, 34);

		infoElement = infoElement.previousSiblingElement("record");

		nRow++;
	}

	file.close();

	QDomNodeList list = doc.elementsByTagName("record");
	QString aviurl_del = list.at(1).toElement().attribute(QString("aviurl"));
	//删除list中第二条纪录
	root = doc.documentElement();

	int i;
	for (i = 45; i < nRow; i++)
	{
		root.removeChild(list.at(1));
	}

	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		QTextStream out(&file);
		doc.save(out,4);
		file.close();
	}

	//ui->tableViewMediaList->horizontalHeader()->setSortIndicator(1, Qt::DescendingOrder);
	if (model->rowCount() > 0)
	{
		ui->tableViewMediaList->selectRow(0);
		slotMediaListPressed();
	}

	////setTableViewMediaListColor(TABLE_VIEW_COLOR);

	return 0;
}

/************************************************************
描述：关闭对讲管理对话框
参数：无
返回：无
************************************************************/
void TestDialog::on_pushButtonReturn_pressed()
{
	done(0);
}

/************************************************************
描述：选择上一条对讲记录
参数：无
返回：无
************************************************************/
void TestDialog::on_pushButtonPrevMedia_pressed()
{
	//QStandardItemModel *model = (QStandardItemModel *)ui->tableViewMediaList->model();
	int row = ui->tableViewMediaList->verticalScrollBar()->value(); //ui->tableViewInfoList->currentIndex().row();

	if (row - TABLE_ROWS_PER_PAGE >= 0)
	{
		ui->tableViewMediaList->selectRow(row - TABLE_ROWS_PER_PAGE);
		ui->tableViewMediaList->verticalScrollBar()->setSliderPosition(row - TABLE_ROWS_PER_PAGE);
	}
	else if (row > 0)
	{
		ui->tableViewMediaList->selectRow(0);
		ui->tableViewMediaList->verticalScrollBar()->setSliderPosition(0);
	}
	/*else if (model->rowCount() > TABLE_ROWS_PER_PAGE)
	{
		ui->tableViewMediaList->selectRow(model->rowCount() - TABLE_ROWS_PER_PAGE);
		ui->tableViewMediaList->verticalScrollBar()->setSliderPosition(model->rowCount() - TABLE_ROWS_PER_PAGE);
	}*/

	slotMediaListPressed();
}

/************************************************************
描述：选择下一条对讲记录
参数：无
返回：无
************************************************************/
void TestDialog::on_pushButtonNextMedia_pressed()
{
	QStandardItemModel *model = (QStandardItemModel *)ui->tableViewMediaList->model();
	int row = ui->tableViewMediaList->verticalScrollBar()->value(); //ui->tableViewInfoList->currentIndex().row();

	if (row + TABLE_ROWS_PER_PAGE < model->rowCount())
	{
		ui->tableViewMediaList->selectRow(row + TABLE_ROWS_PER_PAGE);
		ui->tableViewMediaList->verticalScrollBar()->setSliderPosition(row + TABLE_ROWS_PER_PAGE);
	}
	/*else
	{
		ui->tableViewMediaList->selectRow(0);
		ui->tableViewMediaList->verticalScrollBar()->setSliderPosition(0);
	}*/

	slotMediaListPressed();
}

/************************************************************
描述：打开当前对讲记录录像回放窗口
参数：无
返回：无
************************************************************/
void TestDialog::on_pushButtonMediaDetail_pressed()
{
	/*g_pMainDlg->m_pSecurityMainDlg->show();
	g_pMainDlg->m_pSecurityMainDlg->raise();
	g_pMainDlg->m_pSecurityMainDlg->m_pSecurityPlayerDlg->setDetail("/mnt/disk/sec_media/20121014-150553-023.avi", "");
	g_pMainDlg->m_pSecurityMainDlg->m_pSecurityPlayerDlg->show();
	g_pMainDlg->m_pSecurityMainDlg->m_pSecurityPlayerDlg->repaint();
	g_pMainDlg->m_pSecurityMainDlg->m_pSecurityPlayerDlg->startPlayer();

	return;

	//printf("TestDialog::on_pushButtonMediaDetail_clicked\n");
	if (m_pMediaPlayerDlg->isVisible()) return; //bug

	QFile file("/mnt/disk/media/record.xml");//MEDIA_LIST_FILE_PATH);
	QDomDocument doc;
	QDomElement root;
	QDomElement infoElement;
	int i;
	int row;

	if (!file.open(QIODevice::ReadOnly))
		return;

	if (!doc.setContent(&file))
	{
		file.close();
		return ;
	}

	file.close();

	root = doc.firstChildElement("sysrecord");
	infoElement = root.lastChildElement("record");

	row = ui->tableViewMediaList->currentIndex().row();

	if (row == -1) return;

	for (i = 0; i < row; i++)
	{
		infoElement = infoElement.previousSiblingElement("record");
	}

	QString aviurl = infoElement.attribute(QString("aviurl"));
	QString netAddress = infoElement.attribute(QString("ipaddress"));
	QStandardItemModel *model = (QStandardItemModel *)ui->tableViewMediaList->model();

	if (model->item(row, 0))
	{
		QString address = model->item(row, 1)->text();

		if (model->item(row, 3)->text() == "yes")
		{*/
			m_pMediaPlayerDlg->setDetail("", "", "/mnt/disk/sec_media/20121014-150553-023.avi");
			m_pMediaPlayerDlg->show();
			m_pMediaPlayerDlg->repaint();
			m_pMediaPlayerDlg->startPlayer();
		//}
	//}
}

/************************************************************
描述：删除对讲记录列表所有信息记录和所有相关对讲录像文件
参数：无
返回：无
************************************************************/
void TestDialog::on_pushButtonDelMediaAll_pressed()
{
	QStandardItemModel *model;
	model = (QStandardItemModel *)ui->tableViewMediaList->model();

	if (model->rowCount() == 0) return;

	setAllButtonsEnabled(this, false);
	ui->tableViewMediaList->setFocus(Qt::TabFocusReason);
	MyMessageBox messBox(this);
	CommonPushButton *okButton = messBox.addButton(QObject::tr("确定"));
	CommonPushButton *cancelButton = messBox.addButton(QObject::tr("取消"), "MyMessageBoxCancelButton");
	Q_UNUSED(okButton);
	cancelButton->setFocus(Qt::TabFocusReason);
	messBox.setText(tr("确定删除所有记录?"));
	messBox.exec();
	setAllButtonsEnabled(this, true);
	ui->pushButtonDelMediaAll->setFocus(Qt::TabFocusReason);
	if(messBox.clickedButton() == cancelButton)
	{
		return;
	}

	{
		QFile file("/mnt/disk/media/record.xml");
		QDomDocument doc;
		if (!file.open(QIODevice::ReadOnly)) return;
		if (!doc.setContent(&file))
		{
			file.close();
			return;
		}
		file.close();

		QDomElement root = doc.documentElement();
		QDomNodeList list = doc.elementsByTagName("record");
		//QString aviurl; //= list.at(row2).toElement().attribute(QString("aviurl"));
		int cnt = list.count();

		int i;
		for (i = 1; i < cnt; i++)
		{
			//aviurl = list.at(1).toElement().attribute(QString("aviurl"));
			//QFile::remove(aviurl);
			root.removeChild(list.at(1));
		}

		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
		QTextStream out(&file);
		doc.save(out, 4);
		file.close();

		model->removeRows(0, model->rowCount());

		system("/bin/rm /mnt/disk/media/*.avi");
	}
}

/************************************************************
描述：删除当前对讲记录和相关对讲录像文件
参数：无
返回：无
************************************************************/
void TestDialog::on_pushButtonDelMedia_pressed()
{
	int row;

	row = ui->tableViewMediaList->currentIndex().row();
	if (row == -1)	return;

	setAllButtonsEnabled(this, false);
	ui->tableViewMediaList->setFocus(Qt::TabFocusReason);
	MyMessageBox messBox(this);
	CommonPushButton *okButton = messBox.addButton(QObject::tr("确定"));
	CommonPushButton *cancelButton = messBox.addButton(QObject::tr("取消"), "MyMessageBoxCancelButton");
	Q_UNUSED(okButton);
	cancelButton->setFocus(Qt::TabFocusReason);
	messBox.setText(tr("确定删除?"));
	messBox.exec();
	setAllButtonsEnabled(this, true);
	ui->pushButtonDelMedia->setFocus(Qt::TabFocusReason);
	if(messBox.clickedButton() == cancelButton)
	{
		return;
	}

	row = ui->tableViewMediaList->currentIndex().row();

	QFile file("/mnt/disk/media/record.xml");
	QDomDocument doc;
	if (!file.open(QIODevice::ReadOnly)) return;
	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomNodeList list = doc.elementsByTagName("record");
	int row2 = list.count() - 1 - row;
	QString aviurl = list.at(row2).toElement().attribute(QString("aviurl"));
	int i;

	root.removeChild(list.at(row2));

	for (i = row2; i < list.count(); i++)
	{
		list.at(i).toElement().attributeNode("recordNum").setValue(QString::number(i));
	}

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
	QTextStream out(&file);
	doc.save(out,4);
	file.close();

	QStandardItemModel *model;
	model = (QStandardItemModel *)ui->tableViewMediaList->model();
	model->removeRow(row);

	/*for(i = row; i < model->rowCount(); i++)
	{
		model->item(i, 0)->setText(QString::number(i + 1));
	}*/

	QFile::remove(aviurl);

	////setTableViewMediaListColor(TABLE_VIEW_COLOR);
}

/************************************************************
描述：对讲记录列表控件双击回调函数
参数：index - 对讲列表控件当前项的序号
返回：无
************************************************************/
void TestDialog::on_tableViewMediaList_doubleClicked(QModelIndex index)
{
	Q_UNUSED(index);

	//on_pushButtonMediaDetail_pressed();
}

/************************************************************
描述：设置对讲记录列表控件的背景色为按行号白-非白显示
参数：color - 非白行的背景色
返回：无
************************************************************/
/*void TestDialog::setTableViewMediaListColor(QColor color)
{
	int i, j;
	QStandardItemModel *model = (QStandardItemModel *)ui->tableViewMediaList->model();

	for (i = 0; i < model->rowCount(); i++)
	{
		QColor col;
		if (i % 2 == 1)
		{
			col = color;
		}
		else
		{
			col = QColor(255, 255, 255);
		}

		for (j = 0; j < model->columnCount(); j++)
		{
			QStandardItem *item = model->item(i, j);
			if (item)
			{
				item->setBackground(QBrush(col));
			}
		}

		model->item(i, 0)->setText(QString::number(i + 1));
	}
}
*/

/************************************************************
描述：窗口显示时，刷新Tab焦点，将主窗口新对讲记录数设为0
参数：无
返回：无
************************************************************/
void TestDialog::showEvent(QShowEvent *)
{
	MainDialog *pMainDlg = (MainDialog *)parent();

	pMainDlg->showRefreshTabFocus(this);

	if (m_newVisitorCnt > 0)
	{
		m_newVisitorCnt = 0;

		emit g_pMainDlg->emitSigSetNotify(0);
		/*QStandardItemModel *model = (QStandardItemModel *)g_pMainDlg->ui->tableViewNotifier->model();
		model->item(2, 1)->setIcon(QIcon(":/images/main/call_none.png"));
		model->item(2, 2)->setText(MainDialog::tr("无未接呼叫"));*/
	}

	QSettings settings_config(CONFIG_FILE, QSettings::IniFormat);
	settings_config.setValue("info/new_visitor_cnt", m_newVisitorCnt);

	//pMainDlg->getUi()->labelIconVisitorCnt->setText("\n\n" + QString::number(m_newVisitorCnt));

	if (ui->pushButtonMediaDetail->isEnabled())
	{
		ui->pushButtonMediaDetail->setFocus(Qt::TabFocusReason);
	}
	else
	{
		ui->pushButtonNextMedia->setFocus(Qt::TabFocusReason);
	}
}

/************************************************************
描述：窗口隐藏时，刷新Tab焦点
参数：无
返回：无
************************************************************/
void TestDialog::hideEvent(QHideEvent *)
{
	MainDialog *pMainDlg = (MainDialog *)parent();

	m_pMediaPlayerDlg->hide();

	pMainDlg->hideRefreshTabFocus(this);

	if (pMainDlg->m_pCurActiveDlg == pMainDlg)
	{
		pMainDlg->ui->pushButtonIntercomMan->setFocus(Qt::TabFocusReason);
	}
}

/************************************************************
描述：按动对讲记录列表控件排序指示符的回调函数
参数：logicalIndex - 排序的列号
	 order - Qt::DescendingOrder 按降序排序
			 Qt::AscendingOrder 按升序排序
返回：无
************************************************************/
/*void TestDialog::tvMediaListSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
	Q_UNUSED(logicalIndex);
	Q_UNUSED(order);

	setTableViewMediaListColor(TABLE_VIEW_COLOR);
}*/

void TestDialog::on_tableViewMediaList_pressed(QModelIndex index)
{
	Q_UNUSED(index);

	m_pTimer->start(200);
}

void TestDialog::slotMediaListPressed()
{
	m_pTimer->stop();

	int row = ui->tableViewMediaList->currentIndex().row();
	if (row < 0) return;

	QStandardItemModel *model = (QStandardItemModel *)ui->tableViewMediaList->model();
	QString strHaveAvi = model->item(row, 3)->text();
	ui->pushButtonMediaDetail->setEnabled(strHaveAvi == "yes");
}
