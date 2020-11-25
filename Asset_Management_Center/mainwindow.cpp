#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "user_info.h"
#include <QDebug>
#include <list>
#include <QPushButton>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <qdesktopservices.h>

extern bool  g_bIsTipsNeeded;  //是否弹框标志
uint CMainWindow::__m_uFlag = 0;
//标题栏的长度
const static int pos_min_x = 0;
const static int pos_max_x = 1920-40;
const static int pos_min_y = 0;
const static int pos_max_y = 30;

class CAMMain;
map<UINT_HANDLE, CAMMain*>  g_mapHandles;    //业务处理句柄



//QProcess *g_ptr_qprocess_AMWatchDog = NULL; //看门狗
//const QString  gc_qstr_AMWatchDog = "AMWatchDog.exe";


CMainWindow::CMainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化页面
    ui->stackedWidget->setCurrentIndex(0);
    ui->btn_ManualWithdrawBroadcast_Search->setVisible(false); //隐藏 "搜索" 按钮    2019-06-24 yqq

    //去掉窗口标题栏
    this->setWindowFlags(Qt::FramelessWindowHint);

    //__m_pAuthSet = new CAuthSetting();
    __m_pAuthSet = NULL;   //在Mainwindow的 Init() 中创建

    //初始化所有成员变量(指针)  防止野指针(析构的时候,程序崩溃)

    //1.充值页面
    __m_pPageChargeList = NULL; //充值页面业务逻辑处理
//    __m_pDlg_pageChargeList_ShortMsgSetting = NULL; //充值列表中 短信设置对话框
    __m_pDlg_auditAndReject = NULL;

    __m_pPageAddrList  = NULL; //地址列表业务逻辑
    __m_pImportAddr = NULL;   //地址列表导入地址对话框

    __m_pAddCoin = NULL;  //添加币种业务逻辑
    __m_pAddCoinDlg = NULL;//添加币种对话框
    __m_pDeleteCoinDlg = NULL;//删除币种对话框

    __m_pExchangeStatements = NULL;//平台财务流水

    __m_pChargeDaily = NULL;//充值日报业务逻辑

    __m_pWithdrawDaily = NULL;//提现业务日报

    __m_pAdminTypeList = NULL;//管理员类型列表

    __m_pAddAdminType = NULL;//添加管理员类型
    //__m_pAuthSet = NULL;//权限设置

    __m_pAdminList = NULL;//管理员列表
    __m_AddAdmin = NULL;//添加管理员
    __m_ResetPwd = NULL;//重置密码

    __m_pOperateLog = NULL;//操作日志

    __m_pWithdraw = NULL;     //提现业务处理
    __m_pCollection = NULL;    //归集业务处理

    __m_getLoginUser = NULL;    //
}

CMainWindow::~CMainWindow()
{
    delete ui;

//    if(NULL != __m_pDlg_pageChargeList_ShortMsgSetting)
//    {
//        delete __m_pDlg_pageChargeList_ShortMsgSetting;
//        __m_pDlg_pageChargeList_ShortMsgSetting = NULL;
//    }
    if (NULL != __m_pDlg_auditAndReject)
    {
        delete __m_pDlg_auditAndReject;
        __m_pDlg_auditAndReject = NULL;
    }
    if(NULL !=__m_pPageChargeList)
    {
        delete __m_pPageChargeList;
        __m_pPageChargeList = NULL;
    }
    if (NULL !=__m_pPageAddrList)
    {
        delete __m_pPageAddrList;
        __m_pPageAddrList = NULL;
    }
    if (NULL !=__m_pImportAddr)
    {
        delete __m_pImportAddr;
        __m_pImportAddr = NULL;
    }
    if (NULL !=__m_pAddCoin)
    {
        delete __m_pAddCoin;
        __m_pAddCoin = NULL;
    }
    if (NULL !=__m_pAddCoinDlg)
    {
        delete __m_pAddCoinDlg;
        __m_pAddCoinDlg = NULL;
    }
    if (NULL !=__m_pDeleteCoinDlg)
    {
        delete __m_pDeleteCoinDlg;
        __m_pDeleteCoinDlg = NULL;
    }
    if (NULL !=__m_pExchangeStatements)
    {
        delete __m_pExchangeStatements;
        __m_pExchangeStatements = NULL;
    }
    if (NULL !=__m_pChargeDaily)
    {
        delete __m_pChargeDaily;
        __m_pChargeDaily = NULL;
    }
    if (NULL !=__m_pWithdrawDaily)
    {
        delete __m_pWithdrawDaily;
        __m_pWithdrawDaily = NULL;
    }
    if (NULL !=__m_pAdminTypeList)
    {
        delete __m_pAdminTypeList;
        __m_pAdminTypeList = NULL;
    }
    if (NULL !=__m_pAddAdminType)
    {
        delete __m_pAddAdminType;
        __m_pAddAdminType = NULL;
    }
    if (NULL !=__m_pAdminList)
    {
        delete __m_pAdminList;
        __m_pAdminList = NULL;
    }
    if (NULL !=__m_AddAdmin)
    {
        delete __m_AddAdmin;
        __m_AddAdmin = NULL;
    }
    if (NULL !=__m_ResetPwd)
    {
        delete __m_ResetPwd;
        __m_ResetPwd = NULL;
    }
    if (NULL !=__m_pOperateLog)
    {
        delete __m_pOperateLog;
        __m_pOperateLog = NULL;
    }
    if (NULL !=__m_pAuthSet)
    {
        delete __m_pAuthSet;
        __m_pAuthSet = NULL;
    }
    if (NULL != __m_getLoginUser)
    {
        delete __m_getLoginUser;
        __m_getLoginUser = NULL;
    }
}

//初始化
int CMainWindow::Init()
{
    int iRet = -1;

    //初始化权限设置对象指针
    __m_pAuthSet = new CAuthSetting();
    iRet = __m_pAuthSet->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "权限设置: 初始化数据库失败";
        return iRet;
    }

    //初始化获取登录用户状态
    __m_getLoginUser = new CGetLoginUser(QString("mainwindow"));

    //初始化业务处理对象
    iRet = __InitHandles();
    if(0 != iRet)
    {
        return iRet;
    }


    __InsertLoginLog();             //登录操作日志写入数据库

    __InitSignalBinding();          //初始化信号与槽绑定

    __InitNavTree();                //初始化导航框

    __InitAuth();                   //初始化权限选中框

    __InitAllTableWidget();         //初始化充值列表表格

    __BindinBtn();                  //初始化标题栏的按钮绑定

    __InitCoinType_cbx();           //初始化所有界面的币种选择框的下拉项

    __initMultiPage();              //初始化分页组件并绑定相关信号槽

    //初始化标题栏
    QString strTitle;
    strTitle = G_STR_EXE_VERSION;
    ui->label_3->setText(strTitle);
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strTitle;

    return NO_ERROR;
}

void CMainWindow::__InsertLoginLog()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("登录");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "操作插入操作日志表失败";
    }

    //登录成功 设置登录的账号相关的按钮权限
    uint uAdminTypeID = CUserInfo::loginUser().AdminTypeID().toUInt();
    QPushButton *arrAllBtn[79] = {ui->btn_ChargeList_Search, ui->btn_ChargeList_Reset, ui->btn_ChargeList_Release, ui->btn_ChargeList_Audit,
                                  ui->btn_AddrCountList_Search, ui->btn_AddrCountList_Reset, ui->btn_AddrCountList_ImportAddr, ui->btn_AddrCountList_LookDetail,
                                  ui->btn_AddrDetail_Search, ui->btn_AddrDetail_Reset, ui->btn_RunWaterDetail, ui->btn_CollectionCreateRawTx_Search,
                                  ui->btn_CollectionCreateRawTx_CreateRawTx, ui->btn_CollectionBroadcast_Search, ui->btn_CollectionBroadcast_Ok,
                                  ui->btn_CollectionBroadcast_ImportRawTx, ui->btn_BigAccountList_Search, ui->btn_BigAccountList_Reset, ui->btn_BigAccountList_AddAddr,
                                  ui->btn_BigAccountList_Delete, ui->btn_CollectionLog_Search, ui->btn_CollectionLog_Reset, ui->btn_AutoWithdraw_Search,
                                  ui->btn_AutoWithdraw_Reset, ui->btn_AutoWithdraw_Reject, ui->btn_ManualWithdraw1_Search, ui->btn_ManualWithdraw1_Reset, ui->btn_ManualWithdraw1_Reject,
                                  ui->btn_ManualWithdraw1_Audit, ui->btn_ManualWithdraw1_CheckAsset, ui->btn_ManualWithdraw1_CheckFlow, ui->btn_ManualWithdraw2_Search,
                                  ui->btn_ManualWithdraw2_Reset, ui->btn_ManualWithdraw2_Reject, ui->btn_ManualWithdraw2_CheckAsset, ui->btn_ManualWithdraw2_CheckFlow,
                                  ui->btn_ManualWithdraw2_Audit, ui->btn_UserAssertData_Ok, ui->btn_UserAssertData_Reset, ui->btn_UserStatements_Search,
                                  ui->btn_UserStatements_JumpToManualWithdraw1, ui->btn_UserStatements_JumpToManualWithdraw2, ui->btn_UserStatements_JumpToUserAssetData,
                                  ui->btn_ManualWithdrawCreateRawTx_Search, ui->btn_ManualWithdrawCreateRawTx_Ok, ui->btn_ManualWithdrawCreateRawTx_Export,
                                  ui->btn_ManualWithdrawBroadcast_Search, ui->btn_ManualWithdrawBroadcast_ImportRawTx, ui->btn_ManualWithdrawBroadcast_Ok,
                                  ui->btn_ManualWithdrawList_Search, ui->btn_ManualWithdrawList_Reset, ui->btn_ManualWithdrawList_Reject, ui->btn_AdminTypeList_Search,
                                  ui->btn_AdminTypeList_AddAdmin, ui->btn_AdminTypeList_Edit, ui->btn_AdminTypeList_Freeze, ui->btn_AdminTypeList_Unfreeze,
                                  ui->btn_AdminTypeList_Delete, ui->btn_AddAdminType_Add, ui->btn_AddAdminType_Edit, ui->btn_AdminList_Search, ui->btn_AdminList_AddAdmin,
                                  ui->btn_AdminList_Edit, ui->btn_AdminList_ResetPwd, ui->btn_AdminList_Freeze, ui->btn_AdminList_Unfreeze, ui->btn_AdminList_Delete,
                                  ui->btn_AdminLog_Search, ui->btn_AdminLog_Freeze, ui->btn_ExchangeStatements_Search, ui->btn_ExchangeStatements_Export,
                                  ui->btn_ExchangeChargeDaily_Search, ui->btn_ExchangeChargeDaily_Export, ui->btn_ExchangeWithdrawDaily_Search, ui->btn_ExchangeWithdrawDaily_Export,
                                  ui->btn_AddCoin_Search, ui->btn_AddCoin_Ok, ui->btn_AddCoin_Edit, ui->btn_AddCoin_Delete};

    iRet = __m_pAuthSet->SetUserBtnAuth(uAdminTypeID, arrAllBtn);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "获取数据库中对应登录账号的权限失败";
        return;
    }
}

