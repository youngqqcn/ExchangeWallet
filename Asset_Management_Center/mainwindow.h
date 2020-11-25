#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "comman.h"
#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QSqlQuery>
#include <QCalendarWidget>

#include "short_msg_setting_dlg.h"   //充值页面 的  短信设置对话框
#include "page_charge_list.h" //充值页面   业务逻辑处理
#include "audit_dlg.h"  //充值页面 审核和波诡对话框
#include "page_addr_list.h" //地址页面 业务逻辑处理
#include "import_addr_dlg.h"   //地址页面 导入地址对话框
#include "page_add_coin.h"   //添加币种  业务逻辑
#include "add_coin_dlg.h"//添加币种 添加币种对话框
#include "delete_coin_dlg.h"//添加币种 删除币种对话框
#include "page_manual_withdraw.h"  //提现业务
#include "page_exchange_statements.h"//平台财务流水
#include "page_charge_daily.h"//充值日报业务逻辑
#include "page_withdraw_daily.h"//提现日报业务逻辑
#include "dialogs/page_Collection/page_Collection.h"  //提现业务
#include "page_admin_type_list.h"   //管理员类型列表
#include "page_add_admin_type.h"//添加管理员类型
#include "page_admin_list.h"//管理员列表
#include "add_admin_dlg.h"//添加管理员
#include "reset_pwd_dlg.h"//重置密码
#include "page_operate_log.h"//操作日志
#include "manualconfirmdialog.h"
#include "auth_setting.h"//权限设置
#include "add_big_addr_dlg.h"   //添加监控地址
#include "delete_big_addr_dlg.h"  //删除监控地址
#include "ccollectionconfirmdialog.h"  //归集确认
#include "get_login_user.h" //获取登录用户状态
#include "modules/paging/multi_page.h"  //分页组件

#define UINT_TIMESTAMP 1546272000

namespace Ui {
class MainWindow;
}



class CMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget *parent = 0);
    ~CMainWindow();


public:
    /***************************************************************************************
     * 函数作者: yqq
     * 功能说明:初始化, 包括数据初始化, 界面的初始化
     * 参数说明:无参数
     * 返回值说明:无返回值
     * 修改日志: 2019-03-27 新建函数
     ***************************************************************************************/
    int Init();


private:
    /***************************************************************************************
     * 函数作者: yqq
     * 功能说明:初始化主界面的左侧的导航(添加文本, 设置颜色等)
     * 参数说明:无参数
     * 返回值说明:无返回值
     * 修改日志: 2019-03-27 新建函数
     ***************************************************************************************/

    void __InsertLoginLog();            //登录成功,插入登录日志
    void __InitNavTree();               //初始化左边的导航栏和导航栏的样式
    void __InitAuth();                  //初始化权限选择框
    void __InitSignalBinding();         //初始化信号与槽的绑定
    int __InitHandles();                //初始化业务逻辑处理对象
    int __InitAllTableWidget();         //初始化所有tablewidget
    void __InitCoinType_cbx();          //初始化所有界面的币种选择框的下拉项
    void __initMultiPage();             //初始化分页组件并绑定相关信号槽

public:



private:

    //导航框中的row 与 嵌入式窗口的  映射表
    map<uint, uint>  __m_mapPage;

    //嵌入式窗口数量
    uint __m_uStackedWidgetCount;

    //-----根据txid或者地址查询对应币种在区块浏览器上的信息
    void checkInfoToBlockBrowser(QTableWidgetItem *tbwItem, const QString &strTbwObjectName, QMap<QString, QString> &mapCoinAndBlockBrowser);

     //自己实现的槽函数
private slots: //徐昌
    //-----充币信息表
    void page_ChargeList_OnSearch(uint uCurrentPage = 1);
    void page_ChargeList_OnReset();
    void page_ChargeList_OnRelease();
    void page_ChargeList_OnExportExcel();
    void OnItemDoubleClicked(QTableWidgetItem *tbwItem);
    void page_ChargeList_OnAudit();
    void page_ChargeList_OnAuditReject();
    //-----地址列表
    void page_AddrList_OnSearch();
    void page_AddrList_OnReset();
    void page_AddrList_OnLookDetail();
    //-----地址详情表
    void page_AddrDetail_OnSearch();
    void page_AddrDetail_OnReset();
    void page_AddrList_OnRunWaterDetail();
    //-----添加币种界面
    void page_AddCoin_OnAdd();
    void page_AddCoin_OnSearch();
    void page_AddCoin_OnEdit();
    void page_AddCoin_OnDelete();
    void OnAddCoinType(QString strCoinTypeName);    //添加币种成功槽函数(主界面上的所有币种选择框)
    void OnEditOrDeleteCoinType(QStringList strlistCoinTypeName);   //编辑或删除币种成功槽函数(主界面上的所有币种选择框)
    //-----平台财务流水
    void page_ExchangeStatements_OnSearch();
    void page_ExchangeStatements_OutPutExcel();
    //-----充值日报
    void page_ChargeDaily_OnSearch();
    void page_ChargeDaily_OutPutExcel();
    //-----提现日报
    void page_WithdrawDaily_OnSearch();
    void page_WithdrawDaily_OutPutExcel();
    //-----管理员类型管理
    void page_AdminTypeList_OnSearch();
    void page_AdminTypeList_OnAddAdminType();
    void page_AdminTypeList_OnEdit();
    void page_AdminTypeList_OnFreeze();
    void page_AdminTypeList_OnUnfreeze();
    void page_AdminTypeList_OnDelete();
    //-----添加管理员类型
    void page_AddAdminType_OnAdd();
    void page_AddAdminType_OnEdit();
    //-----管理员列表
    void page_AdminList_OnSearch();
    void page_AdminList_OnAddAdmin();
    void page_AdminList_OnEdit();
    void page_AdminList_OnResetPwd();
    void page_AdminList_OnFreeze();
    void page_AdminList_OnUnfreeze();
    void page_AdminList_OnDelete();
    //-----操作日志
    void page_OperateLog_OnSearch();
    void page_OperateLog_OnFreeze();

    //**********************mainWindow相关操作
