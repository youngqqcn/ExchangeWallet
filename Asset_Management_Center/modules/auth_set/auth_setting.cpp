/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      权限设置相关功能,包括插入权限标志到数据库,初始化权限treeWidget
*              树的各节点,节点选中状态变更实时更新
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     无修改                       v0.0.1
********************************************************************/
#include "auth_setting.h"
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

//#define INT_AUTH_FLAG 0

CAuthSetting::CAuthSetting(QObject *parent) : QObject(parent)
{
    //不能再构造函数中  初始化
    /*int iRet = Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "权限设置: 初始化数据库失败";
    }*/
}

CAuthSetting::~CAuthSetting()
{

}

int CAuthSetting::Init()
{
    __m_strDBConnectName = QString("AuthSetting");

    if (g_bIsLAN)
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", __m_strDBConnectName);
        __m_db.setDatabaseName(STR_TEST_ASSERTDB_DB_NAME);
        __m_db.setHostName("192.168.10.81");
        __m_db.setPort(3306);
        __m_db.setUserName("root");
        __m_db.setPassword("MyNewPass4!");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }
    else
    {
        __m_db = QSqlDatabase::addDatabase("QMYSQL", __m_strDBConnectName);
        __m_db.setDatabaseName("assertdb");
        __m_db.setHostName("rm-wz991r2rjs3wmd9t1io.mysql.rds.aliyuncs.com");
        __m_db.setPort(3306);
        __m_db.setUserName("accertdb");
        __m_db.setPassword("CG1R47JxIfBofG5uIofHcUKW0Ay1f8");
        __m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1;");
    }

    if (!__m_db.open())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("连接(打开)数据库失败, 错误信息:") << __m_db.lastError().text();
        return DB_ERR;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("启动数据库成功!" );
    return NO_ERROR;
}