//初始化导航列表
void CMainWindow::__InitNavTree()
{
    if (0 == __m_uFlag)     //白色
    {
        ui->navlistview->setIcoColorBg(false);
        ui->navlistview->setColorLine(QColor(222, 231, 242));
        ui->navlistview->setColorBg(QColor(232, 241, 252), QColor(180, 210, 240), QColor(222, 231, 242, 70));
        ui->navlistview->setColorText(QColor(65, 65, 65), QColor(65, 65, 65), QColor(65, 65, 65));
    }
    if (1 == __m_uFlag)     //黑色
    {
        ui->navlistview->setIcoColorBg(false);
        ui->navlistview->setColorLine(QColor(50, 50, 50));
        ui->navlistview->setColorBg(QColor(50, 50, 50), QColor(9, 71, 113), QColor(70, 70, 70, 150));
        ui->navlistview->setColorText(QColor(220, 220, 220), QColor(220, 220, 220), QColor(220, 220, 220));
    }

    //设置数据方式
    QStringList strLstNavTree;
    uint uRow = 0;
    uint uPage = 0;

    strLstNavTree.append(tr("地址管理||0|")); uRow++;
    strLstNavTree.append(tr("充值列表|地址管理|0|")); __m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("地址数量|地址管理|0|")); __m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("地址详情|地址管理|0|")); __m_mapPage.insert(make_pair(uRow++, uPage++));
    //    strLstNavTree.append(tr("地址流水|地址管理|0|")); __m_mapPage.insert(make_pair(uRow++, uPage++));

    strLstNavTree.append(tr("归集管理||0|")); uRow++;
    //    strLstNavTree.append(tr("待归集列表|归集管理|0|")); __m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("创建未签名归集交易|归集管理|0|")); __m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("归集交易广播|归集管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("大地址监控|归集管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("归集日志|归集管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));

    strLstNavTree.append(tr("自动提币管理||0|"));uRow++;
    strLstNavTree.append(tr("自动提币列表|自动提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));


    strLstNavTree.append(tr("人工提币管理||0|"));uRow++;
    strLstNavTree.append(tr("初审列表|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("复审列表|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    //    strLstNavTree.append(tr("复审-用户资产数据|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    //    strLstNavTree.append(tr("复审-用户资产流水|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("创建未签名提币交易|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("人工提币交易广播|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("人工提币列表|人工提币管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));

    strLstNavTree.append(tr("权限管理||0|"));uRow++;
    strLstNavTree.append(tr("管理员类型管理|权限管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("添加管理员类型|权限管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("管理员列表|权限管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("操作日志|权限管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));

    strLstNavTree.append(tr("平台财务管理||0|"));uRow++;
    strLstNavTree.append(tr("平台流水|平台财务管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("充值日报|平台财务管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    strLstNavTree.append(tr("提现日报|平台财务管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));

    strLstNavTree.append(tr("系统管理||0|"));uRow++;
    strLstNavTree.append(tr("添加币种|系统管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));
    //    strLstNavTree.append(tr("手动充值|系统管理|0|"));__m_mapPage.insert(make_pair(uRow++, uPage++));

    //设置页面索引 与  导航栏选项  的对应关系,
    //注意: ui文件中的页面排序必须与 strLstNavTree 的顺序一致
    if((int)__m_mapPage.size() != ui->stackedWidget->count() - 4)
        return;

    ui->navlistview->setData(strLstNavTree);
}

//初始化权限选中框
void CMainWindow::__InitAuth()
{
    connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), __m_pAuthSet, SLOT(OnTreeItemChanged(QTreeWidgetItem*)));
    __m_pAuthSet->InitAuthlist(ui->treeWidget);
}

//初始化信号与槽绑定
void CMainWindow::__InitSignalBinding()
{
    //绑定清除输入框槽函数,左边的导航栏点击时初始化添加管理员类型界面
    connect(this, &CMainWindow::sendClearDisplay, this, &CMainWindow::onClearDisplay);

    //绑定显示用户资产页面信号槽
    connect(this, SIGNAL(Send_ShowAssetPage(QStringList)), this, SLOT(OnShowAssetPage(QStringList)), Qt::DirectConnection /*使用直连方式,防止瞬间多次请求*/);

    //----1.绑定充值列表中的  短信设置按钮消息
//    __m_pDlg_pageChargeList_ShortMsgSetting = new CShortMsgSettingDlg(this);
    __m_pDlg_auditAndReject = new audit_dlg(this);
//    connect(ui->btn_ChargeList_ShortMsgSetting, &QPushButton::clicked, __m_pDlg_pageChargeList_ShortMsgSetting, &CShortMsgSettingDlg::exec);
    connect(ui->btn_ChargeList_Search, &QPushButton::clicked,  this, &CMainWindow::page_ChargeList_OnSearch);
    connect(ui->btn_ChargeList_Reset, &QPushButton::clicked, this, &CMainWindow::page_ChargeList_OnReset);
    connect(ui->btn_ChargeList_Release, &QPushButton::clicked, this, &CMainWindow::page_ChargeList_OnRelease);
    connect(ui->btn_ChargeList_exportExcel, &QPushButton::clicked, this, &CMainWindow::page_ChargeList_OnExportExcel);
    connect(ui->tbw_ChargeList, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem*)));
    connect(ui->btn_ChargeList_Audit, &QPushButton::clicked, this, &CMainWindow::page_ChargeList_OnAudit);
//    connect(ui->btn_ChargeList_auditReject, &QPushButton::clicked, this, &CMainWindow::page_ChargeList_OnAuditReject);

    //----2.地址数量表
    __m_pImportAddr = new CImportAddrDlg(this);
    connect(ui->btn_AddrCountList_ImportAddr, &QPushButton::clicked, __m_pImportAddr, &CImportAddrDlg::exec);//1.地址列表 导入地址按钮
    connect(ui->btn_AddrCountList_Search, &QPushButton::clicked, this, &CMainWindow::page_AddrList_OnSearch);//2.地址列表搜索按钮
    connect(ui->btn_AddrCountList_Reset, &QPushButton::clicked, this, &CMainWindow::page_AddrList_OnReset);//3.地址列表重置按钮
    connect(ui->btn_AddrCountList_LookDetail, &QPushButton::clicked, this, &CMainWindow::page_AddrList_OnLookDetail);//4.地址列表查看详情按钮

    //----3.地址详情
    connect(ui->btn_AddrDetail_Search, &QPushButton::clicked, this, &CMainWindow::page_AddrDetail_OnSearch);//1.搜索按钮
    connect(ui->btn_AddrDetail_Reset, &QPushButton::clicked, this, &CMainWindow::page_AddrDetail_OnReset);//2.重置按钮
    connect(ui->btn_RunWaterDetail, &QPushButton::clicked, this, &CMainWindow::page_AddrList_OnRunWaterDetail);//3.流水详情按钮

    //----4.平台财务流水
    connect(ui->btn_ExchangeStatements_Search, &QPushButton::clicked, this, &CMainWindow::page_ExchangeStatements_OnSearch);//1.查询按钮
    connect(ui->btn_ExchangeStatements_Export, &QPushButton::clicked, this, &CMainWindow::page_ExchangeStatements_OutPutExcel);//2.导出Excel

    //----5.充值日报
    connect(ui->btn_ExchangeChargeDaily_Search, &QPushButton::clicked, this, &CMainWindow::page_ChargeDaily_OnSearch);//1.查询按钮
    connect(ui->btn_ExchangeChargeDaily_Export, &QPushButton::clicked, this, &CMainWindow::page_ChargeDaily_OutPutExcel);//2.导出Excel

    //----6.提现日报
    connect(ui->btn_ExchangeWithdrawDaily_Search, &QPushButton::clicked, this, &CMainWindow::page_WithdrawDaily_OnSearch);//1.查询按钮
    connect(ui->btn_ExchangeWithdrawDaily_Export, &QPushButton::clicked, this, &CMainWindow::page_WithdrawDaily_OutPutExcel);//2.导出Excel

    //----7.管理员类型列表
    connect(ui->btn_AdminTypeList_Search, &QPushButton::clicked, this, &CMainWindow::page_AdminTypeList_OnSearch);
    connect(ui->btn_AdminTypeList_AddAdmin, &QPushButton::clicked, this, &CMainWindow::page_AdminTypeList_OnAddAdminType);
    connect(ui->btn_AdminTypeList_Edit, &QPushButton::clicked, this, &CMainWindow::page_AdminTypeList_OnEdit);
    connect(ui->btn_AdminTypeList_Freeze, &QPushButton::clicked, this, &CMainWindow::page_AdminTypeList_OnFreeze);
    connect(ui->btn_AdminTypeList_Unfreeze, &QPushButton::clicked, this, &CMainWindow::page_AdminTypeList_OnUnfreeze);
    connect(ui->btn_AdminTypeList_Delete, &QPushButton::clicked, this, &CMainWindow::page_AdminTypeList_OnDelete);
    //----8.添加管理员类型
    connect(ui->btn_AddAdminType_Add, &QPushButton::clicked, this, &CMainWindow::page_AddAdminType_OnAdd);
    connect(ui->btn_AddAdminType_Edit, &QPushButton::clicked, this, &CMainWindow::page_AddAdminType_OnEdit);
    //----9.管理员列表
    __m_AddAdmin = new CAddAdminDlg(this);
    __m_ResetPwd = new CResetPwdDlg(this);
    //添加成功管理员类型之后发送信号
    connect(this, SIGNAL(Send_AddAdminType(QString)), __m_AddAdmin, SLOT(OnAddAdminTypeCbx(QString)), Qt::DirectConnection);
    //编辑成功管理员类型之后发送信号
    connect(this, SIGNAL(Send_UpdateAdminType(QStringList)), __m_AddAdmin, SLOT(OnEditAdminTypeCbx(QStringList)), Qt::DirectConnection);
    connect(ui->btn_AdminList_Search, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnSearch);
    connect(ui->btn_AdminList_AddAdmin, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnAddAdmin);
    connect(ui->btn_AdminList_Edit, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnEdit);
    connect(ui->btn_AdminList_ResetPwd, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnResetPwd);
    connect(ui->btn_AdminList_Freeze, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnFreeze);
    connect(ui->btn_AdminList_Delete, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnDelete);
    connect(ui->btn_AdminList_Unfreeze, &QPushButton::clicked, this, &CMainWindow::page_AdminList_OnUnfreeze);
    //----10.操作日志
    connect(ui->btn_AdminLog_Search, &QPushButton::clicked, this, &CMainWindow::page_OperateLog_OnSearch);
    connect(ui->btn_AdminLog_Freeze, &QPushButton::clicked, this, &CMainWindow::page_OperateLog_OnFreeze);

    //----11.添加币种表
    __m_pAddCoinDlg = new CAddCoinDlg(this);
    __m_pDeleteCoinDlg = new CDeleteCoinDlg(this);
    //添加币种成功之后,更新主界面上的所有的币种选择框
    connect(__m_pAddCoinDlg, SIGNAL(Send_AddCoinType(QString)), this, SLOT(OnAddCoinType(QString)));
    //编辑币种成功之后,更新主界面上的所有的币种选择框
    connect(__m_pAddCoinDlg, SIGNAL(Send_UpdateCoinType(QStringList)), this, SLOT(OnEditOrDeleteCoinType(QStringList)));
    //删除币种成功之后,更新主界面上的所有的币种选择框
    connect(__m_pDeleteCoinDlg, SIGNAL(Send_UpdateCoinType(QStringList)), this, SLOT(OnEditOrDeleteCoinType(QStringList)));
    connect(ui->btn_AddCoin_Ok, &QPushButton::clicked, this, &CMainWindow::page_AddCoin_OnAdd);//1.添加币种按钮
    connect(ui->btn_AddCoin_Search, &QPushButton::clicked, this, &CMainWindow::page_AddCoin_OnSearch);//2.查询按钮
    connect(ui->btn_AddCoin_Edit, &QPushButton::clicked, this, &CMainWindow::page_AddCoin_OnEdit);//3.编辑
    connect(ui->btn_AddCoin_Delete, &QPushButton::clicked, this, &CMainWindow::page_AddCoin_OnDelete);//4.删除币种按钮
    connect(ui->tbw_AddCoin, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem*)));


    //------人工审核提币
    //绑定 人工审核提币-创建交易-全选按钮
    connect(ui->checkBox_ManualWithdrawCreateRawTx_SelAll, &QCheckBox::clicked, this, &CMainWindow::page_ManualWithdrawCreateRawTx_OnSelAll);


    //绑定"人工提币-初审-搜索"按钮消息
    connect(ui->btn_ManualWithdraw1_Search, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw1_OnSearch);

    //绑定"人工提币-初审-重置"按钮消息
    connect(ui->btn_ManualWithdraw1_Reset, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw1_OnReset);

    //绑定"人工提币-初审-审核通过"按钮消息
    connect(ui->btn_ManualWithdraw1_Reject, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw1_OnReject);

    //绑定"人工提币-初审-审核驳回"按钮消息
    connect(ui->btn_ManualWithdraw1_Audit, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw1_OnAudit);

    //绑定"人工提币-初审-查看用户资产"按钮消息
    connect(ui->btn_ManualWithdraw1_CheckAsset, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw1_OnCheckAsset);

    //绑定"人工提币-初审-查看用户流水"按钮消息
    connect(ui->btn_ManualWithdraw1_CheckFlow, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw1_OnCheckFlow);


    //复审列表
    connect(ui->btn_ManualWithdraw2_Search, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw2_OnSearch);
    connect(ui->btn_ManualWithdraw2_Reset, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw2_OnReset);
    connect(ui->btn_ManualWithdraw2_Reject, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw2_OnReject);
    connect(ui->btn_ManualWithdraw2_Audit, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw2_OnAudit);
    connect(ui->btn_ManualWithdraw2_CheckAsset, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw2_OnCheckAsset);
    connect(ui->btn_ManualWithdraw2_CheckFlow, &QPushButton::clicked, this, &CMainWindow::page_ManulWithdraw2_OnCheckFlow);

    //自动提币列表
    connect(ui->btn_AutoWithdraw_Reject, &QPushButton::clicked, this, &CMainWindow::page_AutoWithdraw_OnReject);
    connect(ui->btn_AutoWithdraw_Search, &QPushButton::clicked, this, &CMainWindow::page_AutoWithdraw_OnSearch);
    connect(ui->btn_AutoWithdraw_Reset, &QPushButton::clicked, this, &CMainWindow::page_AutoWithdraw_OnReset);
    connect(ui->btn_AutoWithdraw_exportExcel, &QPushButton::clicked, this, &CMainWindow::page_AutoWithdraw_OnExportExcel);
    connect(ui->tbw_AutoWithdraw, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem*)));

    //绑定"查看用户资产-确定"按钮消息
    connect(ui->btn_UserAssertData_Ok, &QPushButton::clicked, this, &CMainWindow::page_UserAssetData_OnOK);
    connect(ui->btn_UserAssertData_Reset, &QPushButton::clicked, this, &CMainWindow::page_UserAssetData_OnReset);
    connect(ui->btn_UserAssertData_Audit, &QPushButton::clicked, this, &CMainWindow::page_UserAssetData_OnAudit);
    connect(ui->btn_UserAssertData_secAudit, &QPushButton::clicked, this, &CMainWindow::page_UserAssetData_OnSecAudit);

    //绑定"查看用户流水-查询"按钮消息
    connect(ui->btn_UserStatements_Search, &QPushButton::clicked, this, &CMainWindow::page_UserFlowData_OnSearch);

    //绑定"查看用户流水-初审"按钮消息
    connect(ui->btn_UserStatements_JumpToManualWithdraw1, &QPushButton::clicked, this, &CMainWindow::page_UserFlowData_OnJumptoManualWithdraw1);

    //绑定"查看用户流水-复审"按钮消息
    connect(ui->btn_UserStatements_JumpToManualWithdraw2, &QPushButton::clicked, this, &CMainWindow::page_UserFlowData_OnJumptoManualWithdraw2);

    //绑定"查看用户流水-复审"按钮消息
    connect(ui->btn_UserStatements_JumpToUserAssetData, &QPushButton::clicked, this, &CMainWindow::page_UserFlowData_OnJumptoAssetData);


    //查询某个币种(某个地址)所有的 已经复审通过的  提币订单数据，重复绑定了
    //  connect(ui->btn_ManualWithdrawCreateRawTx_Search, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawCreateRawTx_OnSearch);

    //将已选中的订单,创建未签名的交易
    connect(ui->btn_ManualWithdrawCreateRawTx_Ok, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawCreateRawTx_OnOk);


    //创建未签名提币交易-导出文件
    connect(ui->btn_ManualWithdrawCreateRawTx_Export, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawCreateRawTx_OnExport);

    //绑定"创建未签名离线交易-搜索"按钮消息
    connect(ui->btn_ManualWithdrawCreateRawTx_Search, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawCreateRawTx_OnSearch);

    //人工提币列表
    connect(ui->btn_ManualWithdrawList_Search, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawList_OnSearch);
    connect(ui->btn_ManualWithdrawList_Reset, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawList_OnReset);
    connect(ui->btn_ManualWithdrawList_Reject, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawList_OnReject);
    connect(ui->tbw_WithdrawList, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem*)));
    //处理异常订单 按钮  TODO:临时处理usdt和htdf的那些没有交易id的订单,后面链没问题之后删除   20190601 xc
    connect(ui->btn_ManualWithdrawList_Release, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawList_Release);
    connect(ui->btn_WithdrawList_exportExcel, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawList_OnExportExcel);

    //人工提币, 广播页面, 全选按钮
    connect(ui->checkBox_ManualWithdrawBroadcast_SelAll, &QCheckBox::clicked, this, &CMainWindow::page_ManualWithdrawBroadcast_OnSelAll);

    //人工提币, 搜索待广播交易
    connect(ui->btn_ManualWithdrawBroadcast_Search, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawBroadcast_OnSearch);

    //人工提币, 导入已签名交易文件
    connect(ui->btn_ManualWithdrawBroadcast_ImportRawTx, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawBroadcast_OnImport);


    //人工提币, 广播已签名交易
    connect(ui->btn_ManualWithdrawBroadcast_Ok, &QPushButton::clicked, this, &CMainWindow::page_ManualWithdrawBroadcast_OnBroadcast);


    //-----------------------------------------------------------
    //-------------归集----------

    //绑定 "创建未签名归集交易-全选"

    connect(ui->checkBox_CollectionCreateRawTx_SelAll, &QCheckBox::clicked, this, &CMainWindow::page_CollectionCreateRawTx_OnSelAll);

    //绑定"创建未签名归集交易-查询"按钮消息
    connect(ui->btn_CollectionCreateRawTx_Search, &QPushButton::clicked, this, &CMainWindow::page_CollectionCreateRawTx_OnSearch);

    //绑定"创建未签名归集交易-根据选中地址创建归集交易并导出"按钮消息
    connect(ui->btn_CollectionCreateRawTx_CreateRawTx, &QPushButton::clicked, this, &CMainWindow::page_CollectionCreateRawTx_OnCreateRawTx);

    connect(ui->btn_CollectionCreateRawTx_ERC20TokensSupplyTxFee, &QPushButton::clicked, this, &CMainWindow::page_CollectionCreateRawTx_OnSupplyTxFee);



    //绑定"归集交易广播-全选"按钮消息
    connect(ui->checkBox_CollectionBroadcast_SelAll, &QCheckBox::clicked, this, &CMainWindow::page_CollectionBroadcast_OnSelAll);

    //绑定"归集交易广播-搜索"按钮消息
    connect(ui->btn_CollectionBroadcast_Search, &QPushButton::clicked, this, &CMainWindow::page_CollectionBroadcast_OnSearch);

    //绑定"归集交易广播-导入待广播文件"按钮消息
    connect(ui->btn_CollectionBroadcast_ImportRawTx, &QPushButton::clicked, this, &CMainWindow::page_CollectionBroadcast_OnImportRawTx);

    //绑定"归集交易广播-确认广播选中地址"按钮消息
    connect(ui->btn_CollectionBroadcast_Ok, &QPushButton::clicked, this, &CMainWindow::page_CollectionBroadcast_OnOK);

    //绑定"大地址监控-查询"按钮消息
    connect(ui->btn_BigAccountList_Search, &QPushButton::clicked, this, &CMainWindow::page_BigAccountList_OnSearch);

    //绑定"大地址监控-重置"按钮消息
    connect(ui->btn_BigAccountList_Reset, &QPushButton::clicked, this, &CMainWindow::page_BigAccountList_OnReset);

    //绑定"大地址监控-删除选中地址"按钮消息
    connect(ui->btn_BigAccountList_Delete, &QPushButton::clicked, this, &CMainWindow::page_BigAccountList_OnDeleteAddr);

    //绑定"大地址监控-添加监控地址"按钮消息
    connect(ui->btn_BigAccountList_AddAddr, &QPushButton::clicked, this, &CMainWindow::page_BigAccountList_OnAddAddr);

    //归集日志列表
    connect(ui->btn_CollectionLog_Search, &QPushButton::clicked, this, &CMainWindow::page_CollectionLog_OnSearch);
    connect(ui->btn_CollectionLog_Reset, &QPushButton::clicked, this, &CMainWindow::page_CollectionLog_OnReset);
    connect(ui->btn_CollectionLog_exportExcel, &QPushButton::clicked, this, &CMainWindow::page_CollectionLog_OnExportExcel);
}