private slots:
    void __OnSkinChangeed(const QString &strCurrentText);        //换肤

private:
    void __OnMinimizePress();       //最小化
    void __OnfeedbackPress();       //回复正常
    void __OnMaximizePress();       //最大化
    void __OnClosePress();          //关闭
    void __OnSettingPress();        //设置
    void __OnWatchDogPress();       //看门狗设置
    void __BindinBtn();             //绑定信号槽

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QPoint __m_mousePosition;
    bool __m_isMousePressed;
    static uint __m_uFlag;//flag标志,标志当前皮肤的样式;  0 -- 白色 , 1 -- 黑色
    //**********************mainWindow相关操作

private slots: //杨青青  李玲
    //人工审核-初审-搜索
    void page_ManulWithdraw1_OnSearch();

    //人工审核-初审-审核驳回
    void page_ManulWithdraw1_OnReject();
    //人工审核-初审-重置
    void page_ManulWithdraw1_OnReset();
    //人工提币-初审-审核
    void page_ManulWithdraw1_OnAudit();
    //人工提币-初审-查看用户资产
    void page_ManulWithdraw1_OnCheckAsset();

    //人工提币-初审-查看用户流水
    void page_ManulWithdraw1_OnCheckFlow();

    //人工提币-复审-审核驳回
    void page_ManulWithdraw2_OnReject();
    //人工提币-复审-审核通过
    void page_ManulWithdraw2_OnAudit();

    //人工提币-复审-查看用户资产
    void page_ManulWithdraw2_OnCheckAsset();


    //人工提币-复审-查看用户流水
    void page_ManulWithdraw2_OnCheckFlow();

    //人工审核-复审-搜索
    void page_ManulWithdraw2_OnSearch();

    //人工审核-复审-重置
    void page_ManulWithdraw2_OnReset();

    //自动提币列表
    void page_AutoWithdraw_OnReject();
    void page_AutoWithdraw_OnSearch();
    void page_AutoWithdraw_OnReset();
    void page_AutoWithdraw_OnExportExcel();

    //用户资产-确定
    void page_UserAssetData_OnOK();
    void page_UserAssetData_OnReset();
    void page_UserAssetData_OnAudit();
    void page_UserAssetData_OnSecAudit();

    //用户流水-查询
    void page_UserFlowData_OnSearch();

    //用户流水-初审
    void page_UserFlowData_OnJumptoManualWithdraw1();

    //用户流水-复审
    void page_UserFlowData_OnJumptoManualWithdraw2();

    //用户流水-查看资产数据
    void page_UserFlowData_OnJumptoAssetData();


    //查询某个币种(某个地址)所有的 已经复审通过的  提币订单数据
    void page_ManualWithdrawCreateRawTx_OnSearch();


    //-------------人工提币------------------------

    //人工提币-创建交易页面-全选
    void page_ManualWithdrawCreateRawTx_OnSelAll();


    //XMR 的人工提币操作比较特殊, 需要特殊处理
    int XMR_PreworkHandler()  ;

    //将已选中的订单,创建未签名的交易
    void page_ManualWithdrawCreateRawTx_OnOk();

    //导出未签名交易到文件中
    void page_ManualWithdrawCreateRawTx_OnExport();

    //人工提币, 导入已签名交易文件
    void page_ManualWithdrawBroadcast_OnImport();

    //人工提币, 广播已签名交易
    void page_ManualWithdrawBroadcast_OnBroadcast();


    //人工提币,广播搜索待广播
    void page_ManualWithdrawBroadcast_OnSelAll();

    //人工提币,广播搜索待广播
    void page_ManualWithdrawBroadcast_OnSearch();


    //人工提币列表-reset
    void page_ManualWithdrawList_OnReset();
    //人工提币列表-搜索
    void page_ManualWithdrawList_OnSearch();
    //人工提币列表-驳回
    void page_ManualWithdrawList_OnReject();
    //人工提币列表-处理异常订单 20190601 xc  TODO:后面链没问题之后删除
    void page_ManualWithdrawList_Release();
    void page_ManualWithdrawList_OnExportExcel();

    //-----------------------------------------
    //归集
    //待归集列表-搜索