void CAuthSetting::InitAuthlist(QTreeWidget *treeWidget)
{
    treeWidget->clear();    //初始化树形控件

    //充值列表
    QTreeWidgetItem* group1 = new QTreeWidgetItem(treeWidget);
    group1->setText(0,"充值列表");    //树形控件显示的文本信息
    group1->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);   //设置树形控件子项的属性
    group1->setCheckState(0,Qt::Unchecked); //初始状态没有被选中
    //第一组子项
    QTreeWidgetItem* subItem11 = new QTreeWidgetItem(group1);
    subItem11->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem11->setText(0,"搜索");  //设置子项显示的文本
    subItem11->setCheckState(0,Qt::Unchecked); //设置子选项的显示格式和状态
    QTreeWidgetItem* subItem12 = new QTreeWidgetItem(group1);
    subItem12->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem12->setText(0,"重置");
    subItem12->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem13 = new QTreeWidgetItem(group1);
    subItem13->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem13->setText(0,"释放");
    subItem13->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem14 = new QTreeWidgetItem(group1);
    subItem14->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem14->setText(0,"审核");
    subItem14->setCheckState(0,Qt::Unchecked);
    //---
    //地址数量
    QTreeWidgetItem* group2 = new QTreeWidgetItem(treeWidget);
    group2->setText(0,"地址数量");
    group2->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group2->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem21 = new QTreeWidgetItem(group2);   //指定子项属于哪一个父项
    subItem21->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem21->setText(0,"搜索");
    subItem21->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem22 = new QTreeWidgetItem(group2);
    subItem22->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem22->setText(0,"重置");
    subItem22->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem23 = new QTreeWidgetItem(group2);
    subItem23->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem23->setText(0,"导入地址");
    subItem23->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem24 = new QTreeWidgetItem(group2);
    subItem24->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem24->setText(0,"查看详情");
    subItem24->setCheckState(0,Qt::Unchecked);
    //---
    //地址详情
    QTreeWidgetItem* group3 = new QTreeWidgetItem(treeWidget);
    group3->setText(0,"地址详情");
    group3->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group3->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem31 = new QTreeWidgetItem(group3);
    subItem31->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem31->setText(0,"搜索");
    subItem31->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem32 = new QTreeWidgetItem(group3);
    subItem32->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem32->setText(0,"重置");
    subItem32->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem33 = new QTreeWidgetItem(group3);
    subItem33->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem33->setText(0,"流水详情");
    subItem33->setCheckState(0,Qt::Unchecked);
    //---
    //    //待归集列表
    //    QTreeWidgetItem* group4 = new QTreeWidgetItem(treeWidget);
    //    group4->setText(0,"待归集列表");
    //    group4->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    //    group4->setCheckState(0,Qt::Unchecked);
    //    QTreeWidgetItem* subItem41 = new QTreeWidgetItem(group4);
    //    subItem41->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    //    subItem41->setText(0,"查询");
    //    subItem41->setCheckState(0,Qt::Unchecked);
    //    QTreeWidgetItem* subItem42 = new QTreeWidgetItem(group4);
    //    subItem42->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    //    subItem42->setText(0,"生成归集充值地址清单");
    //    subItem42->setCheckState(0,Qt::Unchecked);
    //    //---
    //创建未签名归集交易
    QTreeWidgetItem* group4 = new QTreeWidgetItem(treeWidget);
    group4->setText(0,"创建未签名归集交易");
    group4->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group4->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem41 = new QTreeWidgetItem(group4);
    subItem41->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem41->setText(0,"查询");
    subItem41->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem42 = new QTreeWidgetItem(group4);
    subItem42->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem42->setText(0,"根据选中地址创建归集交易并导出");
    subItem42->setCheckState(0,Qt::Unchecked);
    //---
    //归集交易广播
    QTreeWidgetItem* group5 = new QTreeWidgetItem(treeWidget);
    group5->setText(0,"归集交易广播");
    group5->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group5->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem51 = new QTreeWidgetItem(group5);
    subItem51->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem51->setText(0,"查询");
    subItem51->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem52 = new QTreeWidgetItem(group5);
    subItem52->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem52->setText(0,"确认广播选中");
    subItem52->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem53 = new QTreeWidgetItem(group5);
    subItem53->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem53->setText(0,"导入待广播文件");
    subItem53->setCheckState(0,Qt::Unchecked);
    //---
    //大地址监控
    QTreeWidgetItem* group6 = new QTreeWidgetItem(treeWidget);
    group6->setText(0,"大地址监控");
    group6->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group6->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem61 = new QTreeWidgetItem(group6);
    subItem61->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem61->setText(0,"查询");
    subItem61->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem62 = new QTreeWidgetItem(group6);
    subItem62->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem62->setText(0,"重置");
    subItem62->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem63 = new QTreeWidgetItem(group6);
    subItem63->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem63->setText(0,"添加监控地址");
    subItem63->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem64 = new QTreeWidgetItem(group6);
    subItem64->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem64->setText(0,"删除选中地址");
    subItem64->setCheckState(0,Qt::Unchecked);
    //---
    //归集日志
    QTreeWidgetItem* group7 = new QTreeWidgetItem(treeWidget);
    group7->setText(0,"归集日志");
    group7->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group7->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem71 = new QTreeWidgetItem(group7);
    subItem71->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem71->setText(0,"查询");
    subItem71->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem72 = new QTreeWidgetItem(group7);
    subItem72->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem72->setText(0,"重置");
    subItem72->setCheckState(0,Qt::Unchecked);
    //---
    //自动提币列表
    QTreeWidgetItem* group8 = new QTreeWidgetItem(treeWidget);
    group8->setText(0,"自动提币列表");
    group8->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group8->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem81 = new QTreeWidgetItem(group8);
    subItem81->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem81->setText(0,"搜索");
    subItem81->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem82 = new QTreeWidgetItem(group8);
    subItem82->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem82->setText(0,"重置");
    subItem82->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem83 = new QTreeWidgetItem(group8);
    subItem83->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem83->setText(0,"停止提现");
    subItem83->setCheckState(0,Qt::Unchecked);
    //---
    //初审列表
    QTreeWidgetItem* group9 = new QTreeWidgetItem(treeWidget);
    group9->setText(0,"初审列表");
    group9->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group9->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem91 = new QTreeWidgetItem(group9);
    subItem91->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem91->setText(0,"搜索");
    subItem91->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem92 = new QTreeWidgetItem(group9);
    subItem92->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem92->setText(0,"重置");
    subItem92->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem93 = new QTreeWidgetItem(group9);
    subItem93->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem93->setText(0,"审核驳回");
    subItem93->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem94 = new QTreeWidgetItem(group9);
    subItem94->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem94->setText(0,"审核");
    subItem94->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem95 = new QTreeWidgetItem(group9);
    subItem95->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem95->setText(0,"查看用户资产");
    subItem95->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem96 = new QTreeWidgetItem(group9);
    subItem96->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem96->setText(0,"查看用户流水");
    subItem96->setCheckState(0,Qt::Unchecked);
    //---
    //复审列表
    QTreeWidgetItem* group10 = new QTreeWidgetItem(treeWidget);
    group10->setText(0,"复审列表");
    group10->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group10->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem101 = new QTreeWidgetItem(group10);
    subItem101->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem101->setText(0,"搜索");
    subItem101->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem102 = new QTreeWidgetItem(group10);
    subItem102->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem102->setText(0,"重置");
    subItem102->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem103 = new QTreeWidgetItem(group10);
    subItem103->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem103->setText(0,"审核驳回");
    subItem103->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem105 = new QTreeWidgetItem(group10);
    subItem105->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem105->setText(0,"查看用户资产");
    subItem105->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem106 = new QTreeWidgetItem(group10);
    subItem106->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem106->setText(0,"查看用户流水");
    subItem106->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem104 = new QTreeWidgetItem(group10);
    subItem104->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem104->setText(0,"选中复审通过");
    subItem104->setCheckState(0,Qt::Unchecked);
    //---
    //人工提币-用户数据
    QTreeWidgetItem* group11 = new QTreeWidgetItem(treeWidget);
    group11->setText(0,"用户数据");
    group11->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group11->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem111 = new QTreeWidgetItem(group11);
    subItem111->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem111->setText(0,"确认");
    subItem111->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem112 = new QTreeWidgetItem(group11);
    subItem112->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem112->setText(0,"重置");
    subItem112->setCheckState(0,Qt::Unchecked);
    //---
    //人工提币-用户流水
    QTreeWidgetItem* group12 = new QTreeWidgetItem(treeWidget);
    group12->setText(0,"用户流水");
    group12->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group12->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem121 = new QTreeWidgetItem(group12);
    subItem121->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem121->setText(0,"查询");
    subItem121->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem122 = new QTreeWidgetItem(group12);
    subItem122->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem122->setText(0,"初审");
    subItem122->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem123 = new QTreeWidgetItem(group12);
    subItem123->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem123->setText(0,"复审");
    subItem123->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem124 = new QTreeWidgetItem(group12);
    subItem124->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem124->setText(0,"查看数据");
    subItem124->setCheckState(0,Qt::Unchecked);
    //---
    //创建未签名提币交易
    QTreeWidgetItem* group13 = new QTreeWidgetItem(treeWidget);
    group13->setText(0,"创建未签名提币交易");
    group13->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group13->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem131 = new QTreeWidgetItem(group13);
    subItem131->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem131->setText(0,"查询");
    subItem131->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem132 = new QTreeWidgetItem(group13);
    subItem132->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem132->setText(0,"根据选中订单创建未签名交易");
    subItem132->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem133 = new QTreeWidgetItem(group13);
    subItem133->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem133->setText(0,"导出已创建的未签名交易");
    subItem133->setCheckState(0,Qt::Unchecked);
    //---
    //人工提币交易广播
    QTreeWidgetItem* group14 = new QTreeWidgetItem(treeWidget);
    group14->setText(0,"人工提币交易广播");
    group14->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group14->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem141 = new QTreeWidgetItem(group14);
    subItem141->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem141->setText(0,"搜索");
    subItem141->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem142 = new QTreeWidgetItem(group14);
    subItem142->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem142->setText(0,"导入待广播文件");
    subItem142->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem143 = new QTreeWidgetItem(group14);
    subItem143->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem143->setText(0,"确认广播选中");
    subItem143->setCheckState(0,Qt::Unchecked);
    //---
    //人工提币列表
    QTreeWidgetItem* group15 = new QTreeWidgetItem(treeWidget);
    group15->setText(0,"人工提币列表");
    group15->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group15->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem151 = new QTreeWidgetItem(group15);
    subItem151->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem151->setText(0,"搜索");
    subItem151->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem152 = new QTreeWidgetItem(group15);
    subItem152->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem152->setText(0,"重置");
    subItem152->setCheckState(0,Qt::Unchecked);
    subItem151->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem153 = new QTreeWidgetItem(group15);
    subItem153->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem153->setText(0,"驳回");
    subItem153->setCheckState(0,Qt::Unchecked);
    //---
    //管理员类型管理
    QTreeWidgetItem* group16 = new QTreeWidgetItem(treeWidget);
    group16->setText(0,"管理员类型管理");
    group16->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group16->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem161 = new QTreeWidgetItem(group16);
    subItem161->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem161->setText(0,"查询管理员类型");
    subItem161->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem162 = new QTreeWidgetItem(group16);
    subItem162->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem162->setText(0,"添加管理员类型");
    subItem162->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem163 = new QTreeWidgetItem(group16);
    subItem163->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem163->setText(0,"编辑");
    subItem163->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem164 = new QTreeWidgetItem(group16);
    subItem164->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem164->setText(0,"冻结");
    subItem164->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem165 = new QTreeWidgetItem(group16);
    subItem165->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem165->setText(0,"解冻");
    subItem165->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem166 = new QTreeWidgetItem(group16);
    subItem166->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem166->setText(0,"删除");
    subItem166->setCheckState(0,Qt::Unchecked);
    //---
    //添加管理员类型
    QTreeWidgetItem* group17 = new QTreeWidgetItem(treeWidget);
    group17->setText(0,"添加管理员类型");
    group17->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group17->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem171 = new QTreeWidgetItem(group17);
    subItem171->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem171->setText(0,"添加");
    subItem171->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem172 = new QTreeWidgetItem(group17);
    subItem172->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem172->setText(0,"编辑");
    subItem172->setCheckState(0,Qt::Unchecked);
    //---
    //管理员列表
    QTreeWidgetItem* group18 = new QTreeWidgetItem(treeWidget);
    group18->setText(0,"管理员列表");
    group18->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group18->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem181 = new QTreeWidgetItem(group18);
    subItem181->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem181->setText(0,"搜索");
    subItem181->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem182 = new QTreeWidgetItem(group18);
    subItem182->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem182->setText(0,"新增管理员");
    subItem182->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem183 = new QTreeWidgetItem(group18);
    subItem183->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem183->setText(0,"编辑");
    subItem183->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem184 = new QTreeWidgetItem(group18);
    subItem184->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem184->setText(0,"重置密码");
    subItem184->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem185 = new QTreeWidgetItem(group18);
    subItem185->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem185->setText(0,"冻结");
    subItem185->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem186 = new QTreeWidgetItem(group18);
    subItem186->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem186->setText(0,"解冻");
    subItem186->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem187 = new QTreeWidgetItem(group18);
    subItem187->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem187->setText(0,"删除");
    subItem187->setCheckState(0,Qt::Unchecked);
    //---
    //操作日志
    QTreeWidgetItem* group19 = new QTreeWidgetItem(treeWidget);
    group19->setText(0,"操作日志");
    group19->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group19->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem191 = new QTreeWidgetItem(group19);
    subItem191->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem191->setText(0,"搜索");
    subItem191->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem192 = new QTreeWidgetItem(group19);
    subItem192->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem192->setText(0,"冻结");
    subItem192->setCheckState(0,Qt::Unchecked);
    //---
    //平台流水
    QTreeWidgetItem* group20 = new QTreeWidgetItem(treeWidget);
    group20->setText(0,"平台流水");
    group20->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group20->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem201 = new QTreeWidgetItem(group20);
    subItem201->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem201->setText(0,"查询");
    subItem201->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem202 = new QTreeWidgetItem(group20);
    subItem202->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem202->setText(0,"导出EXCEL文档");
    subItem202->setCheckState(0,Qt::Unchecked);
    //---
    //充值日报
    QTreeWidgetItem* group21 = new QTreeWidgetItem(treeWidget);
    group21->setText(0,"充值日报");
    group21->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group21->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem211 = new QTreeWidgetItem(group21);
    subItem211->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem211->setText(0,"查询");
    subItem211->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem212 = new QTreeWidgetItem(group21);
    subItem212->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem212->setText(0,"导出EXCEL文档");
    subItem212->setCheckState(0,Qt::Unchecked);
    //---
    //提现日报
    QTreeWidgetItem* group22 = new QTreeWidgetItem(treeWidget);
    group22->setText(0,"提现日报");
    group22->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group22->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem221 = new QTreeWidgetItem(group22);
    subItem221->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem221->setText(0,"查询");
    subItem221->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem222 = new QTreeWidgetItem(group22);
    subItem222->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem222->setText(0,"导出EXCEL文档");
    subItem222->setCheckState(0,Qt::Unchecked);
    //---
    //添加币种
    QTreeWidgetItem* group23 = new QTreeWidgetItem(treeWidget);
    group23->setText(0,"添加币种");
    group23->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    group23->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem231 = new QTreeWidgetItem(group23);
    subItem231->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem231->setText(0,"查询");
    subItem231->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem232 = new QTreeWidgetItem(group23);
    subItem232->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem232->setText(0,"添加币种");
    subItem232->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem233 = new QTreeWidgetItem(group23);
    subItem233->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem233->setText(0,"编辑币种");
    subItem233->setCheckState(0,Qt::Unchecked);
    QTreeWidgetItem* subItem234 = new QTreeWidgetItem(group23);
    subItem234->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    subItem234->setText(0,"删除币种");
    subItem234->setCheckState(0,Qt::Unchecked);
    //---
    //手动充值