//初始化业务逻辑操作句柄
int CMainWindow::__InitHandles()
{
    int iRet = -1;

    //1.充值列表业务处理
    __m_pPageChargeList = new CPageChargeList(STR_SQLITE_FILE_PATH, "CPageChargeList");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_CHARGE_LIST,  __m_pPageChargeList )); //不要重复插入!!
    //int iSize = gs_mapHandles.size();
    iRet = __m_pPageChargeList->Init();
    if(CAMMain::NO_ERROR != iRet ) //所有 CAMMain的子类对象 在使用时必须调用Init函数, 进行初始化
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CPageChargeList 错误";
        return iRet;
    }

    //2.地址列表业务处理
    __m_pPageAddrList = new CPageAddrList(STR_SQLITE_FILE_PATH, "CPageAddrList");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_ADDR_LIST,  __m_pPageAddrList));
    iRet = __m_pPageAddrList->Init();
    if(CAMMain::NO_ERROR != iRet ) //所有 CAMMain的子类对象 在使用时必须调用Init函数, 进行初始化
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CPageAddrList 错误";
        return iRet;
    }

    //3.提现列表业务处理
    __m_pWithdraw = new CPageManualWithdraw(STR_SQLITE_FILE_PATH, "CWithdrawList");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_WITHDRAW,  __m_pWithdraw)); //不要重复插入!!
    iRet = __m_pWithdraw->Init();
    if(CAMMain::NO_ERROR != iRet ) //所有 CAMMain的子类对象 在使用时必须调用Init函数, 进行初始化
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CWithdraw 错误";
        return iRet;
    }

    //3.添加币种业务处理
    __m_pAddCoin = new CPageAddCoin(STR_SQLITE_FILE_PATH, "CAddCoin");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_ADD_COIN_LIST, __m_pAddCoin));
    iRet = __m_pAddCoin->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CAddCoin 错误";
        return iRet;
    }

    //4.平台财务流水
    __m_pExchangeStatements = new CPageExchangeStatements(STR_SQLITE_FILE_PATH, "ExchangeStatements");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_EXCHANGE_STATEMENTS, __m_pExchangeStatements));
    iRet = __m_pExchangeStatements->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 ExchangeStatements 错误";
        return iRet;
    }

    //5.充值日报
    __m_pChargeDaily = new CPageChargeDaily(STR_SQLITE_FILE_PATH, "ChargeDaily");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_CHARGE_DAILY, __m_pChargeDaily));
    iRet = __m_pChargeDaily->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 ChargeDaily 错误";
        return iRet;
    }
    //6.提现日报
    __m_pWithdrawDaily = new CPageWithdrawDaily(STR_SQLITE_FILE_PATH, "WithdrawDaily");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_WITHDRAW_DAILY, __m_pWithdrawDaily));
    iRet = __m_pWithdrawDaily->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 WithdrawDaily 错误";
        return iRet;
    }

    //7.管理员类型列表
    __m_pAdminTypeList = new CPageAdminTypeList(STR_SQLITE_FILE_PATH, "AdminTypeList");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_ADMINTYPELIST, __m_pAdminTypeList));
    iRet = __m_pAdminTypeList->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CPageAdminTypeList 错误";
        return iRet;
    }

    //7.添加归集业务处理
    __m_pCollection = new CPageCollection(STR_SQLITE_FILE_PATH, "CCollection");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_COLLECTION, __m_pCollection));
    iRet = __m_pCollection->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CCollection 错误";
        return iRet;
    }

    //8.添加管理员类型
    __m_pAddAdminType = new CPageAddAdminType(STR_SQLITE_FILE_PATH, "AddAdminType");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_ADDADMINTYPE, __m_pAddAdminType));
    iRet = __m_pAddAdminType->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CPageAddAdminType 错误";
        return iRet;
    }

    //9.管理员列表
    __m_pAdminList = new CPageAdminList(STR_SQLITE_FILE_PATH, "AdminList");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_ADMINLIST, __m_pAdminList));
    iRet = __m_pAdminList->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CPageAdminList 错误";
        return iRet;
    }

    //10.操作日志
    __m_pOperateLog = new CPageOperateLog(STR_SQLITE_FILE_PATH, "OperateLog");
    g_mapHandles.insert(make_pair(UINT_HANDLE::PAGE_OPERATELOG, __m_pOperateLog));
    iRet = __m_pOperateLog->Init();
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化 CPageOperateLog 错误";
        return iRet;
    }

    return NO_ERROR;
}

//初始化界面上所有的表格
int CMainWindow::__InitAllTableWidget()
{

#define INIT_TABLE_WIDGET_HEADER(tbw, rowCount, colCount, lstStrHeader)\
    do{\
    tbw->setRowCount(rowCount);\
    tbw->setColumnCount(colCount);\
    tbw->setHorizontalHeaderLabels(lstStrHeader);\
    tbw->setEditTriggers(QAbstractItemView::NoEditTriggers);\
    tbw->setSelectionBehavior(QAbstractItemView::SelectRows); \
    tbw->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); \
    tbw->verticalHeader()->setVisible(true); \
    tbw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);\
    tbw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); \
}while(0);

#define TABLE_ROW_COUNT  0

    QStringList  strLstHeader;
    //充值列表
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_CHARGE_TABLE_HEADER;
        //    INIT_TABLE_WIDGET_HEADER(ui->tbw_ChargeList, 0, INT_CHARGE_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_ChargeList->clearContents();
        ui->tbw_ChargeList->setColumnCount(strLstHeader.count());
        ui->tbw_ChargeList->setRowCount(0);
        ui->tbw_ChargeList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ChargeList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ChargeList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ChargeList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_ChargeList->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
        ui->tbw_ChargeList->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
    }

    //地址数量
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_ADDR_LIST_TABLE_HEADER;
        //    INIT_TABLE_WIDGET_HEADER(ui->tbw_AddrCountList, TABLE_ROW_COUNT, INT_ADDR_LIST_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_AddrCountList->clearContents();
        ui->tbw_AddrCountList->setColumnCount(strLstHeader.count());
        ui->tbw_AddrCountList->setRowCount(0);
        ui->tbw_AddrCountList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AddrCountList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AddrCountList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AddrCountList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AddrCountList->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_AddrCountList->setColumnWidth(0, 200);
        ui->tbw_AddrCountList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    }

    //地址详情
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_ADDR_LIST_DETAIL_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_AddrDetail, TABLE_ROW_COUNT, INT_ADDR_LIST_DETAIL_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_AddrDetail->clearContents();
        ui->tbw_AddrDetail->setColumnCount(strLstHeader.count());
        ui->tbw_AddrDetail->setRowCount(0);
        ui->tbw_AddrDetail->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AddrDetail->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AddrDetail->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AddrDetail->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AddrDetail->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_AddrDetail->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_AddrDetail->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_AddrDetail->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_AddrDetail->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_AddrDetail->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        //        ui->tbw_AddrDetail->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    }

    //待归集地址列表
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_RECHARGE_ADDR_LIST_TABLE_HEADER;
        //    INIT_TABLE_WIDGET_HEADER(ui->tbw_CollectionList, TABLE_ROW_COUNT, INT_RECHARGE_ADDR_LIST_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_CollectionList->clearContents();
        ui->tbw_CollectionList->setColumnCount(strLstHeader.count());
        ui->tbw_CollectionList->setRowCount(0);
        ui->tbw_CollectionList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_CollectionList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_CollectionList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_CollectionList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_CollectionList->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_CollectionList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //创建未签名归集交易
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_CREATE_UNSIGNED_RAW_TX_OF_COLLECTION_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_CollectionCreateRawTx, TABLE_ROW_COUNT, INT_CREATE_UNSIGNED_RAW_TX_OF_COLLECTION_COLUMN_COUNT, strLstHeader);
        ui->tbw_CollectionCreateRawTx->clearContents();
        ui->tbw_CollectionCreateRawTx->setColumnCount(strLstHeader.count());
        ui->tbw_CollectionCreateRawTx->setRowCount(0);
        ui->tbw_CollectionCreateRawTx->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_CollectionCreateRawTx->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_CollectionCreateRawTx->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_CollectionCreateRawTx->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_CollectionCreateRawTx->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_CollectionCreateRawTx->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//        ui->tbw_CollectionCreateRawTx->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//        ui->tbw_CollectionCreateRawTx->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
//        ui->tbw_CollectionCreateRawTx->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }

    //归集日志
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_COLLECTION_RECHARGE_LOG_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_CollectionLog, TABLE_ROW_COUNT, INT_COLLECTION_RECHARGE_LOG_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_CollectionLog->clearContents();
        ui->tbw_CollectionLog->setColumnCount(strLstHeader.count());
        ui->tbw_CollectionLog->setRowCount(0);
        ui->tbw_CollectionLog->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_CollectionLog->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_CollectionLog->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_CollectionLog->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_CollectionLog->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_CollectionLog->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    }

    //归集广播
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_COLLECTION_RECHARGE_BROADCAST_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_CollectionBroadcast, TABLE_ROW_COUNT, INT_COLLECTION_RECHARGE_BROADCAST_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_CollectionBroadcast->clearContents();
        ui->tbw_CollectionBroadcast->setColumnCount(strLstHeader.count());
        ui->tbw_CollectionBroadcast->setRowCount(0);
        ui->tbw_CollectionBroadcast->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_CollectionBroadcast->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_CollectionBroadcast->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_CollectionBroadcast->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_CollectionBroadcast->verticalHeader()->setVisible(true); //隐藏行头