//    void page_CollectionList_OnSearch();

    void page_CollectionCreateRawTx_OnSelAll();

    //待归集列表-搜索
 //   void page_CollectionList_OnCreateRawTx();

    //创建未签名归集交易-确认
    void page_CollectionCreateRawTx_OnSearch();

    //创建未签名归集交易-导入待生成清单
    void page_CollectionCreateRawTx_OnCreateRawTx();

    //ERC20 补手续费
    void page_CollectionCreateRawTx_OnSupplyTxFee();

    //创建未签名归集交易-确认生成选中地址
 //   void page_CollectionCreateRawTx_OnExport();




    //归集交易广播-全选
    void page_CollectionBroadcast_OnSelAll();

    //归集交易广播-搜索
    void page_CollectionBroadcast_OnSearch();

    //归集交易广播-导入待广播文件
    void page_CollectionBroadcast_OnImportRawTx();

    //归集交易广播-确认广播选中
    void page_CollectionBroadcast_OnOK();

    //大地址监控-查询
    void page_BigAccountList_OnSearch();

    //大地址监控-重置
    void page_BigAccountList_OnReset();

    //大地址监控-删除选中地址
    void page_BigAccountList_OnDeleteAddr();

    //大地址监控-添加监控地址
    void page_BigAccountList_OnAddAddr();

    //归集日志表
    void page_CollectionLog_OnSearch();
    void page_CollectionLog_OnReset();
    void page_CollectionLog_OnExportExcel();

    void OnShowAssetPage(QStringList strListOrderInfo);

signals:
    void sendClearDisplay();    //发送清除添加管理员类型界面的输入框信号
    void Send_ShowAssetPage(QStringList strListOrderInfo);
    void Send_AddAdminType(QString strAdminTypeName);   //添加管理员类型成功后发送信号
    void Send_UpdateAdminType(QStringList strlistAdminTypeName);//编辑或删除管理员类型成功后发送信号

private slots:
    void on_navlistview_pressed(const QModelIndex &index);          //鼠标点击左侧导航栏是, 会根据index切换页面
    void onClearDisplay();              //清除添加管理员类型界面的输入框
    void onCurrentPageChanged(int iPage);

private:
    Ui::MainWindow *ui;
private:
    uint                    uFlag;      //标志: 0 表示添加; !0 表示编辑
    //1.充值页面
    CPageChargeList         *__m_pPageChargeList; //充值页面业务逻辑处理
//    CShortMsgSettingDlg     *__m_pDlg_pageChargeList_ShortMsgSetting; //充值列表中 短信设置对话框
    audit_dlg               *__m_pDlg_auditAndReject;
    //2.地址页面
    CPageAddrList           *__m_pPageAddrList; //地址列表业务逻辑
    CImportAddrDlg          *__m_pImportAddr;   //地址列表导入地址对话框
    //3.添加币种页面
    CPageAddCoin            *__m_pAddCoin;  //添加币种业务逻辑
    CAddCoinDlg             *__m_pAddCoinDlg;//添加币种对话框
    CDeleteCoinDlg          *__m_pDeleteCoinDlg;//删除币种对话框
    //4.平台财务流水
    CPageExchangeStatements *__m_pExchangeStatements;//平台财务流水
    //5.充值日报
    CPageChargeDaily        *__m_pChargeDaily;//充值日报业务逻辑
    //6.提现日报
    CPageWithdrawDaily      *__m_pWithdrawDaily;//提现业务日报
    //7.管理员类型列表
    CPageAdminTypeList      *__m_pAdminTypeList;//管理员类型列表
    //8.添加管理员类型
    CPageAddAdminType       *__m_pAddAdminType;//添加管理员类型
    CAuthSetting            *__m_pAuthSet;//权限设置
    //9.管理员列表
    CPageAdminList          *__m_pAdminList;//管理员列表
    CAddAdminDlg            *__m_AddAdmin;//添加管理员
    CResetPwdDlg            *__m_ResetPwd;//重置密码
    //10.操作日志
    CPageOperateLog         *__m_pOperateLog;//操作日志
    CMultiPage              *__m_pMutiPage;//分页组件

    CPageManualWithdraw     *__m_pWithdraw;     //提现业务处理
    CPageCollection         *__m_pCollection;    //归集业务处理

    CGetLoginUser           *__m_getLoginUser;  //获取登录用户状态


public:
    //usdt
    QString m_strUsdtCollectionDstAddr;
    uint    m_nOnceCount;

};


#include <QFile>
#include <QApplication>
class CommonHelper
{
public:
    static void setStyle(const QString &style) {
        QFile qss(style);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
};



#endif // MAINWINDOW_H