//    QTreeWidgetItem* group24 = new QTreeWidgetItem(treeWidget);
//    group24->setText(0,"手动充值");
//    group24->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//    group24->setCheckState(0,Qt::Unchecked);
//    QTreeWidgetItem* subItem241 = new QTreeWidgetItem(group24);
//    subItem241->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//    subItem241->setText(0,"确认充值");
//    subItem241->setCheckState(0,Qt::Unchecked);
//    QTreeWidgetItem* subItem242 = new QTreeWidgetItem(group24);
//    subItem242->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//    subItem242->setText(0,"取消");
//    subItem242->setCheckState(0,Qt::Unchecked);
    //---


    treeWidget->setItemsExpandable(false);      //设置权限列表的节点的展开和收缩功能不可用
    treeWidget->expandAll();                        //设置权限列表始终为展开状态
}

void CAuthSetting::OnTreeItemChanged(QTreeWidgetItem *item)
{
    //QString itemText = item->text(0);
    if(Qt::Checked == item->checkState(0))
    {
        // QTreeWidgetItem* parent = item->parent();
        int count = item->childCount(); //返回子项的个数
        if(count >0)
        {
            for(int i=0; i<count; i++)
            {
                item->child(i)->setCheckState(0,Qt::Checked);
            }
        }
        else
        {
            updateParentItem(item);
        }
    }
    else if(Qt::Unchecked == item->checkState(0))
    {
        int count = item->childCount();
        if(count > 0)
        {
            for(int i=0; i<count; i++)
            {
                item->child(i)->setCheckState(0,Qt::Unchecked);
            }
        }
        else
        {
            updateParentItem(item);
        }
    }
//    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "第 " << column << " 列的选中状态变更:";
}