//        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->setColumnWidth(0, 150);
//        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_CollectionBroadcast->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    }

    //归集-大账户监控
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_BIG_ACCOUNT_MONITORING_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_BigAccountList, TABLE_ROW_COUNT, INT_BIG_ACCOUNT_MONITORING_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_BigAccountList->clearContents();
        ui->tbw_BigAccountList->setColumnCount(strLstHeader.count());
        ui->tbw_BigAccountList->setRowCount(0);
        ui->tbw_BigAccountList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_BigAccountList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_BigAccountList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_BigAccountList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_BigAccountList->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_BigAccountList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_BigAccountList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_BigAccountList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    }

    //提现-自动提现
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_CASH_MNG_AUTO_CASH_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_AutoWithdraw, TABLE_ROW_COUNT, INT_CASH_MNG_AUTO_CASH_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_AutoWithdraw->clearContents();
        ui->tbw_AutoWithdraw->setColumnCount(strLstHeader.count());
        ui->tbw_AutoWithdraw->setRowCount(0);
        ui->tbw_AutoWithdraw->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AutoWithdraw->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AutoWithdraw->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AutoWithdraw->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AutoWithdraw->verticalHeader()->setVisible(true); //隐藏行头
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
        ui->tbw_AutoWithdraw->horizontalHeader()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
    }


    //提现-人工初审
    {
        strLstHeader.clear();
        strLstHeader << LIST_MANUAL_REVIEW_FIRSTTRIAL_TALBE_HEADER;
        //INIT_TABLE_WIDGET_HEADER(ui->tbw_ManualWithdraw1, TABLE_ROW_COUNT, INT_MANUAL_REVIEW_FIRSTTRIAL_TALBE_COLUMN_COUNT, strLstHeader);
        ui->tbw_ManualWithdraw1->clearContents();
        ui->tbw_ManualWithdraw1->setColumnCount(strLstHeader.count());
        ui->tbw_ManualWithdraw1->setRowCount(0);
        ui->tbw_ManualWithdraw1->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ManualWithdraw1->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ManualWithdraw1->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ManualWithdraw1->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        //ui->tbw_ManualWithdraw1->verticalHeader()->setVisible(false); //隐藏行头
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
        //        ui->tbw_ManualWithdraw1->horizontalHeader()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
    }

    //提现-人工复审
    {
        strLstHeader.clear();
        strLstHeader << LIST_MANUAL_REVIEW_FIRSTTRIAL_TALBE_HEADER;
        ui->tbw_ManualWithdraw2->clearContents();
        ui->tbw_ManualWithdraw2->setColumnCount(strLstHeader.count());
        ui->tbw_ManualWithdraw2->setRowCount(0);
        ui->tbw_ManualWithdraw2->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ManualWithdraw2->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ManualWithdraw2->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ManualWithdraw2->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        //ui->tbw_ManualWithdraw1->verticalHeader()->setVisible(false); //隐藏行头
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
        //        ui->tbw_ManualWithdraw2->horizontalHeader()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
    }


    //提现-用户资产
    {
        strLstHeader.clear();
        strLstHeader << LIST_MANUAL_REVIEW_USER_ASSET_TABLE_HEADER;
        ui->tbw_UserAssetdata->clearContents();
        ui->tbw_UserAssetdata->setColumnCount(strLstHeader.count());
        ui->tbw_UserAssetdata->setRowCount(0);
        ui->tbw_UserAssetdata->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_UserAssetdata->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_UserAssetdata->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_UserAssetdata->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        //ui->tbw_ManualWithdraw1->verticalHeader()->setVisible(false); //隐藏行头
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);//订单编号
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(10,QHeaderView::Stretch);
        ui->tbw_UserAssetdata->horizontalHeader()->setSectionResizeMode(11,QHeaderView::Stretch);
    }


    //提现-用户流水
    {
        strLstHeader.clear();
        strLstHeader << LIST_MANUAL_REVIEW_USER_FLOW_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_UserStatements, TABLE_ROW_COUNT, INT_MANUAL_REVIEW_USER_FLOW_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_UserStatements->clearContents();
        ui->tbw_UserStatements->setColumnCount(strLstHeader.count());
        ui->tbw_UserStatements->setRowCount(0);
        ui->tbw_UserStatements->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_UserStatements->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_UserStatements->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_UserStatements->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        //ui->tbw_ManualWithdraw1->verticalHeader()->setVisible(false); //隐藏行头
        ui->tbw_UserStatements->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //提现-创建未签名交易
    {
        //        int iCol = 0;
        strLstHeader.clear();
        strLstHeader << LIST_STR_CREATE_UNSIGNED_RAW_TX_OF_WITHDRAW_TABLE_HEADER;
        //INIT_TABLE_WIDGET_HEADER(ui->tbw_ManualWithdrawCreateRawTx, TABLE_ROW_COUNT, INT_CREATE_UNSIGNED_RAW_TX_OF_WITHDRAW_COLUMN_COUNT, strLstHeader);
        ui->tbw_ManualWithdrawCreateRawTx->clearContents();
        ui->tbw_ManualWithdrawCreateRawTx->setColumnCount(strLstHeader.count());
        ui->tbw_ManualWithdrawCreateRawTx->setRowCount(0);
        ui->tbw_ManualWithdrawCreateRawTx->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ManualWithdrawCreateRawTx->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ManualWithdrawCreateRawTx->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ManualWithdrawCreateRawTx->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        //ui->tbw_ManualWithdraw1->verticalHeader()->setVisible(false); //隐藏行头
        //        ui->tbw_ManualWithdrawCreateRawTx->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_ManualWithdrawCreateRawTx->setColumnWidth(0, 150);
        ui->tbw_ManualWithdrawCreateRawTx->setColumnWidth(1, 200);
        ui->tbw_ManualWithdrawCreateRawTx->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawCreateRawTx->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawCreateRawTx->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        ui->tbw_ManualWithdrawCreateRawTx->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        //ui->tbw_ManualWithdrawCreateRawTx->setColumnWidth(5, 200);
        //ui->tbw_ManualWithdrawCreateRawTx->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    }

    //提现-人工广播
    {
        //        int iCol = 0;
        strLstHeader.clear();
        strLstHeader << LIST_STR_MANUAL_REVIEW_BROADCAST_TABLE_HEADER;
        //INIT_TABLE_WIDGET_HEADER(ui->tbw_ManualWithdrawCreateRawTx, TABLE_ROW_COUNT, INT_CREATE_UNSIGNED_RAW_TX_OF_WITHDRAW_COLUMN_COUNT, strLstHeader);
        ui->tbw_ManualWithdrawBroadcast->clearContents();
        ui->tbw_ManualWithdrawBroadcast->setColumnCount(strLstHeader.count());
        ui->tbw_ManualWithdrawBroadcast->setRowCount(0);
        ui->tbw_ManualWithdrawBroadcast->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ManualWithdrawBroadcast->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ManualWithdrawBroadcast->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ManualWithdrawBroadcast->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_ManualWithdraw1->verticalHeader()->setVisible(true); //隐藏行头
//        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_ManualWithdrawBroadcast->setColumnWidth(0, 150);
//        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(1,  QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(2,  QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(3,  QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(4,  QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(5,  QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(6,  QHeaderView::ResizeToContents);
        ui->tbw_ManualWithdrawBroadcast->horizontalHeader()->setSectionResizeMode(7,  QHeaderView::Stretch);
    }

    //提现-人工提现列表
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_WITHDRAW_LIST_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_WithdrawList, TABLE_ROW_COUNT, INT_WITHDRAW_LIST_TABLE_COLUMN_COUNT, strLstHeader);
        ui->tbw_WithdrawList->clearContents();
        ui->tbw_WithdrawList->setColumnCount(strLstHeader.count());
        ui->tbw_WithdrawList->setRowCount(0);
        ui->tbw_WithdrawList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_WithdrawList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_WithdrawList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_WithdrawList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(12, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(13, QHeaderView::ResizeToContents);
        ui->tbw_WithdrawList->horizontalHeader()->setSectionResizeMode(14, QHeaderView::ResizeToContents);
    }


    //管理员类型管理
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_AUTH_MNG_ADMIN_TYPE_LIST_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_AdminTypeList, TABLE_ROW_COUNT, INT_AUTH_MNG_ADMIN_TYPE_LIST_COLUMN_COUNT, strLstHeader);
        ui->tbw_AdminTypeList->clearContents();
        ui->tbw_AdminTypeList->setColumnCount(strLstHeader.count());
        ui->tbw_AdminTypeList->setRowCount(0);
        ui->tbw_AdminTypeList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AdminTypeList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AdminTypeList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AdminTypeList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AdminTypeList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }


    //管理员列表
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_ADMIN_LIST_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_AdminList, TABLE_ROW_COUNT, INT_ADMIN_LIST_COLUMN_COUNT, strLstHeader);
        ui->tbw_AdminList->clearContents();
        ui->tbw_AdminList->setColumnCount(strLstHeader.count());
        ui->tbw_AdminList->setRowCount(0);
        ui->tbw_AdminList->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AdminList->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AdminList->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AdminList->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AdminList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //管理员操作日志
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_OPERATE_LOG_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_AdminLog, TABLE_ROW_COUNT, INT_OPERATE_LOG_COLUMN_COUNT, strLstHeader);
        ui->tbw_AdminLog->clearContents();
        ui->tbw_AdminLog->setColumnCount(strLstHeader.count());
        ui->tbw_AdminLog->setRowCount(0);
        ui->tbw_AdminLog->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AdminLog->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AdminLog->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AdminLog->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AdminLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_AdminLog->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_AdminLog->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_AdminLog->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_AdminLog->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    }

    //平台流水
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_PLATFORM_FINANCE_FLOW_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_ExchangeStatements, 0, INT_PLATFORM_FINANCE_FLOW_COLUMN_COUNT, strLstHeader);
        ui->tbw_ExchangeStatements->clearContents();
        ui->tbw_ExchangeStatements->setColumnCount(strLstHeader.count());
        ui->tbw_ExchangeStatements->setRowCount(0);
        ui->tbw_ExchangeStatements->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ExchangeStatements->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ExchangeStatements->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ExchangeStatements->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_ExchangeStatements->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //充值日报
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_RECHARGE_DAILY_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_ExchangeChargeDaily, TABLE_ROW_COUNT, INT_RECHARGE_DAILY_COLUMN_COUNT, strLstHeader);
        ui->tbw_ExchangeChargeDaily->clearContents();
        ui->tbw_ExchangeChargeDaily->setColumnCount(strLstHeader.count());
        ui->tbw_ExchangeChargeDaily->setRowCount(0);
        ui->tbw_ExchangeChargeDaily->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ExchangeChargeDaily->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ExchangeChargeDaily->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ExchangeChargeDaily->setSelectionMode(QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_ExchangeChargeDaily->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //提现日报
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_WITHDRAW_DAILY_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_ExchangeWithdrawDaily, TABLE_ROW_COUNT, INT_WITHDRAW_DAILY_COLUMN_COUNT, strLstHeader);
        ui->tbw_ExchangeWithdrawDaily->clearContents();
        ui->tbw_ExchangeWithdrawDaily->setColumnCount(strLstHeader.count());
        ui->tbw_ExchangeWithdrawDaily->setRowCount(0);
        ui->tbw_ExchangeWithdrawDaily->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_ExchangeWithdrawDaily->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_ExchangeWithdrawDaily->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_ExchangeWithdrawDaily->setSelectionMode(QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_ExchangeWithdrawDaily->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    //添加币种
    {
        strLstHeader.clear();
        strLstHeader << LIST_STR_SYS_MNG_ADD_COIN_TYPE_TABLE_HEADER;
        //        INIT_TABLE_WIDGET_HEADER(ui->tbw_AddCoin, TABLE_ROW_COUNT, INT_SYS_MNG_ADD_COIN_TYPE_COLUMN_COUNT, strLstHeader);
        ui->tbw_AddCoin->clearContents();
        ui->tbw_AddCoin->setColumnCount(strLstHeader.count());
        ui->tbw_AddCoin->setRowCount(0);
        ui->tbw_AddCoin->setHorizontalHeaderLabels(strLstHeader);//设置表格列头
        ui->tbw_AddCoin->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
        ui->tbw_AddCoin->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
        ui->tbw_AddCoin->setSelectionMode(QAbstractItemView::SingleSelection); //只能选中一行
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        ui->tbw_AddCoin->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    }
    return NO_ERROR;
}

//初始化界面上的所有的币种选择框
void CMainWindow::__InitCoinType_cbx()
{
    QStringList strlist;
    int iRet = __m_pPageChargeList->CheckAllCoinType(strlist);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败";
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("注意:查询失败,请检查数据库!"));
        MESSAGE_BOX_OK;
        return;
    }
    ui->cbx_ChargeList_CoinType->addItems(strlist);                 //充值列表
    ui->cbx_AddrCountList_CoinType->addItems(strlist);              //地址数量
    ui->cbx_AddrDetail_CoinType->addItems(strlist);                 //地址详情
    ui->cbx_CollectionCreateRawTx_CoinType->addItems(strlist);      //创建未签名归集交易
    ui->cbx_CollectionBroadcast_CoinType->addItems(strlist);        //归集交易广播
    ui->cbx_BigAccountList_CoinType->addItems(strlist);             //大地址监控
    ui->cbx_CollectionLog_CoinType->addItems(strlist);              //归集日志
    ui->cbx_AutoWithdraw_CoinType->addItems(strlist);               //自动提币
    ui->cbx_ManualWithdraw1_CoinType->addItems(strlist);            //初审列表
    ui->cbx_ManualWithdraw2_CoinType->addItems(strlist);            //复审列表
    ui->cbx_ManualWithdrawCreateRawTx_CoinType->addItems(strlist);  //创建未签名提币交易
    ui->cbx_ManualWithdrawBroadcast_CoinType->addItems(strlist);    //人工提币交易广播
    ui->cbx_ManualWithdrawList_CoinType->addItems(strlist);         //人工提币列表
    ui->cbx_ExchangeStatements_CoinType->addItems(strlist);         //平台流水
    ui->cbx_ExchangeChargeDaily_CoinType->addItems(strlist);        //充值日报
    ui->cbx_ExchangeWithdrawDaily_CoinType->addItems(strlist);      //提现日报
    ui->cbx_AddCoin_CoinType->addItems(strlist);                    //添加币种
    ui->cbx_UserAssertData_CoinType->addItems(strlist);             //用户资产数据
    ui->cbx_UserStatements_CoinType->addItems(strlist);             //用户流水
}

void CMainWindow::__initMultiPage()
{
    //添加分页组件到主界面中
    __m_pMutiPage = new CMultiPage();
    ui->multiPageHLayout->addWidget(__m_pMutiPage);
    __m_pMutiPage->setMaxPage(1);
    __m_pMutiPage->setCurrentPage(1);
    connect(__m_pMutiPage, &CMultiPage::sendCurrentPageChanged, this, &CMainWindow::onCurrentPageChanged);
}

//充值信息表
void CMainWindow::page_ChargeList_OnSearch(uint uCurrentPage)
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充币列表界面搜索按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币列表界面搜索按钮操作插入操作日志表失败";
    }
    ui->btn_ChargeList_Search->setEnabled(false);
    ui->btn_ChargeList_Search->setText("搜索中...");
    typedef ChargeList::SEARCHCOND  SearchCond;

//    uint uCurrentDatetime = QDateTime::currentDateTime().toTime_t();
    uint uStartDatetime = QDateTime::fromString(ui->date_ChargeList_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    uint uEndDatetime = QDateTime::fromString(ui->date_ChargeList_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    if (UINT_TIMESTAMP == uEndDatetime)
        ui->date_ChargeList_End->setDateTime(QDateTime::currentDateTime());

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_ChargeList_Search->setEnabled(true);
        ui->btn_ChargeList_Search->setText("搜索");
        return;
    }

    if (0 == uCurrentPage)
        ++uCurrentPage;
    SearchCond searchCond;
    searchCond.strCoinType = ui->cbx_ChargeList_CoinType->currentText().trimmed();
    searchCond.strTxid = ui->le_ChargeList_Txid->text().trimmed().toLower();
    searchCond.strDstAddr = ui->le_ChargeList_DstAddr->text().trimmed();
    searchCond.strChargeStatus = ui->cbx_ChargeList_ChargeStatus->currentText().trimmed();
    searchCond.strUploadStatus = ui->cbx_ChargeList_UploadStatus->currentText().trimmed();
    searchCond.uDatetimeStart = QDateTime::fromString(ui->date_ChargeList_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    searchCond.uDatetimeEnd = QDateTime::fromString(ui->date_ChargeList_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss").toTime_t();
    searchCond.strAuditStatus = ui->cbx_ChargeList_auditStatus->currentText();
    searchCond.uCurrentPage = uCurrentPage;
    vector<QStringList> vctChargeInfo; //搜索结果
    uint uMaxPage;  //最大页数

    iRet = __m_pPageChargeList->ConditionSearch(searchCond, vctChargeInfo, uMaxPage);
    if(NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询充值信息失败,请检查数据库"));
        MESSAGE_BOX_OK;
        ui->btn_ChargeList_Search->setEnabled(true);
        ui->btn_ChargeList_Search->setText("搜索");
        return;
    }
    if (0 == vctChargeInfo.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_ChargeList_Search->setEnabled(true);
        ui->btn_ChargeList_Search->setText("搜索");
        return;
    }
    //查询数据之前,先清除表格,并设置按钮不可点
    ui->tbw_ChargeList->clearContents();
    ui->tbw_ChargeList->setRowCount(0);
    //设置界面上表格数据显示
    SetTableWidgetFillData(ui->tbw_ChargeList, vctChargeInfo);
    __m_pMutiPage->setMaxPage(uMaxPage / UINT_ONE_PAGE_DATA_COUNT + 1);
    __m_pMutiPage->setCurrentPage(uCurrentPage);

    ui->btn_ChargeList_Search->setEnabled(true);
    ui->btn_ChargeList_Search->setText("搜索");
}
void CMainWindow::page_ChargeList_OnReset()
{
    //重置搜索条件
    ui->le_ChargeList_Txid->clear();
    ui->le_ChargeList_DstAddr->clear();
    ui->cbx_ChargeList_ChargeStatus->setCurrentIndex(0);
    ui->cbx_ChargeList_CoinType->setCurrentIndex(0);
    ui->cbx_ChargeList_UploadStatus->setCurrentIndex(0);
    ui->cbx_ChargeList_auditStatus->setCurrentIndex(0);
}
void CMainWindow::page_ChargeList_OnRelease()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充币列表界面释放按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币列表界面释放按钮操作插入操作日志表失败";
    }
    //判断选中的数据是否是上传失败的数据
    int iRetSelectedRow = ui->tbw_ChargeList->currentRow();
    if (0 == ui->tbw_ChargeList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }
    if (tr("上传失败") != ui->tbw_ChargeList->item(iRetSelectedRow, 5)->text())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择上传状态为失败的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }

    QMessageBox msgbox(QMessageBox::Question, tr("确定"), tr("确认释放这笔充值？"));
    MESSAGE_BOX_OK_CANCEL;
    int iQmessageRet = msgbox.exec();

    iRet = __m_pPageChargeList->ReUploadFailedOrder(iQmessageRet, ui->tbw_ChargeList->item(iRetSelectedRow, 0)->text().trimmed(),
                                                    ui->tbw_ChargeList->item(iRetSelectedRow, 6)->text().trimmed(),
                                                    ui->tbw_ChargeList->item(iRetSelectedRow, 2)->text().trimmed());

    if (INVALID_DATA_ERR == iRet)
    {
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "取消了释放!!";
        return;
    }
    if(NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "释放充值失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("释放充值失败, 请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    else
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("释放充值成功!!!"));
        MESSAGE_BOX_OK;
    }
}
void CMainWindow::page_ChargeList_OnExportExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充值列表界面导出EXCEL按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充值列表界面导出EXCEL按钮操作插入操作日志表失败";
    }
    if (0 == ui->tbw_ChargeList->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有充值数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中不存在数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_ChargeList);
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充值数据导出EXCEL文件失败";
        QMessageBox msgbox(QMessageBox::Warning, tr("失败"), tr("EXCEL文件导出失败!"));
        MESSAGE_BOX_OK;
        return;
    }

    QMessageBox msgbox(QMessageBox::Information, tr("成功"), tr("导出EXCEL成功!"));
    MESSAGE_BOX_OK;
}
void CMainWindow::OnItemDoubleClicked(QTableWidgetItem *tbwItem)
{
    if (tbwItem->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("选中的单元格没有内容!"));
        MESSAGE_BOX_OK;
        return;
    }

    QString strObjectName = tbwItem->tableWidget()->objectName();
    if (0 == strObjectName.compare("tbw_ChargeList") && UINT_CHARGELIST_TXID != tbwItem->column()
            && UINT_CHARGELIST_SRCADDR != tbwItem->column() && UINT_CHARGELIST_DSTADDR != tbwItem->column())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("只能选择txid和地址进行网页查询!"));
        MESSAGE_BOX_OK;
        return;
    }
    if (0 == strObjectName.compare("tbw_AutoWithdraw") && UINT_AUTOWITHDRAW_TXID != tbwItem->column()
            && UINT_AUTOWITHDRAW_DSTADDR != tbwItem->column())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("只能选择txid和地址进行网页查询!"));
        MESSAGE_BOX_OK;
        return;
    }
    if (0 == strObjectName.compare("tbw_WithdrawList") && UINT_WITHDRAWLIST_TXID != tbwItem->column()
            && UINT_WITHDRAWLIST_DSTADDR != tbwItem->column())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("只能选择txid和地址进行网页查询!"));
        MESSAGE_BOX_OK;
        return;
    }
    if (0 == strObjectName.compare("tbw_AddCoin") && 7 != tbwItem->column())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("只能选择区块浏览器IP进行网页查询!"));
        MESSAGE_BOX_OK;
        return;
    }

    QMap<QString, QString> mapCoinAndBlcokBrowser;
    int iRet = __m_pAddCoin->queryAllCoinType(mapCoinAndBlcokBrowser);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询币种表失败!请检查数据库";
        return;
    }
    checkInfoToBlockBrowser(tbwItem, strObjectName, mapCoinAndBlcokBrowser);
}
void CMainWindow::page_ChargeList_OnAudit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充币列表界面审核按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币列表界面审核按钮操作插入操作日志表失败";
    }
    //判断选中的数据是否是需要审核的数据
    int iSelectedRow = ui->tbw_ChargeList->currentRow();
    if (0 == ui->tbw_ChargeList->item(iSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }
    if (tr("需审核") != ui->tbw_ChargeList->item(iSelectedRow, 6)->text())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择需审核的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }
    if (ui->tbw_ChargeList->item(iSelectedRow, 1)->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("订单编号为空!"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }

    ChargeList::CHARGEINFO selectedChargeInfo;
    selectedChargeInfo.strAuditor = ui->tbw_ChargeList->item(iSelectedRow, 7)->text();
    selectedChargeInfo.strAuditRemark = ui->tbw_ChargeList->item(iSelectedRow, 8)->text();
    selectedChargeInfo.strAuditStatus = ui->tbw_ChargeList->item(iSelectedRow, 6)->text();
    selectedChargeInfo.strChargeStatus = ui->tbw_ChargeList->item(iSelectedRow, 4)->text();
    selectedChargeInfo.strCoinType = ui->tbw_ChargeList->item(iSelectedRow, 2)->text();
    selectedChargeInfo.strCount = ui->tbw_ChargeList->item(iSelectedRow, 3)->text();
    selectedChargeInfo.strDstAddr = ui->tbw_ChargeList->item(iSelectedRow, 11)->text();
    selectedChargeInfo.strOrderId = ui->tbw_ChargeList->item(iSelectedRow, 1)->text();
    selectedChargeInfo.strSrcAddr = ui->tbw_ChargeList->item(iSelectedRow, 10)->text();
    selectedChargeInfo.strTime = QString("%1").arg(QDateTime::fromString(ui->tbw_ChargeList->item(iSelectedRow, 0)->text(), "yyyy-MM-dd hh:mm:ss").toTime_t());
    selectedChargeInfo.strTxid = ui->tbw_ChargeList->item(iSelectedRow, 9)->text();

    __m_pDlg_auditAndReject->setDlgTitle(QString("审   核"), 0, selectedChargeInfo);
    __m_pDlg_auditAndReject->exec();

    if (QDialog::Accepted == __m_pDlg_auditAndReject->getFlag())
        page_ChargeList_OnSearch();
}
void CMainWindow::page_ChargeList_OnAuditReject()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充币列表界面驳回按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充币列表界面驳回按钮操作插入操作日志表失败";
    }
    //判断选中的数据是否是需要审核的数据
    int iSelectedRow = ui->tbw_ChargeList->currentRow();
    if (0 == ui->tbw_ChargeList->item(iSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }
    if (tr("需审核") != ui->tbw_ChargeList->item(iSelectedRow, 6)->text())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择需要审核的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }
    if (ui->tbw_ChargeList->item(iSelectedRow, 1)->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("订单编号为空!"));
        MESSAGE_BOX_OK;
        ui->tbw_ChargeList->setCurrentItem(NULL);
        return;
    }

    ChargeList::CHARGEINFO selectedChargeInfo;
    selectedChargeInfo.strAuditor = ui->tbw_ChargeList->item(iSelectedRow, 7)->text();
    selectedChargeInfo.strAuditRemark = ui->tbw_ChargeList->item(iSelectedRow, 8)->text();
    selectedChargeInfo.strAuditStatus = ui->tbw_ChargeList->item(iSelectedRow, 6)->text();
    selectedChargeInfo.strChargeStatus = ui->tbw_ChargeList->item(iSelectedRow, 4)->text();
    selectedChargeInfo.strCoinType = ui->tbw_ChargeList->item(iSelectedRow, 2)->text();
    selectedChargeInfo.strCount = ui->tbw_ChargeList->item(iSelectedRow, 3)->text();
    selectedChargeInfo.strDstAddr = ui->tbw_ChargeList->item(iSelectedRow, 11)->text();
    selectedChargeInfo.strOrderId = ui->tbw_ChargeList->item(iSelectedRow, 1)->text();
    selectedChargeInfo.strSrcAddr = ui->tbw_ChargeList->item(iSelectedRow, 10)->text();
    selectedChargeInfo.strTime = QString("%1").arg(QDateTime::fromString(ui->tbw_ChargeList->item(iSelectedRow, 0)->text(), "yyyy-MM-dd HH:mm:ss").toTime_t());
    selectedChargeInfo.strTxid = ui->tbw_ChargeList->item(iSelectedRow, 9)->text();

    __m_pDlg_auditAndReject->setDlgTitle(QString("驳   回"), 1, selectedChargeInfo);
    __m_pDlg_auditAndReject->exec();
}

//地址数量列表
void CMainWindow::page_AddrList_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("地址数量界面搜索按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "地址数量界面搜索按钮操作插入操作日志表失败";
    }
    //先清除表格内容,并设置按钮不可点
    ui->tbw_AddrCountList->clearContents();
    ui->tbw_AddrCountList->setRowCount(0);
    ui->btn_AddrCountList_Search->setEnabled(false);
    ui->btn_AddrCountList_Search->setText("搜索中...");
    //获取查询条件
    QString     strCoinType = ui->cbx_AddrCountList_CoinType->currentText().trimmed();
    int         iCoinType = ui->cbx_AddrCountList_CoinType->currentIndex();
    int         iCoinTypeCount = ui->cbx_AddrCountList_CoinType->count() - 1;
    vector<QString> vctAllCoinType;
    for (int i = 1; i < ui->cbx_AddrCountList_CoinType->count(); i++)
    {
        vctAllCoinType.push_back(ui->cbx_AddrCountList_CoinType->itemText(i).trimmed());
    }
    vector<QStringList> vctAddrInfo;

    iRet = __m_pPageAddrList->SearchCoinTypeToCount(iCoinTypeCount, iCoinType, strCoinType, vctAllCoinType, vctAddrInfo);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询地址数量失败,请检查数据库"));
        MESSAGE_BOX_OK;
        ui->btn_AddrCountList_Search->setEnabled(true);
        ui->btn_AddrCountList_Search->setText("搜索");
        return;
    }

    if (0 >= vctAddrInfo.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("数据库中没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_AddrCountList_Search->setEnabled(true);
        ui->btn_AddrCountList_Search->setText("搜索");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_AddrCountList, vctAddrInfo);
    ui->btn_AddrCountList_Search->setEnabled(true);
    ui->btn_AddrCountList_Search->setText("搜索");
}
void CMainWindow::page_AddrList_OnReset()
{
    ui->cbx_AddrCountList_CoinType->setCurrentIndex(0);
}
void CMainWindow::page_AddrList_OnLookDetail()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("地址数量界面查看详情按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "地址数量界面查看详情按钮操作插入操作日志表失败";
    }
    //清除地址详情表格内容
    ui->tbw_AddrDetail->clearContents();
    ui->tbw_AddrDetail->setRowCount(0);

    int iRetSelectedRow = ui->tbw_AddrCountList->currentRow();
    if (0 == ui->tbw_AddrCountList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AddrCountList->setCurrentItem(NULL);
        return;
    }
    //跳转到地址详情页
    ui->stackedWidget->setCurrentIndex(2);

    vector<QString> vctCoinType;
    vctCoinType.push_back("");
    vctCoinType.push_back(ui->tbw_AddrCountList->item(iRetSelectedRow, 0)->text().trimmed());

    vector<QStringList> vctAddrDetail;

    iRet = __m_pPageAddrList->LookAddrDetail(vctCoinType, vctAddrDetail);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询地址详情失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }

    if (0 >= vctAddrDetail.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        return;
    }
    SetTableWidgetFillData(ui->tbw_AddrDetail, vctAddrDetail);
}

//地址详情列表
void CMainWindow::page_AddrDetail_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("地址详情界面搜索按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "地址详情界面搜索按钮操作插入操作日志表失败";
    }
    //清除地址详情表格内容
    ui->tbw_AddrDetail->clearContents();
    ui->tbw_AddrDetail->setRowCount(0);
    ui->btn_AddrDetail_Search->setEnabled(false);
    ui->btn_AddrDetail_Search->setText("搜索中...");

    vector<QString>     vctCondition;
    vctCondition.push_back(ui->le_AddrDetail_DstAddr->text().trimmed());
    vctCondition.push_back(ui->cbx_AddrDetail_CoinType->currentText().trimmed());
    vector<QStringList> vctAddrDetail;

    iRet = __m_pPageAddrList->LookAddrDetail(vctCondition, vctAddrDetail);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询地址详情失败,请检查数据库"));
        MESSAGE_BOX_OK;
        ui->btn_AddrDetail_Search->setEnabled(true);
        ui->btn_AddrDetail_Search->setText("搜索");
        return;
    }
    if (0 >= vctAddrDetail.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("无相应结果"));
        MESSAGE_BOX_OK;
        ui->btn_AddrDetail_Search->setEnabled(true);
        ui->btn_AddrDetail_Search->setText("搜索");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_AddrDetail, vctAddrDetail);
    ui->btn_AddrDetail_Search->setEnabled(true);
    ui->btn_AddrDetail_Search->setText("搜索");
}
void CMainWindow::page_AddrDetail_OnReset()
{
    ui->le_AddrDetail_DstAddr->clear();
    ui->cbx_AddrDetail_CoinType->setCurrentIndex(0);
}
void CMainWindow::page_AddrList_OnRunWaterDetail()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("地址数量界面流水详情按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "地址数量界面流水详情按钮操作插入操作日志表失败";
    }
    //查询数据之前,先清除表格
    ui->tbw_ChargeList->clearContents();
    ui->tbw_ChargeList->setRowCount(0);
    //使用 ChargeList 命名空间中的 结构体
    typedef ChargeList::SEARCHCOND  SearchCond;

    int iRetSelectedRow = ui->tbw_AddrDetail->currentRow();
    if (0 == ui->tbw_AddrDetail->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AddrDetail->setCurrentItem(NULL);
        return;
    }
    ui->stackedWidget->setCurrentIndex(0);

    SearchCond searchCond;
    searchCond.strCoinType = QString("所有币种");
    searchCond.strTxid = QString("");
    searchCond.strDstAddr = ui->tbw_AddrDetail->item(iRetSelectedRow, 4)->text().trimmed();
    searchCond.strChargeStatus = QString("");
    searchCond.strUploadStatus = QString("");
    searchCond.uDatetimeEnd = QDateTime::currentDateTime().toTime_t();
    searchCond.uDatetimeStart = UINT_TIMESTAMP;

    vector<QStringList> vctChargeInfo;
    uint uMaxPage;
    iRet = __m_pPageChargeList->ConditionSearch(searchCond, vctChargeInfo, uMaxPage);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询流水详情失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    if (0 >= vctChargeInfo.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应数据"));
        MESSAGE_BOX_OK;
        return;
    }
    SetTableWidgetFillData(ui->tbw_ChargeList, vctChargeInfo);
}

//添加币种界面
void CMainWindow::page_AddCoin_OnAdd()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("添加币种界面添加币种按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加币种界面添加币种按钮操作插入操作日志表失败";
    }
    __m_pAddCoinDlg->setTitle(tr("添加币种"), -1);
    __m_pAddCoinDlg->exec();
}
void CMainWindow::page_AddCoin_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("添加币种界面查询按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加币种界面查询按钮操作插入操作日志表失败";
    }
    //清除表格内容
    ui->tbw_AddCoin->clearContents();
    ui->tbw_AddCoin->setRowCount(0);
    ui->btn_AddCoin_Search->setEnabled(false);
    ui->btn_AddCoin_Search->setText("查询中...");

    vector<QString> vctCondition;
    vctCondition.push_back(ui->cbx_AddCoin_CoinType->currentText().trimmed());
    vctCondition.push_back(ui->le_AddCoin_AutoWithdrawSrcAddr->text().trimmed());
    vector<QStringList> vctAddCoin;

    QString strErrMsg;
    try
    {
        iRet = __m_pAddCoin->Search(vctCondition, vctAddCoin);
        if (NO_ERROR != iRet)
        {
            strErrMsg = "条件查询失败";
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
            throw std::runtime_error( strErrMsg.toStdString()  );
        }

        SetTableWidgetFillData(ui->tbw_AddCoin, vctAddCoin);

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("币种配置信息查询成功! \r\n是否继续上传最新的提币额度配置到交易所管理台?"));
        msgbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgbox.setButtonText(QMessageBox::Ok, "是");
        msgbox.setButtonText(QMessageBox::Cancel, "否");
        msgbox.setWindowFlags(Qt::FramelessWindowHint);
        if (QMessageBox::Ok == msgbox.exec())
        {
            ui->btn_AddCoin_Search->setText("上传中...");
            __m_pAddCoin->UploadAllConfigToExAmdin();
            QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("上传配置成功"));
            MESSAGE_BOX_OK;
        }
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("错误: %1").arg(e.what());
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }
    catch(...)
    {
        strErrMsg = QString("未知错误");
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), strErrMsg);
        MESSAGE_BOX_OK;
    }


    ui->btn_AddCoin_Search->setEnabled(true);
    ui->btn_AddCoin_Search->setText("查询");
}
void CMainWindow::page_AddCoin_OnEdit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("添加币种界面编辑币种按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加币种界面编辑币种按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AddCoin->currentRow();
    if (0 == ui->tbw_AddCoin->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AddCoin->setCurrentItem(NULL);
        return;
    }
    vector<QString> vctCondition;
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 0)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 1)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 2)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 3)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 4)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 5)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 6)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 7)->text().trimmed());
    vctCondition.push_back(ui->tbw_AddCoin->item(iRetSelectedRow, 8)->text().trimmed());

    __m_pAddCoinDlg->GetCoinInfo(vctCondition);
    __m_pAddCoinDlg->setTitle(tr("编辑币种"), ui->tbw_AddCoin->item(iRetSelectedRow, 1)->text().toInt());
    __m_pAddCoinDlg->exec();
}
void CMainWindow::page_AddCoin_OnDelete()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("添加币种界面删除币种按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加币种界面删除币种按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AddCoin->currentRow();
    if (0 == ui->tbw_AddCoin->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AddCoin->setCurrentItem(NULL);
        return;
    }

    QString strCondition = ui->tbw_AddCoin->item(iRetSelectedRow, 3)->text().trimmed();
    QString strCoinType = ui->tbw_AddCoin->item(iRetSelectedRow, 0)->text().trimmed();

    __m_pDeleteCoinDlg->getAddr(strCondition, strCoinType);
    __m_pDeleteCoinDlg->exec();
}
void CMainWindow::OnAddCoinType(QString strCoinTypeName)
{
    ui->cbx_ChargeList_CoinType->addItem(strCoinTypeName);                 //充值列表
    ui->cbx_AddrCountList_CoinType->addItem(strCoinTypeName);              //地址数量
    ui->cbx_AddrDetail_CoinType->addItem(strCoinTypeName);                 //地址详情
    ui->cbx_CollectionCreateRawTx_CoinType->addItem(strCoinTypeName);      //创建未签名归集交易
    ui->cbx_CollectionBroadcast_CoinType->addItem(strCoinTypeName);        //归集交易广播
    ui->cbx_BigAccountList_CoinType->addItem(strCoinTypeName);             //大地址监控
    ui->cbx_CollectionLog_CoinType->addItem(strCoinTypeName);              //归集日志
    ui->cbx_AutoWithdraw_CoinType->addItem(strCoinTypeName);               //自动提币
    ui->cbx_ManualWithdraw1_CoinType->addItem(strCoinTypeName);            //初审列表
    ui->cbx_ManualWithdraw2_CoinType->addItem(strCoinTypeName);            //复审列表
    ui->cbx_ManualWithdrawCreateRawTx_CoinType->addItem(strCoinTypeName);  //创建未签名提币交易
    ui->cbx_ManualWithdrawBroadcast_CoinType->addItem(strCoinTypeName);    //人工提币交易广播
    ui->cbx_ManualWithdrawList_CoinType->addItem(strCoinTypeName);         //人工提币列表
    ui->cbx_ExchangeStatements_CoinType->addItem(strCoinTypeName);         //平台流水
    ui->cbx_ExchangeChargeDaily_CoinType->addItem(strCoinTypeName);        //充值日报
    ui->cbx_ExchangeWithdrawDaily_CoinType->addItem(strCoinTypeName);      //提现日报
    ui->cbx_AddCoin_CoinType->addItem(strCoinTypeName);                    //添加币种
    ui->cbx_UserAssertData_CoinType->addItem(strCoinTypeName);             //用户资产数据
    ui->cbx_UserStatements_CoinType->addItem(strCoinTypeName);             //用户流水
}
void CMainWindow::OnEditOrDeleteCoinType(QStringList strlistCoinTypeName)
{
    QComboBox *arrCbx[19] = {
        ui->cbx_ChargeList_CoinType, ui->cbx_AddrCountList_CoinType, ui->cbx_AddrDetail_CoinType, ui->cbx_CollectionCreateRawTx_CoinType,
        ui->cbx_CollectionBroadcast_CoinType, ui->cbx_BigAccountList_CoinType, ui->cbx_CollectionLog_CoinType, ui->cbx_AutoWithdraw_CoinType,
        ui->cbx_ManualWithdraw1_CoinType, ui->cbx_ManualWithdraw2_CoinType, ui->cbx_ManualWithdrawCreateRawTx_CoinType,
        ui->cbx_ManualWithdrawBroadcast_CoinType, ui->cbx_ManualWithdrawList_CoinType, ui->cbx_ExchangeStatements_CoinType,
        ui->cbx_ExchangeChargeDaily_CoinType, ui->cbx_ExchangeWithdrawDaily_CoinType, ui->cbx_AddCoin_CoinType,
        ui->cbx_UserAssertData_CoinType, ui->cbx_UserStatements_CoinType
    };

    for (uint i = 0; i < 19; i++)
    {
        arrCbx[i]->clear();
        arrCbx[i]->addItem("所有币种");
        for (int j = 0; j < strlistCoinTypeName.count(); j++)
        {
            arrCbx[i]->addItem(strlistCoinTypeName.at(j));
        }
    }
}