int CAuthSetting::__ExecTransaction(const int &admin_Type_ID, QTreeWidget *treeWidget)
{
    int uAuthAdmin1 = 0x0;  int uAuthAdmin2 = 0x0;  int uAuthAdmin3 = 0x0;  int uAuthAdmin4 = 0x0;  int uAuthAdmin5 = 0x0;  int uAuthAdmin6 = 0x0;
    int uAuthAdmin7 = 0x0;  int uAuthAdmin8 = 0x0;  int uAuthAdmin9 = 0x0;  int uAuthAdmin10 = 0x0;  int uAuthAdmin11 = 0x0;  int uAuthAdmin12 = 0x0;
    int uAuthAdmin13 = 0x0;  int uAuthAdmin14 = 0x0;  int uAuthAdmin15 = 0x0;  int uAuthAdmin16 = 0x0;  int uAuthAdmin17 = 0x0;  int uAuthAdmin18 = 0x0;
    int uAuthAdmin19 = 0x0;  int uAuthAdmin20 = 0x0;  int uAuthAdmin21 = 0x0;  int uAuthAdmin22 = 0x0;  int uAuthAdmin23 = 0x0;  /*int uAuthAdmin24 = 0x0;*/
    int flag = 1;

    QSqlQuery query(__m_db);

    QTreeWidgetItemIterator iter(treeWidget);

//    if(__m_db.transaction())
    if (query.exec("SET AUTOCOMMIT=0;"))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("设置不自动提交事务成功") ;
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("设置不自动提交事务失败, 请检查数据库") << query.lastError();
        //做清理工作....
        return DB_ERR;
    }
    query.clear();
    if (!query.exec("START TRANSACTION;"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开启事务失败" << query.lastError();
        return DB_ERR;
    }

    try{
        while (*iter)
        {
            //do something like
            //充值列表 flag = 1
            int INT_AUTH_FLAG = 2;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin1 |= INT_CTRL_AUTH_RECHARGELIST_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin1 |= INT_CTRL_AUTH_RECHARGELIST_RESET;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "释放" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin1 |= INT_CTRL_AUTH_RECHARGELIST_RELEASE;
            }
            if ((*iter)->text(0) == "审核" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin1 |= INT_CTRL_AUTH_RECHARGELIST_MESSAGE_NOTIFY;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set charge_list = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin1).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //地址数量 flag = 6
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin2 |= INT_CTRL_AUTH_ADDRLIST_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin2 |= INT_CTRL_AUTH_ADDRLIST_RESET;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "导入地址" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin2 |= INT_CTRL_AUTH_ADDRLIST_IMPORT;
            }
            if ((*iter)->text(0) == "查看详情" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin2 |= INT_CTRL_AUTH_ADDRLIST_DETAILS;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set addr_count_list = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin2).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //地址详情 flag = 11
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin3 |= INT_CTRL_AUTH_ADDRLISTDETAIL_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin3 |= INT_CTRL_AUTH_ADDRLISTDETAIL_RESET;
            }
            if ((*iter)->text(0) == "流水详情" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin3 |= INT_CTRL_AUTH_ADDRLISTDETAIL_RUNWATERDETAILS;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set addr_detail = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin3).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //创建未签名归集交易 flag = 15
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin4 |= INT_CTRL_AUTH_COLLECTIONCREATERAWTX_SEARCH;
            }
            if ((*iter)->text(0) == "根据选中地址创建归集交易并导出" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin4 |= INT_CTRL_AUTH_COLLECTIONCREATERAWTX_CREATE;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set collection_create_raw_Tx = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin4).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //归集交易广播 flag = 18
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin5 |= INT_CTRL_AUTH_COLLECTXBROADCAST_QUERY;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "确认广播选中" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin5 |= INT_CTRL_AUTH_COLLECTXBROADCAST_AFFIRMBROADCAST;
            }
            if ((*iter)->text(0) == "导入待广播文件" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin5 |= INT_CTRL_AUTH_COLLECTXBROADCAST_IMPORT;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set collection_broadcast = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin5).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //大地址监控 flag = 22
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin6 |= INT_CTRL_AUTH_BIGACCOUNTMONITORING_QUERY;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin6 |= INT_CTRL_AUTH_BIGACCOUNTMONITORING_RESET;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "添加监控地址" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin6 |= INT_CTRL_AUTH_BIGACCOUNTMONITORING_ADDMONITORINGADDR;
            }
            if ((*iter)->text(0) == "删除选中地址" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin6 |= INT_CTRL_AUTH_BIGACCOUNTMONITORING_DELETESELECTEDADDR;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set big_account_list = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin6).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //归集日志 flag = 27
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin7 |= INT_CTRL_AUTH_COLLECTLOG_SEARCH;
            }
            if ((*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin7 |= INT_CTRL_AUTH_COLLECTLOG_RESET;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set collection_log = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin7).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //自动提现列表 flag = 30
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin8 |= INT_CTRL_AUTH_WITHDRAW_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin8 |= INT_CTRL_AUTH_WITHDRAW_RESET;
            }
            if ((*iter)->text(0) == "停止提现" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin8 |= INT_CTRL_AUTH_WITHDRAW_STOPWITHDRAW;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set auto_withdraw = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin8).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //初审列表 flag = 34
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin9 |= INT_CTRL_AUTH_FIRSTTRIAL_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin9 |= INT_CTRL_AUTH_FIRSTTRIAL_RESET;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "审核驳回" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin9 |= INT_CTRL_AUTH_FIRSTTRIAL_REJECT;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "审核" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin9 |= INT_CTRL_AUTH_FIRSTTRIAL_AUDIT;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查看用户资产" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin9 |= INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERASSERT;
            }
            if ((*iter)->text(0) == "查看用户流水" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin9 |= INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERRUNWATER;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set manual_withdraw1 = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin9).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //复审列表 flag = 41
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin10 |= INT_CTRL_AUTH_RECHECK_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin10 |= INT_CTRL_AUTH_RECHECK_RESET;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "审核驳回" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin10 |= INT_CTRL_AUTH_RECHECK_REJECT;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查看用户资产" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin10 |= INT_CTRL_AUTH_RECHECK_CHECKUSERASSERT;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查看用户流水" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin10 |= INT_CTRL_AUTH_RECHECK_CHECKUSERRUNWATER;
            }
            if ((*iter)->text(0) == "选中复审通过" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin10 |= INT_CTRL_AUTH_RECHECK_SELECTEDRECHECKPASS;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set manual_withdraw2 = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin10).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //用户数据 flag = 48
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "确认" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin11 |= INT_CTRL_AUTH_USERDATA_AFFIRM;
            }
            if ((*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin11 |= INT_CTRL_AUTH_USERDATA_RESET;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set user_assert_data = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin11).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //用户流水 flag = 51
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin12 |= INT_CTRL_AUTH_USERRUNWATER_AFFIRM;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "初审" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin12 |= INT_CTRL_AUTH_USERRUNWATER_FIRST;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "复审" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin12 |= INT_CTRL_AUTH_USERRUNWATER_RECHECK;
            }
            if ((*iter)->text(0) == "查看数据" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin12 |= INT_CTRL_AUTH_USERRUNWATER_CHECKDATA;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set user_statements = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin12).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //创建未签名提币交易 flag = 56
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin13 |= INT_CTRL_AUTH_WITHDRAWCREATERAWTX_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "根据选中订单创建未签名交易" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin13 |= INT_CTRL_AUTH_WITHDRAWCREATERAWTX_CREATE;
            }
            if ((*iter)->text(0) == "导出已创建的未签名交易" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin13 |= INT_CTRL_AUTH_WITHDRAWCREATERAWTX_EXPORT;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set manual_withdraw_create_raw_Tx = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin13).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //人工提币交易广播 flag = 60
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin14 |= INT_CTRL_AUTH_MANUALCHECKBROADCAST_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "导入待广播文件" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin14 |= INT_CTRL_AUTH_MANUALCHECKBROADCAST_IMPORTBROADCASTFILE;
            }
            if ((*iter)->text(0) == "确认广播选中" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin14 |= INT_CTRL_AUTH_MANUALCHECKBROADCAST_AFFIRMBROADCASTSELECTED;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set manual_withdraw_broadcast = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin14).arg(admin_Type_ID);
                query.exec(strSql);
            }

            //人工提币列表 flag = 64
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin15 |= INT_PAGE_AUTH_WITHDRAWLIST_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin15 |= INT_PAGE_AUTH_WITHDRAWLIST_RESET;
            }
            if ((*iter)->text(0) == "驳回" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin15 |= INT_PAGE_AUTH_WITHDRAWLIST_REJECT;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set withdraw_list = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin15).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //管理员类型管理 flag = 68
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询管理员类型" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin16 |= INT_CTRL_AUTH_ADMINTYPEMNG_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "添加管理员类型" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin16 |= INT_CTRL_AUTH_ADMINTYPEMNG_ADD;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "编辑" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin16 |= INT_CTRL_AUTH_ADMINTYPEMNG_EDIT;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "冻结" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin16 |= INT_CTRL_AUTH_ADMINTYPEMNG_FREEZE;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "解冻" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin16 |= INT_CTRL_AUTH_ADMINTYPEMNG_UNFREEZE;
            }
            if ((*iter)->text(0) == "删除" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin16 |= INT_CTRL_AUTH_ADMINTYPEMNG_DELETE;
            }
            if (INT_AUTH_FLAG== flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set admin_type_list = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin16).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //添加管理员类型 flag = 75
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "添加" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin17 |= INT_CTRL_AUTH_ADDADMINTYPE_ADD;
            }
            if ((*iter)->text(0) == "编辑" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin17 |= INT_CTRL_AUTH_ADDADMINTYPE_EDIT;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set add_admin_type = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin17).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //管理员列表 flag = 78
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "新增管理员" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_ADD;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "编辑" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_EDIT;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "重置密码" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_RESET;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "冻结" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_FREEZE;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "解冻" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_UNFREEZE;
            }
            if ((*iter)->text(0) == "删除" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin18 |= INT_CTRL_AUTH_ADMINLIST_DELETE;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set admin_list = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin18).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //操作日志 flag = 86
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "搜索" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin19 |= INT_CTRL_AUTH_OPERATIONLOG_SEARCH;
            }
            if ((*iter)->text(0) == "冻结" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin19 |= INT_CTRL_AUTH_OPERATIONLOG_FREEZE;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set admin_log = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin19).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //平台流水 flag = 89
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin20 |= INT_CTRL_AUTH_PLATFORMFLOW_QUERY;
            }
            if ((*iter)->text(0) == "导出EXCEL文档" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin20 |= INT_CTRL_AUTH_PLATFORMFLOW_EXPORTEXCEL;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set exchange_statements = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin20).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //日报-充值日报 flag = 92
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin21 |= INT_CTRL_AUTH_RECHARGEDAILYPAPER_QUERY;
            }
            if ((*iter)->text(0) == "导出EXCEL文档" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin21 |= INT_CTRL_AUTH_RECHARGEDAILYPAPER_EXPORTEXCEL;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set exchange_charge_daily = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin21).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //日报-提现日报 flag = 95
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin22 |= INT_CTRL_AUTH_WITHDRAWDAILYPAPER_QUERY;
            }
            if ((*iter)->text(0) == "导出EXCEL文档" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin22 |= INT_CTRL_AUTH_WITHDRAWDAILYPAPER_EXPORTEXCEL;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set exchange_withdraw_daily = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin22).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //添加币种 flag = 98
            INT_AUTH_FLAG++;
            INT_AUTH_FLAG++;
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "查询" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin23 |= INT_CTRL_AUTH_ADDCOINTYPE_SEARCH;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "添加币种" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin23 |= INT_CTRL_AUTH_ADDCOINTYPE_ADD;
            }
            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "编辑币种" && Qt::Checked == (*iter)->checkState(0))
            {
                uAuthAdmin23 |= INT_CTRL_AUTH_ADDCOINTYPE_EDIT;
            }
            if ((*iter)->text(0) == "删除币种" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
            {
                uAuthAdmin23 |= INT_CTRL_AUTH_ADDCOINTYPE_DELETE;
            }
            if (INT_AUTH_FLAG == flag)
            {
                query.clear();
                QString strSql = QString("update tb_auth_flag set add_coin = %1 where admin_type_id = \'%2\' ").arg(uAuthAdmin23).arg(admin_Type_ID);
                query.exec(strSql);
            }
            //手动充值 flag = 103
//            INT_AUTH_FLAG++;
//            INT_AUTH_FLAG++;
//            if (INT_AUTH_FLAG++ == flag && (*iter)->text(0) == "确认充值" && Qt::Checked == (*iter)->checkState(0))
//            {
//                uAuthAdmin24 |= INT_CTRL_AUTH_MANUALTOPUP_AFFIRMGENERATE;
//                qDebug() << "确认充值" << uAuthAdmin24;
//            }
//            if ((*iter)->text(0) == "取消" && Qt::Checked == (*iter)->checkState(0) && (INT_AUTH_FLAG) == flag)
//            {
//                uAuthAdmin24 |= INT_CTRL_AUTH_MANUALTOPUP_CANCEL;
//                qDebug() << "取消:" << uAuthAdmin24;
//            }
//            if (INT_AUTH_FLAG == flag)
//            {
//                query.clear();
//                QString strSql = QString("update tb_auth_flag set airdrop_charge = \'%1\' where admin_type_id = \'%2\' ").arg(uAuthAdmin24).arg(admin_Type_ID);
//                query.exec(strSql);
//            }

            ++flag;
            ++iter;
        }

        //提交事务
        query.clear();