//平台财务流水界面
void CMainWindow::page_ExchangeStatements_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("平台流水界面查询按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "平台流水界面查询按钮操作插入操作日志表失败";
    }
    QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("暂不提供流水查询"));
    MESSAGE_BOX_OK;
    return;
    //先清除表格内容
    ui->tbw_ExchangeStatements->clearContents();
    ui->tbw_ExchangeStatements->setRowCount(0);
    ui->btn_ExchangeStatements_Search->setEnabled(false);
    ui->btn_ExchangeStatements_Search->setText("查询中...");

    vector<QString> vctCondition;
    vctCondition.push_back(ui->cbx_ExchangeStatements_CoinType->currentText().trimmed());
    vctCondition.push_back(ui->cbx_ExchangeStatements_StatementType->currentText().trimmed());
    vector<QStringList> vctExchangeStatements;

    iRet = __m_pExchangeStatements->StatementSearch(vctCondition, vctExchangeStatements);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询失败,请检查数据库"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeStatements_Search->setEnabled(true);
        ui->btn_ExchangeStatements_Search->setText("查询");
        return;
    }

    if (0 >= vctExchangeStatements.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("无相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeStatements_Search->setEnabled(true);
        ui->btn_ExchangeStatements_Search->setText("查询");
        return;
    }
    __m_pExchangeStatements->SortForDate(vctExchangeStatements);
//    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
//    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_ExchangeStatements, vctExchangeStatements);
    ui->btn_ExchangeStatements_Search->setEnabled(true);
    ui->btn_ExchangeStatements_Search->setText("查询");
}
void CMainWindow::page_ExchangeStatements_OutPutExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("平台流水界面导出EXCEL文档按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "平台流水界面导出EXCEL文档按钮操作插入操作日志表失败";
    }
    if (0 == ui->tbw_ExchangeStatements->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有财务流水数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中没有数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_ExchangeStatements);
    if (NO_ERROR != iRet)
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("平台财务流水导出失败\n请检查数据库书否有问题"));
        MESSAGE_BOX_OK;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "Excel表格导出失败";
        return;
    }

    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("平台财务流水导出成功"));
    MESSAGE_BOX_OK;
}

//充值日报界面
void CMainWindow::page_ChargeDaily_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充值日报界面查询按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充值日报界面查询按钮操作插入操作日志表失败";
    }
    ui->tbw_ExchangeChargeDaily->clearContents();
    ui->tbw_ExchangeChargeDaily->setRowCount(0);
    ui->btn_ExchangeChargeDaily_Search->setEnabled(false);
    ui->btn_ExchangeChargeDaily_Search->setText("查询中...");

    QDateTime       currentDatetime = QDateTime::currentDateTime();
    uint            uCurrentDatetime = currentDatetime.toTime_t();
    QDateTime       startDatetime = QDateTime::fromString(ui->date_ExchangeChargeDaily_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss");
    QDateTime       endDatetime = QDateTime::fromString(ui->date_ExchangeChargeDaily_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss");
    uint            uStartDatetime = startDatetime.toTime_t();
    uint            uEndDatetime = endDatetime.toTime_t();

    if (UINT_TIMESTAMP == uEndDatetime)
    {
        ui->date_ExchangeChargeDaily_End->setDateTime(currentDatetime);
    }

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeChargeDaily_Search->setEnabled(true);
        ui->btn_ExchangeChargeDaily_Search->setText("查询");
        return;
    }
    if (uStartDatetime > uCurrentDatetime)
    {
        qCritical() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << "输入的开始时间大于当前时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("输入的开始时间大于当前时间"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeChargeDaily_Search->setEnabled(true);
        ui->btn_ExchangeChargeDaily_Search->setText("查询");
        return;
    }

    vector<QString> vctCondition;
    vctCondition.push_back(ui->cbx_ExchangeChargeDaily_CoinType->currentText().trimmed());
    vctCondition.push_back(ui->date_ExchangeChargeDaily_Start->text().trimmed());
    vctCondition.push_back(ui->date_ExchangeChargeDaily_End->text().trimmed());
    vector<QStringList> vctChargeDaily;

    iRet = __m_pChargeDaily->ChargeDailySearch(vctCondition, vctChargeDaily);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("没有相关数据,请核对查询条件"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeChargeDaily_Search->setEnabled(true);
        ui->btn_ExchangeChargeDaily_Search->setText("查询");
        return;
    }

    if (0 >= vctChargeDaily.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeChargeDaily_Search->setEnabled(true);
        ui->btn_ExchangeChargeDaily_Search->setText("查询");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_ExchangeChargeDaily, vctChargeDaily);
    ui->btn_ExchangeChargeDaily_Search->setEnabled(true);
    ui->btn_ExchangeChargeDaily_Search->setText("查询");
}
void CMainWindow::page_ChargeDaily_OutPutExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("充值日报界面导出EXCEL文档按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "充值日报界面导出EXCEL文档按钮操作插入操作日志表失败";
    }
    if (0 == ui->tbw_ExchangeChargeDaily->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有充币日报数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中不存在数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_ExchangeChargeDaily);
    if (NO_ERROR != iRet)
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("充值日报导出失败\n请检查数据库书否有问题"));
        MESSAGE_BOX_OK;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "Excel表格导出失败";
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("充值日报导出成功"));
    MESSAGE_BOX_OK;
}

//提现日报界面
void CMainWindow::page_WithdrawDaily_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("提现日报界面查询按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "提现日报界面查询按钮操作插入操作日志表失败";
    }
    ui->tbw_ExchangeWithdrawDaily->clearContents();
    ui->tbw_ExchangeWithdrawDaily->setRowCount(0);
    ui->btn_ExchangeWithdrawDaily_Search->setEnabled(false);
    ui->btn_ExchangeWithdrawDaily_Search->setText("查询中...");

    QDateTime       currentDatetime = QDateTime::currentDateTime();
    uint            uCurrentDatetime = currentDatetime.toTime_t();
    QDateTime       startDatetime = QDateTime::fromString(ui->date_ExchangeWithdrawDaily_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss");
    QDateTime       endDatetime = QDateTime::fromString(ui->date_ExchangeWithdrawDaily_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss");
    uint            uStartDatetime = startDatetime.toTime_t();
    uint            uEndDatetime = endDatetime.toTime_t();

    if (UINT_TIMESTAMP == uEndDatetime)
    {
        ui->date_ExchangeWithdrawDaily_End->setDateTime(currentDatetime);
    }

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"),  tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeWithdrawDaily_Search->setEnabled(true);
        ui->btn_ExchangeWithdrawDaily_Search->setText("查询");
        return;
    }
    if (uStartDatetime > uCurrentDatetime)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "输入的开始时间大于当前时间";
        qCritical() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << "输入的开始时间大于当前时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("输入的开始时间大于当前时间"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeWithdrawDaily_Search->setEnabled(true);
        ui->btn_ExchangeWithdrawDaily_Search->setText("查询");
        return;
    }

    vector<QString> vctCondition;
    vctCondition.push_back(ui->cbx_ExchangeWithdrawDaily_CoinType->currentText().trimmed());
    vctCondition.push_back(ui->date_ExchangeWithdrawDaily_Start->text().trimmed());
    vctCondition.push_back(ui->date_ExchangeWithdrawDaily_End->text().trimmed());
    vector<QStringList> vctWithdrawDaily;

    iRet = __m_pWithdrawDaily->WithdrawDailySearch(vctCondition, vctWithdrawDaily);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "条件查询失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请监测数据库或核对查询条件"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeWithdrawDaily_Search->setEnabled(true);
        ui->btn_ExchangeWithdrawDaily_Search->setText("查询");
        return;
    }

    if (0 >= vctWithdrawDaily.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_ExchangeWithdrawDaily_Search->setEnabled(true);
        ui->btn_ExchangeWithdrawDaily_Search->setText("查询");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_ExchangeWithdrawDaily, vctWithdrawDaily);
    ui->btn_ExchangeWithdrawDaily_Search->setEnabled(true);
    ui->btn_ExchangeWithdrawDaily_Search->setText("查询");
}
void CMainWindow::page_WithdrawDaily_OutPutExcel()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("提现日报界面导出EXCEL文档按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "提现日报界面导出EXCEL文档按钮操作插入操作日志表失败";
    }
    if (0 == ui->tbw_ExchangeWithdrawDaily->rowCount())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "表格中没有提币日报数据";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("表格中不存在数据\n请先搜索您需要的数据"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pExchangeStatements->ExportExcel(ui->tbw_ExchangeWithdrawDaily);
    if (NO_ERROR != iRet)
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("提现日报导出失败\n请检查数据库书否有问题"));
        MESSAGE_BOX_OK;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "Excel表格导出失败";
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("提现日报导出成功"));
    MESSAGE_BOX_OK;
}

//管理员类型列表界面
void CMainWindow::page_AdminTypeList_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员类型管理界面查询管理员类型按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员类型管理界面查询管理员类型按钮操作插入操作日志表失败";
    }
    ui->tbw_AdminTypeList->clearContents();
    ui->tbw_AdminTypeList->setRowCount(0);
    ui->btn_AdminTypeList_Search->setEnabled(false);
    ui->btn_AdminTypeList_Search->setText("查询中...");

    vector<QStringList> vctAdminType;
    iRet = __m_pAdminTypeList->ConditionSearch(vctAdminType);
    if (NO_ERROR != iRet)
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询管理员类型失败\n请检查数据库书否有问题"));
        MESSAGE_BOX_OK;
        ui->btn_AdminTypeList_Search->setEnabled(true);
        ui->btn_AdminTypeList_Search->setText("查询管理员类型");
        return;
    }

    if (0 >= vctAdminType.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_AdminTypeList_Search->setEnabled(true);
        ui->btn_AdminTypeList_Search->setText("查询管理员类型");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_AdminTypeList, vctAdminType);
    ui->btn_AdminTypeList_Search->setEnabled(true);
    ui->btn_AdminTypeList_Search->setText("查询管理员类型");
}
void CMainWindow::page_AdminTypeList_OnAddAdminType()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员类型管理界面添加管理员类型按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员类型管理界面添加管理员类型按钮操作插入操作日志表失败";
    }

    ui->btn_AddAdminType_Edit->hide();
    ui->btn_AddAdminType_Add->show();
    ui->btn_AddAdminType_Add->setText(tr("添加"));
    ui->lb_AddAdminType_4->setText("添加管理员类型");
    uFlag = 0;

    onClearDisplay();
    ui->stackedWidget->setCurrentIndex(14);
}
void CMainWindow::page_AdminTypeList_OnEdit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员类型管理界面编辑按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员类型管理界面编辑按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminTypeList->currentRow();
    if (0 == ui->tbw_AdminTypeList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }

    ui->btn_AddAdminType_Edit->show();
    ui->btn_AddAdminType_Edit->setText("编辑");
    ui->btn_AddAdminType_Add->hide();
    ui->lb_AddAdminType_4->setText("编辑管理员类型");

    QTreeWidgetItemIterator iter(ui->treeWidget);
    while (*iter)
    {
        if(Qt::Checked == (*iter)->checkState(0))
        {
            (*iter)->setCheckState(0, Qt::Unchecked);
        }
        ++iter;
    }

    uFlag = ui->tbw_AdminTypeList->item(iRetSelectedRow, 0)->text().toUInt();
    ui->le_AddAdminType_TypeName->setText(ui->tbw_AdminTypeList->item(iRetSelectedRow, 1)->text());
    ui->le_AddAdminType_Department->setText(ui->tbw_AdminTypeList->item(iRetSelectedRow, 2)->text());
    ui->le_AddAdminType_Remark->setText(ui->tbw_AdminTypeList->item(iRetSelectedRow, 3)->text());
    __m_pAuthSet->DisplayAuth(uFlag, ui->treeWidget);
    ui->stackedWidget->setCurrentIndex(14);
}
void CMainWindow::page_AdminTypeList_OnFreeze()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员类型管理界面冻结按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员类型管理界面冻结按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminTypeList->currentRow();
    if (0 == ui->tbw_AdminTypeList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }
    if (tr("正常") != ui->tbw_AdminTypeList->item(iRetSelectedRow, 4)->text().trimmed())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择状态为\"正常\"的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }
    if (0 == ui->tbw_AdminTypeList->item(iRetSelectedRow, 0)->text().trimmed().compare(CUserInfo::loginUser().AdminTypeID()))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("不能冻结已登录管理员的管理员类型"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }

    uint uCondition = ui->tbw_AdminTypeList->item(iRetSelectedRow, 0)->text().toUInt();
    iRet = __m_pAdminTypeList->FreezeAdminType(uCondition);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "冻结失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("冻结失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("冻结成功,请注意:\n该种类型的管理员都会被冻结!"));
    MESSAGE_BOX_OK;
}
void CMainWindow::page_AdminTypeList_OnUnfreeze()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员类型管理界面解冻按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员类型管理界面解冻按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminTypeList->currentRow();
    if (0 == ui->tbw_AdminTypeList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }
    if (tr("冻结") != ui->tbw_AdminTypeList->item(iRetSelectedRow, 4)->text().trimmed())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择状态为\"冻结\"的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }

    uint uCondition = ui->tbw_AdminTypeList->item(iRetSelectedRow, 0)->text().toUInt();
    iRet = __m_pAdminTypeList->UnfreezeAdminType(uCondition);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "解冻失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("解冻失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("解冻成功,请注意:\n该种类型的管理员都会被解冻!"));
    MESSAGE_BOX_OK;
}
void CMainWindow::page_AdminTypeList_OnDelete()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员类型管理界面删除按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员类型管理界面删除按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminTypeList->currentRow();
    if (0 == ui->tbw_AdminTypeList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminTypeList->setCurrentItem(NULL);
        return;
    }

    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("确定删除该类型?注意:\n该类型的管理员将全部删除!"));
    MESSAGE_BOX_OK_CANCEL;
    iRet = msgbox.exec();
    if (QMessageBox::Ok != iRet)
    {
        return;
    }

    uint uCondition = ui->tbw_AdminTypeList->item(iRetSelectedRow, 0)->text().toUInt();
    iRet = __m_pAdminTypeList->DeleteAdminType(uCondition);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("删除失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    {
        QMessageBox msgbox(QMessageBox::Information, tr("成功"), tr("删除成功,注意:\n该类型的管理员将全部删除!"));
        MESSAGE_BOX_OK;
    }

    QStringList strlist;
    iRet = __m_pAdminList->SetAdminTypeCbx(strlist);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型名失败";
    }
    emit Send_UpdateAdminType(strlist);
}

//添加管理员类型界面
void CMainWindow::page_AddAdminType_OnAdd()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("添加管理员类型界面添加按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加管理员类型界面添加按钮操作插入操作日志表失败";
    }
    if (0 != uFlag)
    {
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "当前界面操作为:编辑管理员类型";
        return;
    }
    if (ui->le_AddAdminType_TypeName->text().isEmpty() || ui->le_AddAdminType_Department->text().isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "信息填写不完整";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请填写完整的信息"));
        MESSAGE_BOX_OK;
        return;
    }

    vector<QString> vctCondition;
    vctCondition.push_back(ui->le_AddAdminType_TypeName->text().trimmed());
    vctCondition.push_back(ui->le_AddAdminType_Department->text().trimmed());
    if (ui->le_AddAdminType_Remark->text().isEmpty())
    {
        vctCondition.push_back("--");
    }
    else
    {
        vctCondition.push_back(ui->le_AddAdminType_Remark->text().trimmed());
    }

    uint uAdminTypeID;

    iRet = __m_pAddAdminType->CheckAdminType(ui->le_AddAdminType_TypeName->text().trimmed());
    if (INVALID_DATA_ERR == iRet)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("却定"), tr("注意:已存在的管理员类型"));
        MESSAGE_BOX_OK;
        return;
    }

    iRet = __m_pAddAdminType->AddAdminType(vctCondition, uAdminTypeID);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加管理员类型失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("添加管理员类型失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    else
    {
        iRet = __m_pAuthSet->AddAdminType(uAdminTypeID, ui->treeWidget);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加管理员类型的权限失败";
            QMessageBox msgbox(QMessageBox::Critical, tr("添加失败"), tr("添加管理员类型的权限失败"));
            MESSAGE_BOX_OK;
            return;
        }
        QMessageBox msgbox(QMessageBox::Information, tr("添加成功"), tr("添加管理员类型成功!"));
        MESSAGE_BOX_OK;
    }

    emit Send_AddAdminType(ui->le_AddAdminType_TypeName->text().trimmed());
    ui->le_AddAdminType_TypeName->clear();
    ui->le_AddAdminType_Department->clear();
    ui->le_AddAdminType_Remark->clear();
    onClearDisplay();
}
void CMainWindow::page_AddAdminType_OnEdit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("添加管理员类型界面编辑按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "添加管理员类型界面编辑按钮操作插入操作日志表失败";
    }
    if (0 == uFlag)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "当前界面操作为:添加管理员类型";
        return;
    }

    if (ui->le_AddAdminType_TypeName->text().isEmpty() || ui->le_AddAdminType_Department->text().isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "信息填写不完整";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请填写完整的信息"));
        MESSAGE_BOX_OK;
        return;
    }

    vector<QString> vctCondition;
    vctCondition.push_back(ui->le_AddAdminType_TypeName->text());
    vctCondition.push_back(ui->le_AddAdminType_Department->text());
    if (ui->le_AddAdminType_Remark->text().isEmpty())
    {
        vctCondition.push_back("--");
    }
    else
    {
        vctCondition.push_back(ui->le_AddAdminType_Remark->text());
    }

    iRet = __m_pAddAdminType->UpdateAdminType(uFlag, vctCondition/*, vctValue*/);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "编辑管理员类型失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("编辑管理员类型失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    else
    {
        iRet = __m_pAuthSet->EditAdminType(uFlag, ui->treeWidget);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "编辑管理员类型失败";
            QMessageBox msgbox(QMessageBox::Warning, tr("编辑失败"), tr("编辑管理员类型失败!"));
            MESSAGE_BOX_OK;
            return;
        }
        QMessageBox msgbox(QMessageBox::Information, tr("添加成功"), tr("编辑管理员类型成功!"));
        MESSAGE_BOX_OK;
    }

    QStringList strlist;
    iRet = __m_pAdminList->SetAdminTypeCbx(strlist);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询管理员类型名失败";
    }
    emit Send_UpdateAdminType(strlist);
    ui->le_AddAdminType_TypeName->clear();
    ui->le_AddAdminType_Department->clear();
    ui->le_AddAdminType_Remark->clear();
    onClearDisplay();
}

//管理员列表界面
void CMainWindow::page_AdminList_OnSearch()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面搜索按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面搜索按钮操作插入操作日志表失败";
    }
    ui->tbw_AdminList->clearContents();
    ui->tbw_AdminList->setRowCount(0);
    ui->btn_AdminList_Search->setEnabled(false);
    ui->btn_AdminList_Search->setText("搜索中...");

    QDateTime       currentDatetime = QDateTime::currentDateTime();
    uint            uCurrentDatetime = currentDatetime.toTime_t();
    ui->date_AdminList_End->setDateTime(currentDatetime);
    QDateTime       startDatetime = QDateTime::fromString(ui->date_AdminList_Start->text().trimmed(), "yyyy-MM-dd hh:mm:ss");
    QDateTime       endDatetime = QDateTime::fromString(ui->date_AdminList_End->text().trimmed(), "yyyy-MM-dd hh:mm:ss");
    uint            uStartDatetime = startDatetime.toTime_t();
    uint            uEndDatetime = endDatetime.toTime_t();

    if (uStartDatetime > uEndDatetime)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开始时间大于结束时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("开始时间大于结束时间"));
        MESSAGE_BOX_OK;
        ui->btn_AdminList_Search->setEnabled(true);
        ui->btn_AdminList_Search->setText("搜索");
        return;
    }
    if ((uStartDatetime > uCurrentDatetime) || (uEndDatetime > uCurrentDatetime))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "输入的开始时间或结束时间大于当前时间";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("输入的开始时间或者结束时间大于当前时间"));
        MESSAGE_BOX_OK;
        ui->btn_AdminList_Search->setEnabled(true);
        ui->btn_AdminList_Search->setText("搜索");
        return;
    }

    vector<QString> vctCondition;
    vctCondition.push_back(ui->date_AdminList_Start->text().trimmed());
    vctCondition.push_back(ui->date_AdminList_End->text().trimmed());
    vctCondition.push_back(ui->le_AdminList_Name->text().trimmed());
    vctCondition.push_back(ui->le_AdminList_Tel->text().trimmed());
    vctCondition.push_back(ui->cbx_AdminList_Status->currentText().trimmed());
    vector<QStringList> vctAdminInfo;

    iRet = __m_pAdminList->SearchAdminInfo(vctCondition, vctAdminInfo);
    if (NO_ERROR != iRet)
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询管理员信息失败\n请检查数据库书否有问题"));
        MESSAGE_BOX_OK;
        ui->btn_AdminList_Search->setEnabled(true);
        ui->btn_AdminList_Search->setText("搜索");
        return;
    }

    if (0 >= vctAdminInfo.size())
    {
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_AdminList_Search->setEnabled(true);
        ui->btn_AdminList_Search->setText("搜索");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_AdminList, vctAdminInfo);
    ui->btn_AdminList_Search->setEnabled(true);
    ui->btn_AdminList_Search->setText("搜索");
}
void CMainWindow::page_AdminList_OnAddAdmin()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面新增管理员按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面新增管理员按钮操作插入操作日志表失败";
    }
    __m_AddAdmin->SetTitle(tr("添加管理员"), 0);
    __m_AddAdmin->exec();
}
void CMainWindow::page_AdminList_OnEdit()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面编辑按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面编辑按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminList->currentRow();
    if (0 == ui->tbw_AdminList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    vector<QString> vctCondition;
    vctCondition.push_back(ui->tbw_AdminList->item(iRetSelectedRow, 3)->text().trimmed());
    vctCondition.push_back(ui->tbw_AdminList->item(iRetSelectedRow, 1)->text().trimmed());
    vctCondition.push_back(ui->tbw_AdminList->item(iRetSelectedRow, 2)->text().trimmed());
    vctCondition.push_back("");
    vctCondition.push_back(ui->tbw_AdminList->item(iRetSelectedRow, 4)->text().trimmed());
    uint uAdminId = ui->tbw_AdminList->item(iRetSelectedRow, 0)->text().trimmed().toUInt();
    __m_AddAdmin->SetTitle(tr("编辑管理员"), uAdminId);
    __m_AddAdmin->SetUIAdminInfo(vctCondition);
    __m_AddAdmin->exec();
}
void CMainWindow::page_AdminList_OnResetPwd()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面重置密码按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面重置密码按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminList->currentRow();
    if (0 == ui->tbw_AdminList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    uint uAdminId = ui->tbw_AdminList->item(iRetSelectedRow, 0)->text().toUInt();
    __m_ResetPwd->GetAdminId(uAdminId);
    __m_ResetPwd->exec();
}
void CMainWindow::page_AdminList_OnFreeze()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面新冻结按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面新冻结按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminList->currentRow();
    if (0 == ui->tbw_AdminList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    if (tr("正常") != ui->tbw_AdminList->item(iRetSelectedRow, 5)->text().trimmed())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择管理员状态为\'正常\'的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    QString strUserLoginName = ui->tbw_AdminList->item(iRetSelectedRow, 1)->text().trimmed();
    if (0 == strUserLoginName.compare(CUserInfo::loginUser().LoginName(), Qt::CaseSensitive))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("冻结失败"), tr("注意:不能冻结自身!"));
        MESSAGE_BOX_OK;
        return;
    }

    uint uAdminId = ui->tbw_AdminList->item(iRetSelectedRow, 0)->text().toUInt();
    iRet = __m_pAdminList->FreezeAdmin(uAdminId);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "冻结管理员失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("冻结管理员失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("冻结成功:该管理员已被冻结"));
    MESSAGE_BOX_OK;
}
void CMainWindow::page_AdminList_OnUnfreeze()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面解冻按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面解冻按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminList->currentRow();
    if (0 == ui->tbw_AdminList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    if (tr("冻结") != ui->tbw_AdminList->item(iRetSelectedRow, 5)->text().trimmed())
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("请选择管理员状态为\'冻结\'的数据"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    uint uAdminId = ui->tbw_AdminList->item(iRetSelectedRow, 0)->text().toUInt();
    iRet = __m_pAdminList->UnfreezeAdmin(uAdminId);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "解冻管理员失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("解冻管理员失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("解冻成功:该管理员已解冻"));
    MESSAGE_BOX_OK;
}
void CMainWindow::page_AdminList_OnDelete()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("管理员列表界面冻结按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员列表界面冻结按钮操作插入操作日志表失败";
    }
    int iRetSelectedRow = ui->tbw_AdminList->currentRow();
    if (0 == ui->tbw_AdminList->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminList->setCurrentItem(NULL);
        return;
    }
    uint uAdminId = ui->tbw_AdminList->item(iRetSelectedRow, 0)->text().toUInt();

    iRet = QMessageBox::question(nullptr, tr("删除管理员"), tr("确定删除管理员信息?"), tr("确定"), tr("取消"));
    if (0 == iRet)
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "管理员将被删除";
        int uRet = __m_pAdminList->DeleteAdminInfo(uAdminId);
        if (NO_ERROR != uRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "删除管理员失败,请检查数据库";
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("删除管理员失败,请检查数据库"));
            MESSAGE_BOX_OK;
            return;
        }
        QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("删除成功:该管理员已删除"));
        MESSAGE_BOX_OK;
    }
    else
    {
        qWarning() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "取消删除管理员";
    }
}

//操作日志
void CMainWindow::page_OperateLog_OnSearch()
{
    ui->tbw_AdminLog->clearContents();
    ui->tbw_AdminLog->setRowCount(0);
    ui->btn_AdminLog_Search->setEnabled(false);
    ui->btn_AdminLog_Search->setText("搜索中...");

    OperateLogSpace::Condition condition;
    condition.strFamilyName = ui->le_AdminLog_Name->text().trimmed();
    condition.strTel = ui->le_AdminLog_Tel->text().trimmed();
    condition.strOperateType = ui->cbx_AdminLog_AdminType->currentText().trimmed();
    condition.strAdminId = ui->le_AdminLog_AdminID->text().trimmed();
    vector<QStringList> vctOperteLog;

    int iRet = __m_pOperateLog->SearchOperateLog(condition, vctOperteLog);
    if (NO_ERROR != iRet)
    {
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("查询操作日志失败\n请检查数据库是否有问题"));
        MESSAGE_BOX_OK;
        ui->btn_AdminLog_Search->setEnabled(true);
        ui->btn_AdminLog_Search->setText("搜索");
        return;
    }

    if (0 >= vctOperteLog.size())
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询操作日志失败";
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("没有相应的数据"));
        MESSAGE_BOX_OK;
        ui->btn_AdminLog_Search->setEnabled(true);
        ui->btn_AdminLog_Search->setText("搜索");
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("查询数据成功!"));
    MESSAGE_BOX_OK;
    SetTableWidgetFillData(ui->tbw_AdminLog, vctOperteLog);
    ui->btn_AdminLog_Search->setEnabled(true);
    ui->btn_AdminLog_Search->setText("搜索");
}
void CMainWindow::page_OperateLog_OnFreeze()
{
    OperateLogSpace::OperateLog LoginLog;
    LoginLog.strOperateType = QString("操作日志界面冻结按钮");
    int iRet = __m_pOperateLog->InsertOperateLog(LoginLog);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "操作日志界面冻结按钮的操作插入失败";
    }
    int iRetSelectedRow = ui->tbw_AdminLog->currentRow();
    if (0 == ui->tbw_AdminLog->item(iRetSelectedRow, 1))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("请选择有数据的行"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminLog->setCurrentItem(NULL);
        return;
    }
    if (0 == ui->tbw_AdminLog->item(iRetSelectedRow, 1)->text().trimmed().compare(CUserInfo::loginUser().AdminID()))
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("确定"), tr("不能冻结已登录用户!"));
        MESSAGE_BOX_OK;
        ui->tbw_AdminLog->setCurrentItem(NULL);
        return;
    }
    uint uAdminId = ui->tbw_AdminLog->item(iRetSelectedRow, 1)->text().toUInt();
    iRet = __m_pAdminList->FreezeAdmin(uAdminId);
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "冻结管理员失败,请检查数据库";
        QMessageBox msgbox(QMessageBox::Critical, tr("确定"), tr("冻结管理员失败,请检查数据库"));
        MESSAGE_BOX_OK;
        return;
    }
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("冻结成功:该管理员已被冻结"));
    MESSAGE_BOX_OK;
}