//        if(__m_db.commit())
        if (query.exec("COMMIT"))
        {
            qDebug() << tr("事务提交成功") ;
        }
        else
        {
            qDebug() << tr("事务提交失败, 请检查数据库") ;
//            __m_db.rollback();
            query.clear();
            query.exec("ROLLBACK;");
            return DB_ERR;
        }
        query.clear();
        if (!query.exec("SET AUTOCOMMIT=1;"))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" <<"设置数据库自动提交事务失败:" << query.lastError();
            return DB_ERR;
        }
    }
    catch(std::exception &e)
    {
        qDebug() << e.what() ;
        //做清理工作....
    }
    return NO_ERROR;
}

void CAuthSetting::updateParentItem(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    if(parent == NULL)
    {
        return ;
    }
    int selectedCount = 0;
    int childCount = parent->childCount();
    for(int i=0; i<childCount; i++) //判断有多少个子项被选中
    {
        QTreeWidgetItem* childItem = parent->child(i);
        if(childItem->checkState(0) == Qt::Checked)
        {
            selectedCount++;
        }
    }
    //    if(selectedCount <= 0)  //如果没有子项被选中，父项设置为未选中状态
    //    {
    //        parent->setCheckState(0,Qt::Unchecked);
    ////        parent->setCheckState(0, Qt::Checked);
    //    }
    //    else if(selectedCount>0 && selectedCount<childCount)    //如果有部分子项被选中，父项设置为部分选中状态，即用灰色显示
    //    {
    //        parent->setCheckState(0,Qt::PartiallyChecked);
    ////        parent->setCheckState(0, Qt::Checked);
    //    }
    //    else if(selectedCount == childCount)    //如果子项全部被选中，父项则设置为选中状态
    //    {
    //        parent->setCheckState(0,Qt::Checked);
    //    }
}