//根据txid或者地址查询对应币种在区块浏览器上的信息
void CMainWindow::checkInfoToBlockBrowser(QTableWidgetItem *tbwItem, const QString &strTbwObjectName, QMap<QString, QString> &mapCoinAndBlockBrowser)
{
    if ("tbw_ChargeList" == strTbwObjectName)
    {
        QString strCoinType = ui->tbw_ChargeList->item(tbwItem->row(), 2)->text();
        if (0 == strCoinType.compare("BTC"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/btc/address/%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/btc/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("USDT"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("ETH"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("LTC"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/%1").arg(tbwItem->text())));
        else if (0 == strCoinType.compare("DASH"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("BCH"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/bch/address/%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/bch/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("BSV"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("HTDF"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/account_detail?userhash=%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/deal_hash?tradehash=%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("USDP"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/accountdetail.html?userhash=%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tradedetail.html?tradehash=%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("HET"))
        {
            if (UINT_CHARGELIST_SRCADDR == tbwItem->column() || UINT_CHARGELIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/account_detail?userhash=%1").arg(tbwItem->text())));
            else if (UINT_CHARGELIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/deal_hash?tradehash=%1").arg(tbwItem->text())));
        }
        else
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("暂时不支持该币种的网页浏览!"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if ("tbw_AutoWithdraw" == strTbwObjectName)
    {
        QString strCoinType = ui->tbw_AutoWithdraw->item(tbwItem->row(), 6)->text();
        if (0 == strCoinType.compare("BTC"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/btc/address/%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/btc/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("USDT"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("ETH"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("LTC"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/%1").arg(tbwItem->text())));
        else if (0 == strCoinType.compare("DASH"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("BCH"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/bch/address/%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/bch/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("BSV"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("HTDF"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/account_detail?userhash=%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/deal_hash?tradehash=%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("USDP"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/accountdetail.html?userhash=%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tradedetail.html?tradehash=%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("HET"))
        {
            if (UINT_AUTOWITHDRAW_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/account_detail?userhash=%1").arg(tbwItem->text())));
            else if (UINT_AUTOWITHDRAW_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/deal_hash?tradehash=%1").arg(tbwItem->text())));
        }
        else
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("暂时不支持该币种的网页浏览!"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if ("tbw_WithdrawList" == strTbwObjectName)
    {
        QString strCoinType = ui->tbw_WithdrawList->item(tbwItem->row(), 6)->text();
        if (0 == strCoinType.compare("BTC"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/btc/address/%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/btc/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("USDT"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("ETH"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("LTC"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/%1").arg(tbwItem->text())));
        else if (0 == strCoinType.compare("DASH"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("BCH"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/bch/address/%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/bch/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("BSV"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/address/%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tx/%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("HTDF"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/account_detail?userhash=%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/deal_hash?tradehash=%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("USDP"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/accountdetail.html?userhash=%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/tradedetail.html?tradehash=%1").arg(tbwItem->text())));
        }
        else if (0 == strCoinType.compare("HET"))
        {
            if (UINT_WITHDRAWLIST_DSTADDR == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/account_detail?userhash=%1").arg(tbwItem->text())));
            else if (UINT_WITHDRAWLIST_TXID == tbwItem->column())
                QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/#/deal_hash?tradehash=%1").arg(tbwItem->text())));
        }
        else
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("查询失败"), tr("暂时不支持该币种的网页浏览!"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if ("tbw_AddCoin" == strTbwObjectName)
    {
        QString strCoinType = ui->tbw_AddCoin->item(tbwItem->row(), 0)->text();
        if (0 == strCoinType.compare("BTC"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("USDT"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("ETH"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("LTC"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("DASH"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("BCH"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType] + QString("/explorer")));
        else if (0 == strCoinType.compare("BSV"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("HTDF"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("USDP"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else if (0 == strCoinType.compare("HET"))
            QDesktopServices::openUrl(QUrl(mapCoinAndBlockBrowser[strCoinType]));
        else
            qWarning() << "暂不支持的币种浏览器查询!";
    }
    else
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("打开失败"), tr("该表格暂不支持跳转网页!"));
        MESSAGE_BOX_OK;
        return;
    }
}

//**********************mainWindow相关操作
void CMainWindow::__OnSkinChangeed(const QString &strCurrentText)
{
    if (tr("白色靓丽") == strCurrentText)
    {
        this->__m_uFlag = 0;
        //加载qss样式
        CommonHelper::setStyle(":/white.qss");
        __InitNavTree();
    }
    if (tr("黑色炫酷") == strCurrentText)
    {
        this->__m_uFlag = 1;
        //加载qss样式
        CommonHelper::setStyle(":/black.qss");
        __InitNavTree();
    }
}
void CMainWindow::__OnMinimizePress()
{
    this->showMinimized();
}
void CMainWindow::__OnfeedbackPress()
{
    this->showNormal();
}
void CMainWindow::__OnMaximizePress()
{
    this->showMaximized();
    //    this->showFullScreen();
}
void CMainWindow::__OnClosePress()
{
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("确定退出程序?"));
    MESSAGE_BOX_OK_CANCEL;
    int iRet = msgbox.exec();   //取消:4194304  确定:1024
    if (QMessageBox::Ok != iRet)
    {
        return;
    }

    /*if(NULL != g_ptr_qprocess_AMWatchDog)
    {
        //int iRet = QMessageBox::question(this, tr("信息"), tr("监测到看门狗仍在运行,是否关闭看门狗?"), QMessageBox::Yes, QMessageBox::No);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "手动关闭程序,自动杀死看门狗.";
        g_ptr_qprocess_AMWatchDog->kill();
        delete g_ptr_qprocess_AMWatchDog;
        g_ptr_qprocess_AMWatchDog = NULL;
    }

    QString strCmd = QString( "taskkill /im %1 /f").arg(gc_qstr_AMWatchDog);
    QProcess p(0);
    p.execute(strCmd);
    p.close();
    */

#if 0
    iRet = __m_getLoginUser->updateLoginStatus(CUserInfo::loginUser().LoginName());
    if (NO_ERROR != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "更新数据库中用户登录状态失败,请检查数据库";
    }
#endif
    this->close();
}
void CMainWindow::__OnSettingPress()
{
    ui->stackedWidget->setCurrentIndex(21);
}
void CMainWindow::__OnWatchDogPress()
{
    if ("看门狗开启" == ui->lbl_watchDog->text())
    {
        QIcon icon;
        icon.addFile(":/已关闭.png");
        ui->tbtn_watchDog->setIcon(icon);
        ui->lbl_watchDog->setText("看门狗关闭");

        /*if(NULL != g_ptr_qprocess_AMWatchDog)
        {
            g_ptr_qprocess_AMWatchDog->kill(); //杀死进程
            delete g_ptr_qprocess_AMWatchDog;
            g_ptr_qprocess_AMWatchDog = NULL;
        }

        QString strCmd = QString( "taskkill /im %1 /f").arg(gc_qstr_AMWatchDog);
        QProcess p(0);
        p.execute(strCmd);
        p.close();
        */

    }
    else if ("看门狗关闭" == ui->lbl_watchDog->text())
    {
        QIcon icon;
        icon.addFile(":/已开启.png");
        ui->tbtn_watchDog->setIcon(icon);
        ui->lbl_watchDog->setText("看门狗开启");

        /*
        QString strCmd = QString( "taskkill /im %1 /f").arg(gc_qstr_AMWatchDog);
        QProcess p(0);
        p.execute(strCmd);
        p.close();



        if(NULL != g_ptr_qprocess_AMWatchDog)
        {
            g_ptr_qprocess_AMWatchDog->kill(); //杀死进程
            delete g_ptr_qprocess_AMWatchDog;
            g_ptr_qprocess_AMWatchDog = NULL;
        }

        QStringList strListArgs;
        strListArgs << g_qstr_MainExeName << CUserInfo::loginUser().LoginName() << QByteArray::fromBase64( CUserInfo::loginUser().Password().toLatin1());

        g_ptr_qprocess_AMWatchDog = new QProcess();
        g_ptr_qprocess_AMWatchDog->start(gc_qstr_AMWatchDog, strListArgs);
        */

    }
}
void CMainWindow::__BindinBtn()
{
    //初始化看门狗按钮状态
    QIcon icon;
    icon.addFile(":/已关闭.png");
    ui->tbtn_watchDog->setIcon(icon);
    ui->lbl_watchDog->setText("看门狗关闭");

    connect(ui->cbx_skin, SIGNAL(currentTextChanged(QString)), this, SLOT(__OnSkinChangeed(QString)));
    connect(ui->btn_minimize, &QPushButton::clicked, this, &CMainWindow::__OnMinimizePress);
    connect(ui->btn_feedback, &QPushButton::clicked, this, &CMainWindow::__OnfeedbackPress);
    connect(ui->btn_maximize, &QPushButton::clicked, this, &CMainWindow::__OnMaximizePress);
    connect(ui->btn_close, &QPushButton::clicked, this, &CMainWindow::__OnClosePress);
    connect(ui->btn_setting, &QPushButton::clicked, this, &CMainWindow::__OnSettingPress);
    connect(ui->tbtn_watchDog, &QToolButton::clicked, this, &CMainWindow::__OnWatchDogPress);

    //注意:每个新增的qcombobox控件都要用下面的设置
    ui->cbx_ChargeList_CoinType->setView(new QListView());
    ui->cbx_ChargeList_ChargeStatus->setView(new QListView());
    ui->cbx_ChargeList_UploadStatus->setView(new QListView());
    ui->cbx_ChargeList_auditStatus->setView(new QListView());
    ui->cbx_AddrCountList_CoinType->setView(new QListView());
    ui->cbx_AddrDetail_CoinType->setView(new QListView());
    ui->cbx_CollectionList_CoinType->setView(new QListView());
    ui->cbx_CollectionCreateRawTx_CoinType->setView(new QListView());
    ui->cbx_CollectionBroadcast_CoinType->setView(new QListView());
    ui->cbx_BigAccountList_CoinType->setView(new QListView());
    ui->cbx_CollectionLog_CollectionStatus->setView(new QListView());
    ui->cbx_CollectionLog_CoinType->setView(new QListView());
    ui->cbx_AutoWithdraw_Status->setView(new QListView());
    ui->cbx_AutoWithdraw_CoinType->setView(new QListView());
    ui->cbx_ManualWithdraw1_CoinType->setView(new QListView());
    ui->cbx_ManualWithdraw1_Status->setView(new QListView());
    ui->cbx_ManualWithdraw2_CoinType->setView(new QListView());
    ui->cbx_ManualWithdraw2_Status->setView(new QListView());
    ui->cbx_ManualWithdrawCreateRawTx_CoinType->setView(new QListView());
    ui->cbx_ManualWithdrawBroadcast_CoinType->setView(new QListView());
    ui->cbx_AdminList_Status->setView(new QListView());
    ui->cbx_AdminLog_AdminType->setView(new QListView());
    ui->cbx_ExchangeStatements_CoinType->setView(new QListView());
    ui->cbx_ExchangeStatements_StatementType->setView(new QListView());
    ui->cbx_ExchangeChargeDaily_CoinType->setView(new QListView());
    ui->cbx_ExchangeWithdrawDaily_CoinType->setView(new QListView());
    ui->cbx_AddCoin_CoinType->setView(new QListView());
    ui->cbx_UserAssertData_CoinType->setView(new QListView());
    ui->cbx_UserAssertData_Status->setView(new QListView());
    ui->cbx_UserStatements_CoinType->setView(new QListView());
    ui->cbx_UserStatements_StatementType->setView(new QListView());
    ui->cbx_skin->setView(new QListView());
    ui->cbx_ManualWithdrawList_Status->setView(new QListView);
    ui->cbx_ManualWithdrawList_CoinType->setView(new QListView);
}
void CMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if ( __m_isMousePressed==true )
    {
        QPoint movePot = event->globalPos() - __m_mousePosition;
        move(movePot);
    }
}
void CMainWindow::mousePressEvent(QMouseEvent *event)
{
    __m_mousePosition = event->pos();
    //只对标题栏范围内的鼠标事件进行处理
    if (__m_mousePosition.x() <= pos_min_x)
        return;
    if (__m_mousePosition.x() >= pos_max_x)
        return;
    if (__m_mousePosition.y() <= pos_min_y )
        return;
    if (__m_mousePosition.y() >= pos_max_y)
        return;
    __m_isMousePressed = true;
}
void CMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    (void)event;
    //qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "event->x()为: " << event->x();
    __m_isMousePressed=false;
}
//**********************mainWindow相关操作

void CMainWindow::on_navlistview_pressed(const QModelIndex &index)
{
    if(__m_mapPage.end() != __m_mapPage.find(index.row()))
    {
        uint uPageIndex = __m_mapPage[index.row()];
        ui->stackedWidget->setCurrentIndex(uPageIndex);
        if (14 == uPageIndex)
        {
            emit sendClearDisplay();
        }
        else if(PAGE_INDEX_MANUALWITHDRAW1 == uPageIndex)
        {
            //ui->tbw_ManualWithdraw1->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);
        }
        else if(PAGE_INDEX_MANUALWITHDRAW2 == uPageIndex)
        {
            //ui->tbw_ManualWithdraw2->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);
        }
        else if(PAGE_INDEX_CREATERAWTX == uPageIndex)
        {
            ui->tbw_ManualWithdrawCreateRawTx->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);

        }
        else if(PAGE_INDEX_BROADCASTRAWTX == uPageIndex)
        {
            ui->tbw_ManualWithdrawBroadcast->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);
        }
        else if(PAGE_INDEX_MANUALWITHDRAWLIST == uPageIndex)
        {
            ui->tbw_WithdrawList->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);
        }
        else if (PAGE_INDEX_CHARGELIST == uPageIndex)
        {
            //ui->tbw_ChargeList->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);
        }
        else if (PAGE_INDEX_AUTOWITHDRAW == uPageIndex)
        {
            //ui->tbw_AutoWithdraw->setRowCount(0);
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);

        }
        else
        {
            __m_pMutiPage->setMaxPage(1);
            __m_pMutiPage->setCurrentPage(1);
        }
    }
    //如果不是叶子节点, 不切换页面
}

void CMainWindow::onClearDisplay()
{
    uFlag = 0;


    ui->btn_AddAdminType_Edit->hide();
    ui->btn_AddAdminType_Add->show();
    ui->btn_AddAdminType_Add->setText("添加");
    ui->lb_AddAdminType_4->setText("添加管理员类型");



    ui->le_AddAdminType_TypeName->clear();
    ui->le_AddAdminType_Department->clear();
    ui->le_AddAdminType_Remark->clear();
    QTreeWidgetItemIterator iter(ui->treeWidget);
    while (*iter)
    {
        if(Qt::Checked == (*iter)->checkState(0))
        {
            (*iter)->setCheckState(0, Qt::Unchecked);
        }
        ++iter;
    }
    ui->stackedWidget->setCurrentIndex(14);
    ui->stackedWidget->update();
}

void CMainWindow::onCurrentPageChanged(int iPage)
{
    //获取当前界面上显示的表格
    QString strCurrentTbwName = ui->stackedWidget->currentWidget()->objectName();

    if (0 == strCurrentTbwName.compare("page_ChargeList", Qt::CaseSensitive))
    {
        ui->tbw_ChargeList->clearContents();
        ui->tbw_ChargeList->setRowCount(0);
        page_ChargeList_OnSearch((uint)iPage);
    }
}