int CAuthSetting::AddAdminType(const uint admin_Type_ID, QTreeWidget *treeWidget)
{
    QSqlQuery query(__m_db);
    QString strSql = QString("insert into tb_auth_flag values(%1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)").arg(admin_Type_ID);

    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入权限标志sql语句: " << strSql;
    //初始化权限表数据
    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入权限标志失败:" << query.lastError();
        return DB_ERR;
    }

    int iRet = __ExecTransaction(admin_Type_ID, treeWidget);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库权限标志失败";
        return DB_ERR;
    }

    return NO_ERROR;
}

int CAuthSetting::EditAdminType(const uint admin_type_ID, QTreeWidget *treeWidget)
{
    //数据库权限标志表
    QStringList strListAuthFlag;
    strListAuthFlag.clear();
    strListAuthFlag<<QString("charge_list=0")<<QString("addr_count_list=0")<<QString("addr_detail=0") \
                  <<QString("collection_create_raw_Tx=0")<<QString("collection_broadcast=0")<<QString("big_account_list=0")<<QString("collection_log=0") \
                 <<QString("auto_withdraw=0")<<QString("manual_withdraw1=0")<<QString("manual_withdraw2=0")<<QString("user_assert_data=0") \
                <<QString("user_statements=0")<<QString("manual_withdraw_create_raw_Tx=0")<<QString("manual_withdraw_broadcast=0") \
               <<QString("withdraw_list=0")<<QString("admin_type_list=0")<<QString("add_admin_type=0")<<QString("admin_list=0") \
              <<QString("admin_log=0")<<QString("exchange_statements=0")<<QString("exchange_charge_daily=0") \
             <<QString("exchange_withdraw_daily=0")<<QString("add_coin=0")<<QString("airdrop_charge=0");

    QSqlQuery query(__m_db);
    QString strSql = QString("update tb_auth_flag set ");
    for (int i = 0; i < strListAuthFlag.count(); i++)
    {
        strSql = strSql + strListAuthFlag.at(i);
        if(i != strListAuthFlag.count() - 1)
        {
            strSql += QString(",");
        }
        else
        {
            strSql += QString(" where admin_type_id = %1").arg(admin_type_ID);
        }
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "插入权限标志sql语句: " << strSql;
    //初始化权限表数据
    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新权限标志全部为零失败:" << query.lastError();
        return DB_ERR;
    }

    int iRet = __ExecTransaction(admin_type_ID, treeWidget);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "编辑权限标志失败";
        return DB_ERR;
    }

    return NO_ERROR;
}

int CAuthSetting::DisplayAuth(const uint uAdminTypeID, QTreeWidget *treeWidget)
{
    QTreeWidgetItemIterator iter(treeWidget);
    QSqlQuery query(__m_db);
    QString strSql = QString("select * from tb_auth_flag where admin_type_id = %1 ").arg(uAdminTypeID);
    vector<int> vctAuth;    //数据库中各个界面的按钮权限值
    int iflag = 0;
    //    QStringList strlistAuthToBinary;    //数据库中各个权限值,二进制表示

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询对应管理员类型ID的权限信息失败" << query.lastError();
        return DB_ERR;
    }
    if (query.next())
    {
        for (int i = 0; i < 24; i++)
        {
            vctAuth.push_back(query.value(i).toInt());
        }
    }

    while (*iter)
    {
        unsigned int uFlag = 1;
        unsigned int uOffset = 0;
        //充值列表
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHARGELIST_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHARGELIST_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHARGELIST_RELEASE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHARGELIST_MESSAGE_NOTIFY)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //地址数量
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_ADDRLIST_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDRLIST_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDRLIST_IMPORT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDRLIST_DETAILS)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //地址详情
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_ADDRLISTDETAIL_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDRLISTDETAIL_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDRLISTDETAIL_RUNWATERDETAILS)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //创建未签名归集交易
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_COLLECTIONCREATERAWTX_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_COLLECTIONCREATERAWTX_CREATE)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //归集交易广播
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_COLLECTXBROADCAST_QUERY))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_COLLECTXBROADCAST_AFFIRMBROADCAST))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_COLLECTXBROADCAST_IMPORT)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //大地址监控
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_QUERY))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_ADDMONITORINGADDR))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_DELETESELECTEDADDR)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //归集日志
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_COLLECTLOG_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_COLLECTLOG_RESET)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //自动提币列表
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_WITHDRAW_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_WITHDRAW_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_WITHDRAW_STOPWITHDRAW)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //初审列表
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_FIRSTTRIAL_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_FIRSTTRIAL_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_FIRSTTRIAL_REJECT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_FIRSTTRIAL_AUDIT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERASSERT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERRUNWATER)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //复审列表
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_RECHECK_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHECK_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHECK_REJECT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHECK_CHECKUSERASSERT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHECK_CHECKUSERRUNWATER))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHECK_SELECTEDRECHECKPASS)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //用户资产数据
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_USERDATA_AFFIRM))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_USERDATA_RESET)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //用户流水
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_USERRUNWATER_AFFIRM))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_USERRUNWATER_FIRST))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_USERRUNWATER_RECHECK))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_USERRUNWATER_CHECKDATA)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //创建未签名提币交易
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_WITHDRAWCREATERAWTX_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_WITHDRAWCREATERAWTX_CREATE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_WITHDRAWCREATERAWTX_EXPORT)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //人工提币交易广播
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_MANUALCHECKBROADCAST_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_MANUALCHECKBROADCAST_IMPORTBROADCASTFILE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_MANUALCHECKBROADCAST_AFFIRMBROADCASTSELECTED)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //人工提币列表
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_PAGE_AUTH_WITHDRAWLIST_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_PAGE_AUTH_WITHDRAWLIST_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_PAGE_AUTH_WITHDRAWLIST_REJECT)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //管理员类型管理
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_ADMINTYPEMNG_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINTYPEMNG_ADD))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINTYPEMNG_EDIT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINTYPEMNG_FREEZE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINTYPEMNG_UNFREEZE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINTYPEMNG_DELETE)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //添加管理员类型
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_ADDADMINTYPE_ADD))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDADMINTYPE_EDIT)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //管理员列表
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_ADMINLIST_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINLIST_ADD))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINLIST_EDIT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINLIST_RESET))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINLIST_FREEZE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINLIST_UNFREEZE))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADMINLIST_DELETE)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //操作日志
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_OPERATIONLOG_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_OPERATIONLOG_FREEZE)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //平台流水
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_PLATFORMFLOW_QUERY))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_PLATFORMFLOW_EXPORTEXCEL)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //充值日报
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_RECHARGEDAILYPAPER_QUERY))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_RECHARGEDAILYPAPER_EXPORTEXCEL)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //提现日报
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_WITHDRAWDAILYPAPER_QUERY))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_WITHDRAWDAILYPAPER_EXPORTEXCEL)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
            uFlag++;
        }
        //添加币种
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_ADDCOINTYPE_SEARCH))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDCOINTYPE_ADD))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDCOINTYPE_EDIT))
            (*iter)->setCheckState(0, Qt::Checked);
        else
            uOffset++;
        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_ADDCOINTYPE_DELETE)){
            (*iter)->setCheckState(0, Qt::Checked);
            uFlag++;
        }
        else{
            uOffset++;
//            uFlag++;
        }
        //手动充值
//        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset] & INT_CTRL_AUTH_MANUALTOPUP_AFFIRMGENERATE))
//            (*iter)->setCheckState(0, Qt::Checked);
//        else
//            uOffset++;
//        if ((iflag == uFlag++) && (vctAuth[iflag - uOffset++] & INT_CTRL_AUTH_MANUALTOPUP_CANCEL))
//            (*iter)->setCheckState(0, Qt::Checked);
//        else
//            uOffset++;

        ++iflag;
        ++iter;
    }

    return NO_ERROR;
}

int CAuthSetting::SetUserBtnAuth(const uint uAdminTypeID, QPushButton *array[])
{
    QSqlQuery query(__m_db);
    QString strSql = QString("select * from tb_auth_flag where admin_type_id = %1 ").arg(uAdminTypeID);
    vector<int> vctAuth;    //数据库中各个界面的按钮权限值
    //    QStringList strlistAuthToBinary;    //数据库中各个权限值,二进制表示

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询对应管理员类型ID的权限信息失败" << query.lastError();
        return DB_ERR;
    }
    if (query.next())
    {
        for (int i = 1; i < 24; i++)
        {
            vctAuth.push_back(query.value(i).toInt());
        }
    }

    for (uint i = 0; i < vctAuth.size(); i++)
    {
        uint uFlag = 0;
        //充值列表1
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_RECHARGELIST_SEARCH)
                array[0]->setEnabled(true);
            else{
                array[0]->setEnabled(false);
            }
            if (vctAuth[i] & INT_CTRL_AUTH_RECHARGELIST_RESET)
                array[1]->setEnabled(true);
            else{
                array[1]->setEnabled(false);
            }
            if (vctAuth[i] & INT_CTRL_AUTH_RECHARGELIST_RELEASE)
                array[2]->setEnabled(true);
            else{
                array[2]->setEnabled(false);
            }
            if (vctAuth[i] & INT_CTRL_AUTH_RECHARGELIST_MESSAGE_NOTIFY)
                array[3]->setEnabled(true);
            else{
                array[3]->setEnabled(false);
            }
        }
        //地址数量2
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLIST_SEARCH)
                array[4]->setEnabled(true);
            else
                array[4]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLIST_RESET)
                array[5]->setEnabled(true);
            else
                array[5]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLIST_IMPORT)
                array[6]->setEnabled(true);
            else
                array[6]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLIST_DETAILS)
                array[7]->setEnabled(true);
            else
                array[7]->setEnabled(false);
        }
        //地址详情3
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLISTDETAIL_SEARCH)
                array[8]->setEnabled(true);
            else
                array[8]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLISTDETAIL_RESET)
                array[9]->setEnabled(true);
            else
                array[9]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDRLISTDETAIL_RUNWATERDETAILS)
                array[10]->setEnabled(true);
            else
                array[10]->setEnabled(false);
        }
        //创建未签名归集交易4
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTIONCREATERAWTX_SEARCH)
                array[11]->setEnabled(true);
            else
                array[11]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTIONCREATERAWTX_CREATE)
                array[12]->setEnabled(true);
            else
                array[12]->setEnabled(false);
        }
        //归集交易广播5
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTXBROADCAST_QUERY)
                array[13]->setEnabled(true);
            else
                array[13]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTXBROADCAST_AFFIRMBROADCAST)
                array[14]->setEnabled(true);
            else
                array[14]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTXBROADCAST_IMPORT)
                array[15]->setEnabled(true);
            else
                array[15]->setEnabled(false);
        }
        //大地址监控6
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_QUERY)
                array[16]->setEnabled(true);
            else
                array[16]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_RESET)
                array[17]->setEnabled(true);
            else
                array[17]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_ADDMONITORINGADDR)
                array[18]->setEnabled(true);
            else
                array[18]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_BIGACCOUNTMONITORING_DELETESELECTEDADDR)
                array[19]->setEnabled(true);
            else
                array[19]->setEnabled(false);
        }
        //归集日志7
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTLOG_SEARCH)
                array[20]->setEnabled(true);
            else
                array[20]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_COLLECTLOG_RESET)
                array[21]->setEnabled(true);
            else
                array[21]->setEnabled(false);
        }
        //自动提现列表8
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAW_SEARCH)
                array[22]->setEnabled(true);
            else
                array[22]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAW_RESET)
                array[23]->setEnabled(true);
            else
                array[23]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAW_STOPWITHDRAW)
                array[24]->setEnabled(true);
            else
                array[24]->setEnabled(false);
        }
        //初审列表9
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_FIRSTTRIAL_SEARCH)
                array[25]->setEnabled(true);
            else
                array[25]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_FIRSTTRIAL_RESET)
                array[26]->setEnabled(true);
            else
                array[26]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_FIRSTTRIAL_REJECT)
                array[27]->setEnabled(true);
            else
                array[27]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_FIRSTTRIAL_AUDIT)
                array[28]->setEnabled(true);
            else
                array[28]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERASSERT)
                array[29]->setEnabled(true);
            else
                array[29]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERRUNWATER)
                array[30]->setEnabled(true);
            else
                array[30]->setEnabled(false);
        }
        //复审列表10
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_RECHECK_SEARCH)
                array[31]->setEnabled(true);
            else
                array[31]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_RECHECK_RESET)
                array[32]->setEnabled(true);
            else
                array[32]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_RECHECK_REJECT)
                array[33]->setEnabled(true);
            else
                array[33]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_RECHECK_CHECKUSERASSERT)
                array[34]->setEnabled(true);
            else
                array[34]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_RECHECK_CHECKUSERRUNWATER)
                array[35]->setEnabled(true);
            else
                array[35]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_RECHECK_SELECTEDRECHECKPASS)
                array[36]->setEnabled(true);
            else
                array[36]->setEnabled(false);
        }
        //用户资产数据11
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_USERDATA_AFFIRM)
                array[37]->setEnabled(true);
            else
                array[37]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_USERDATA_RESET)
                array[38]->setEnabled(true);
            else
                array[38]->setEnabled(false);
        }
        //用户流水12
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_USERRUNWATER_AFFIRM)
                array[39]->setEnabled(true);
            else
                array[39]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_USERRUNWATER_FIRST)
                array[40]->setEnabled(true);
            else
                array[40]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_USERRUNWATER_RECHECK)
                array[41]->setEnabled(true);
            else
                array[41]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_USERRUNWATER_CHECKDATA)
                array[42]->setEnabled(true);
            else
                array[42]->setEnabled(false);
        }
        //创建未签名提币交易13
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAWCREATERAWTX_SEARCH)
                array[43]->setEnabled(true);
            else
                array[43]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAWCREATERAWTX_CREATE)
                array[44]->setEnabled(true);
            else
                array[44]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAWCREATERAWTX_EXPORT)
                array[45]->setEnabled(true);
            else
                array[45]->setEnabled(false);
        }
        //人工提币交易广播14
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_MANUALCHECKBROADCAST_SEARCH)
                array[46]->setEnabled(true);
            else
                array[46]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_MANUALCHECKBROADCAST_IMPORTBROADCASTFILE)
                array[47]->setEnabled(true);
            else
                array[47]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_MANUALCHECKBROADCAST_AFFIRMBROADCASTSELECTED)
                array[48]->setEnabled(true);
            else
                array[48]->setEnabled(false);
        }
        //人工提币列表15
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_PAGE_AUTH_WITHDRAWLIST_SEARCH)
                array[49]->setEnabled(true);
            else
                array[49]->setEnabled(false);
            if (vctAuth[i] & INT_PAGE_AUTH_WITHDRAWLIST_RESET)
                array[50]->setEnabled(true);
            else
                array[50]->setEnabled(false);
            if (vctAuth[i] & INT_PAGE_AUTH_WITHDRAWLIST_REJECT)
                array[51]->setEnabled(true);
            else
                array[51]->setEnabled(false);
        }
        //管理员类型管理16
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINTYPEMNG_SEARCH)
                array[52]->setEnabled(true);
            else
                array[52]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINTYPEMNG_ADD)
                array[53]->setEnabled(true);
            else
                array[53]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINTYPEMNG_EDIT)
                array[54]->setEnabled(true);
            else
                array[54]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINTYPEMNG_FREEZE)
                array[55]->setEnabled(true);
            else
                array[55]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINTYPEMNG_UNFREEZE)
                array[56]->setEnabled(true);
            else
                array[56]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINTYPEMNG_DELETE)
                array[57]->setEnabled(true);
            else
                array[57]->setEnabled(false);
        }
        //添加管理员类型17
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_ADDADMINTYPE_ADD)
                array[58]->setEnabled(true);
            else
                array[58]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDADMINTYPE_EDIT)
                array[59]->setEnabled(true);
            else
                array[59]->setEnabled(false);
        }
        //管理员列表18
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_SEARCH)
                array[60]->setEnabled(true);
            else
                array[60]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_ADD)
                array[61]->setEnabled(true);
            else
                array[61]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_EDIT)
                array[62]->setEnabled(true);
            else
                array[62]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_RESET)
                array[63]->setEnabled(true);
            else
                array[63]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_FREEZE)
                array[64]->setEnabled(true);
            else
                array[64]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_UNFREEZE)
                array[65]->setEnabled(true);
            else
                array[65]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADMINLIST_DELETE)
                array[66]->setEnabled(true);
            else
                array[66]->setEnabled(false);
        }
        //操作日志19
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_OPERATIONLOG_SEARCH)
                array[67]->setEnabled(true);
            else
                array[67]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_OPERATIONLOG_FREEZE)
                array[68]->setEnabled(true);
            else
                array[68]->setEnabled(false);
        }
        //平台流水20
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_PLATFORMFLOW_QUERY)
                array[69]->setEnabled(true);
            else
                array[69]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_PLATFORMFLOW_EXPORTEXCEL)
                array[70]->setEnabled(true);
            else
                array[70]->setEnabled(false);
        }
        //充值日报21
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_RECHARGEDAILYPAPER_QUERY)
                array[71]->setEnabled(true);
            else
                array[71]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_RECHARGEDAILYPAPER_EXPORTEXCEL)
                array[72]->setEnabled(true);
            else
                array[72]->setEnabled(false);
        }
        //提现日报22
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAWDAILYPAPER_QUERY)
                array[73]->setEnabled(true);
            else
                array[73]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_WITHDRAWDAILYPAPER_EXPORTEXCEL)
                array[74]->setEnabled(true);
            else
                array[74]->setEnabled(false);
        }
        //添加币种23
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_ADDCOINTYPE_SEARCH)
                array[75]->setEnabled(true);
            else
                array[75]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDCOINTYPE_ADD)
                array[76]->setEnabled(true);
            else
                array[76]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDCOINTYPE_EDIT)
                array[77]->setEnabled(true);
            else
                array[77]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_ADDCOINTYPE_DELETE)
                array[78]->setEnabled(true);
            else
                array[78]->setEnabled(false);
        }
        //手动充值24
        if (i == uFlag++)
        {
            if (vctAuth[i] & INT_CTRL_AUTH_MANUALTOPUP_AFFIRMGENERATE)
                array[79]->setEnabled(true);
            else
                array[79]->setEnabled(false);
            if (vctAuth[i] & INT_CTRL_AUTH_MANUALTOPUP_CANCEL)
                array[80]->setEnabled(true);
            else
                array[80]->setEnabled(false);
        }
    }

    return NO_ERROR;
}
