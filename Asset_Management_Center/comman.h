#ifndef COMMAN_H
#define COMMAN_H

#pragma execution_character_set("utf-8")

#include "config.h"  //配置文件

#include "rawtx_comman.h"
using namespace std;

//此文件放公共的函数 , 宏定义, 工具函数, 或者结构体
//  1.如果是各自模块的独有的不要放在此文件, 可在模块中单独新建一个头文件存放, 必须达到模块独立
//  2.如果是公共的函数, 请设置为static, 并保证已经测试成功过
//  3.如果是全局变量, 请注意  普通全局变量  和  静态全局变量   的区别和使用方法!

#define STR_SQLITE_FILE_PATH "DEPRECATED" //此值已废弃, 如有用到尽快修改

#define MESSAGE_BOX_OK msgbox.setStandardButtons(QMessageBox::Ok); \
    msgbox.setButtonText(QMessageBox::Ok, "确定");    \
    msgbox.setWindowFlags(Qt::FramelessWindowHint); \
    msgbox.exec()

#define MESSAGE_BOX_OK_CANCEL msgbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel); \
    msgbox.setButtonText(QMessageBox::Ok, "确定");    \
    msgbox.setButtonText(QMessageBox::Cancel, "取消");    \
    msgbox.setWindowFlags(Qt::FramelessWindowHint)

//-------正则表达式---------

//匹配 登录用户名6-20个字符
#define REGEXP_LOGIN_NAME  "^\\w{6,20}$"
//匹配 密码8-20个字符
#define REGEXP_PASSWD  "^(?![0-9]+$)(?![a-zA-Z]+$)[0-9A-Za-z]{8,20}$"
//匹配 手机
#define REGEXP_PHONE "^\\d{1,11}$"


//充币定时查询数据库中上传失败和待入账数据的计数阀值
#define INT_CHARGE_CHECK_UPLOADED_FAILED_ORDERS_COUNTER 200
#if INT_CHARGE_CHECK_UPLOADED_FAILED_ORDERS_COUNTER <= 0
#error("INT_COUNT 不符合要求!!! 必须大于0!!")
#endif

//-------正则表达式---------

//-------区块浏览器网址--------不要删除
//#define STR_BTC_BLOCKURL QString("https://live.blockcypher.com")
//#define STR_USDT_BLOCKURL QString("https://www.omniexplorer.info")
//#define STR_ETH_BLOCKURL QString("https://etherscan.io")
//#define STR_LTC_BLOCKURL QString("https://ltc.bitaps.com")
//#define STR_DASH_BLOCKURL QString("https://explorer.dash.org")
//#define STR_BCH_BLOCKURL QString("https://www.blockchain.com")
//#define STR_BSV_BLOCKURL QString("https://bchsvexplorer.com")
//#define STR_HTDF_BLOCKURL QString("http://www.htdfscan.com")
//#define STR_USDP_BLOCKURL QString("http://www.usdpscan.io")
//#define STR_HET_BLOCKURL QString("http://www.hetbiscan.io")
//-------区块浏览器网址--------

#define  SEARCHALL   99   //搜索所有数据

#define PAGE_INDEX_CHARGELIST       (0)
#define PAGE_INDEX_AUTOWITHDRAW     (7)
#define PAGE_INDEX_MANUALWITHDRAW1  (8)
#define PAGE_INDEX_MANUALWITHDRAW2  (9)
#define PAGE_INDEX_CREATERAWTX      (10)
#define PAGE_INDEX_BROADCASTRAWTX   (11)
#define PAGE_INDEX_MANUALWITHDRAWLIST   (12)
#define PAGE_INDEX_USERASSET   (22)
#define PAGE_INDEX_USERFLOW    (23)


#define UINT_TEL_LEN_MAX       (11)         //电话号码最大长度
#define UINT_TEL_LEN_MIN       (13)         //电话号码最小长度

#define UINT_PX_WIDTH_DATETIME      (130)   //时间 , 130像素
#define UINT_PX_WIDTH_COINTYPE      (50)    //币种 , 100像素
#define UINT_PX_WIDTH_ORDERID       (150)   //订单编号 , 150像素
#define UINT_PX_WIDTH_ADDR          (240)   //地址, 240像素
#define UINT_PX_NUMBER              (80)   //金额相关, 120像素
#define UINT_PX_CHECKBOX            (50)    //选择复选框, 50像素
#define UINT_PX_WIDTH_TXID          (465)   //TXID , 465像素
#define UINT_PX_WIDTH_STATUS        (60)    //状态 , 60像素
#define UINT_PX_WIDTH_USERID        (100)   //用户id, 100像素
#define UINT_PX_WIDTH_TEL           (100)    //用户手机号码
#define UINT_PX_WIDTH_USDERNAME     (60)    //用户姓名

//公共的错误码
enum COM_ERR_CODE{
    NO_ERROR        = 0,          //正常
    DB_ERR          = 10001,      //数据库错误
    ARGS_ERR        = 10002,      //函数参数错误
    FILE_ERROR      = 10003,      //文件错误
    INVALID_DATA_ERR= 1006,       //无效参数
    HTTP_ERR        = 1,          //http请求错误
    JSON_ERR        = 2,          //json解析错误
    INVALID_USERNAME = 3,         //无效用户名
    AUTH_FAILED     = 4,          //判断Java返回的签名和本地加密的签名不一致
};


//句柄所对应的索引
enum UINT_HANDLE{
    PAGE_CHARGE_LIST            = 0,                //充值列表
    PAGE_ADDR_LIST              = 1,                //地址列表
    PAGE_ADD_COIN_LIST          = 2,           		//添加币种
    PAGE_WITHDRAW               = 3,                //提现
    PAGE_EXCHANGE_STATEMENTS    = 4,                //平台财务流水
    PAGE_CHARGE_DAILY           = 5,                //充值日报
    PAGE_WITHDRAW_DAILY         = 6,                //提现日报
    PAGE_COLLECTION             = 7,                //归集
    PAGE_ADMINTYPELIST          = 8,                //管理员类型列表
    PAGE_ADDADMINTYPE           = 9,                //添加管理员类型
    PAGE_ADMINLIST              = 10,               //管理员列表
    PAGE_OPERATELOG             = 11,               //操作日志

};



//===============================================================================
class CAMMain;
extern map<UINT_HANDLE, CAMMain*>  g_mapHandles;   //声明外部全局变量

//获取业务句柄
template <class TDerive>
TDerive* GetHandle(UINT_HANDLE uHandle)
{
    auto itHandle = g_mapHandles.find(uHandle);//map<UINT_HANDLE, CAMMain*>::iterator itHandle = gs_mapHandles.find(uHandle);
    if(g_mapHandles.end() == itHandle)
        return nullptr;

    //将基类对象指针, 转为派生类对象指针  必须用 dynamic_cast  , 不能用原始的裸强制转换!!!
    // 如果是强制转换必须使用   static_cast, 而不用原始裸强制转换!!!
    return dynamic_cast<TDerive*>(itHandle->second);  //如果转失败, 返回 null
}
//===============================================================================

//===================所有表头信息宏定义=============================================
#define INT_CHARGE_TABLE_COLUMN_COUNT   (9)
#define LIST_STR_CHARGE_TABLE_HEADER   QString("充值时间")<<QString("订单编号")<<QString("币种")<<QString("充值数量")<<QString("充值状态")<<QString("上传状态")<<QString("审核状态")<<QString("审核人")<<QString("审核备注")<<QString("TXID")<<QString("来源地址")<<QString("充值地址")
//地址数量
#define INT_ADDR_LIST_TABLE_COLUMN_COUNT (2)
#define LIST_STR_ADDR_LIST_TABLE_HEADER QString("币种")<<QString("总数量")
//地址详情
#define INT_ADDR_LIST_DETAIL_TABLE_COLUMN_COUNT (5)
#define LIST_STR_ADDR_LIST_DETAIL_TABLE_HEADER QString("最新充值时间")<<QString("导入时间")<<QString("币种")<<QString("操作人")<<QString("地址")
//地址流水
#define INT_ADDR_LIST_RUNNING_WATER_DETAIL_COLUMN_COUNT (9)
#define LIST_STR_ADDR_LIST_RUNNING_WATER_DETAIL_TABLE_HEADER QString("充值时间")<<QString("订单编号")<<QString("充值币种")<<QString("充值数量")<<QString("充值状态")<<QString("上传状态")<<QString("TXID")<<QString("来源地址")<<QString("充值地址")
//待归集地址列表
#define INT_RECHARGE_ADDR_LIST_TABLE_COLUMN_COUNT (4)
#define LIST_STR_RECHARGE_ADDR_LIST_TABLE_HEADER QString("币种")<<QString("待归集余额")<<QString("待归集地址")<<QString("状态")
//归集日志
#define INT_COLLECTION_RECHARGE_LOG_TABLE_COLUMN_COUNT (9)
#define LIST_STR_COLLECTION_RECHARGE_LOG_TABLE_HEADER QString("币种")<<QString("订单编号")<<QString("源地址")<<QString("目的地址")<<QString("金额")<<QString("手续费")<<QString("TXID")<<QString("完成时间")<<QString("管理员")
//归集广播
#define INT_COLLECTION_RECHARGE_BROADCAST_TABLE_COLUMN_COUNT (6)
#define LIST_STR_COLLECTION_RECHARGE_BROADCAST_TABLE_HEADER QString("币种")<<QString("订单编号")<<QString("源地址")<<QString("目的地址")<<QString("金额")<<QString("矿工费")<<QString("签名状态")<<QString("广播状态")
//大账户监控
#define INT_BIG_ACCOUNT_MONITORING_TABLE_COLUMN_COUNT (4)
#define LIST_STR_BIG_ACCOUNT_MONITORING_TABLE_HEADER QString("币种")<<QString("地址余额")<<QString("监控地址")<<QString("最后查询时间")
//自动提现列表
#define INT_CASH_MNG_AUTO_CASH_TABLE_COLUMN_COUNT (12)
#define LIST_STR_CASH_MNG_AUTO_CASH_TABLE_HEADER QString("订单编号")<<QString("提现时间")<<QString("完成时间")<<QString("用户UID")<<QString("用户注册邮箱或手机")<<QString("会员姓名")<<QString("提现币种")<<QString("提现数量")<<QString("手续费")<<QString("矿工费")<<QString("提现状态")<<QString("备注")<<QString("TXID")<<QString("提现地址")
//人工审核提币初审
#define INT_MANUAL_REVIEW_FIRSTTRIAL_TALBE_COLUMN_COUNT (13)
#define LIST_MANUAL_REVIEW_FIRSTTRIAL_TALBE_HEADER QString("订单编号")<<QString("提现时间")<<QString("用户UID")<<QString("用户注册邮箱或手机")<<QString("会员姓名")<<QString("提现币种")<<QString("提现数量")<<QString("手续费")<<QString("矿工费")<<QString("提现状态")<<QString("备注")<<QString("提现地址")<<QString("审核人")
//人工审核提币复审
#define INT_MANUAL_REVIEW_RECHECK_TALBE_COLUMN_COUNT (13)
#define LIST_MANUAL_REVIEW_RECHECK_TALBE_HEADER QString("订单编号")<<QString("提现时间")<<QString("用户UID")<<QString("用户注册邮箱或手机")<<QString("会员姓名")<<QString("提现币种")<<QString("提现数量")<<QString("手续费")<<QString("矿工费")<<QString("提现状态")<<QString("备注")<<QString("提现地址")<<QString("审核人")
//用户资产
#define INT_MANUAL_REVIEW_USER_ASSET_TABLE_COLUMN_COUNT (10)
#define LIST_MANUAL_REVIEW_USER_ASSET_TABLE_HEADER QString("币种")<<QString("订单编号")<<QString("充值总数量")<<QString("提现总数量")<<QString("总余额")<<QString("可提现金额")<<QString("冻结金额")<<QString("提币冻结")<<QString("锁定金额")<<QString("提币金额")<<QString("手续费")<<QString("状态")
//用户流水
#define INT_MANUAL_REVIEW_USER_FLOW_TABLE_COLUMN_COUNT (5)
#define LIST_MANUAL_REVIEW_USER_FLOW_TABLE_HEADER QString("币种")<<QString("类型")<<QString("数量")<<QString("余额")<<QString("时间")
//人工审核提币广播
#define INT_MANUAL_REVIEW_BROADCAST_TABLE_COLUMN_COUNT (6)
#define LIST_STR_MANUAL_REVIEW_BROADCAST_TABLE_HEADER  QString("币种")<<QString("订单编号")<<QString("目的地址")<<QString("源地址")<<QString("金额")<<QString("矿工费")<<QString("签名状态")<<QString("广播状态")
//人工提币列表
#define INT_WITHDRAW_LIST_TABLE_COLUMN_COUNT (14)
#define LIST_STR_WITHDRAW_LIST_TABLE_HEADER QString("订单编号")<<QString("提现时间")<<QString("完成时间")<<QString("会员UID")<<QString("用户注册邮箱或手机")<<QString("会员姓名")<<QString("提现币种")<<QString("提现数量")<<QString("手续费")<<QString("矿工费")<<QString("提现状态")<<QString("备注")<<QString("审核人")<<QString("TXID")<<QString("提现地址")
//添加币种
#define INT_SYS_MNG_ADD_COIN_TYPE_COLUMN_COUNT (6)
//#define LIST_STR_SYS_MNG_ADD_COIN_TYPE_TABLE_HEADER QString("币种")<<QString("序号")<<QString("地址数量")<<QString("自动提现地址")<<QString("最小人工提币数量")<<QString("单日累计提币最大值")<<QString("充币审核阀值")<<QString("区块浏览器IP")
#define LIST_STR_SYS_MNG_ADD_COIN_TYPE_TABLE_HEADER QString("币种")<<QString("序号")<<QString("地址数量")<<QString("自动提现地址")<<QString("全自动提币上限")<<QString("半自动提币上限")<<QString("单日累计提币最大值")<<QString("充币审核阀值")<<QString("区块浏览器IP")
//管理员类型列表
#define INT_AUTH_MNG_ADMIN_TYPE_LIST_COLUMN_COUNT (6)
#define LIST_STR_AUTH_MNG_ADMIN_TYPE_LIST_TABLE_HEADER QString("ID")<<QString("管理员类型")<<QString("所属部门")<<QString("备注")<<QString("状态")<<QString("创建时间")
//管理员列表
#define INT_ADMIN_LIST_COLUMN_COUNT (7)
#define LIST_STR_ADMIN_LIST_TABLE_HEADER QString("管理员ID")<<QString("用户名")<<QString("姓名")<<QString("管理员类型")<<QString("手机号码")<<QString("状态")<<QString("添加时间")
//操作日志
#define INT_OPERATE_LOG_COLUMN_COUNT (5)
#define LIST_STR_OPERATE_LOG_TABLE_HEADER QString("操作时间")<<QString("管理员ID")<<QString("管理员姓名")<<QString("手机号码")<<QString("类型")
//平台流水
#define INT_PLATFORM_FINANCE_FLOW_COLUMN_COUNT (6)
#define LIST_STR_PLATFORM_FINANCE_FLOW_HEADER QString("币种")<<QString("类型")<<QString("数量")<<QString("充币总金额")<<QString("提现总金额")<<QString("时间")
//充值日报
#define INT_RECHARGE_DAILY_COLUMN_COUNT (4)
#define LIST_STR_RECHARGE_DAILY_TABLE_HEADER QString("币种")<<QString("类型")<<QString("数量")<<QString("时间")
//提现日报
#define INT_WITHDRAW_DAILY_COLUMN_COUNT (4)
#define LIST_STR_WITHDRAW_DAILY_TABLE_HEADER QString("币种")<<QString("类型")<<QString("数量")<<QString("时间")
//创建未签名归集交易
#define INT_CREATE_UNSIGNED_RAW_TX_OF_COLLECTION_COLUMN_COUNT (3)
#define LIST_STR_CREATE_UNSIGNED_RAW_TX_OF_COLLECTION_TABLE_HEADER QString("币种")<<QString("待归集余额")<<QString("待生成地址")<<QString("手续费不足")
//创建未签名提现交易
#define INT_CREATE_UNSIGNED_RAW_TX_OF_WITHDRAW_COLUMN_COUNT (6)
#define LIST_STR_CREATE_UNSIGNED_RAW_TX_OF_WITHDRAW_TABLE_HEADER QString("币种")<<QString("订单编号")<<QString("目的地址")<<QString("源地址")<<QString("金额")<<QString("矿工费")
//===================所有表头信息宏定义=============================================

//===================所有工具函数宏定义=============================================
//填充数据到表格中
#define UINT_ONE_PAGE_DATA_COUNT 500

inline void SetTableWidgetFillData(QTableWidget *pTableWidget, vector<QStringList> &vctTableValue)
{
    int nRow = 0;
    for (auto it : vctTableValue)
    {
        pTableWidget->insertRow(nRow);
        for (int j = 0; j < pTableWidget->columnCount(); j++)
        {
            pTableWidget->setItem(nRow, j, new QTableWidgetItem(it.at(j)));
        }
        nRow++;
    }
}
//===================所有工具函数宏定义=============================================

//===================所有数字宏定义================================================
#define UINT_CHARGELIST_TXID        9
#define UINT_CHARGELIST_SRCADDR     10
#define UINT_CHARGELIST_DSTADDR     11

#define UINT_AUTOWITHDRAW_TXID      11
#define UINT_AUTOWITHDRAW_DSTADDR   12

#define UINT_WITHDRAWLIST_TXID      12
#define UINT_WITHDRAWLIST_DSTADDR   13


//===================所有魔鬼数字宏定义================================================

//===================权限管理相关宏定义============================================
/*
 * 权限管理算法:
 *
 *       用uint64存储权限标记    高32位保存"页面权限"   低32位表示"控件权限"
 *
 *      一个管理员的权限  =  (页面权限)  |   (控件权限)
 *
 */
//充值列表页面的子控件(按钮)的权限--1
#define INT_CTRL_AUTH_RECHARGELIST_SEARCH               0x1 //"搜索"
#define INT_CTRL_AUTH_RECHARGELIST_RESET                0x2 //"重置"
#define INT_CTRL_AUTH_RECHARGELIST_RELEASE              0x4 //"释放"
#define INT_CTRL_AUTH_RECHARGELIST_MESSAGE_NOTIFY       0x8 //"审核"
//地址数量页面的子控件(按钮)的权限--2
#define INT_CTRL_AUTH_ADDRLIST_SEARCH   0x1    //"搜索"
#define INT_CTRL_AUTH_ADDRLIST_RESET    0x2    //"重置"
#define INT_CTRL_AUTH_ADDRLIST_IMPORT   0x4    //"导入地址"
#define INT_CTRL_AUTH_ADDRLIST_DETAILS  0x8    //查看详情
//地址详情界面的子控件(按钮)的权限--3
#define INT_CTRL_AUTH_ADDRLISTDETAIL_SEARCH     0x1     //搜索
#define INT_CTRL_AUTH_ADDRLISTDETAIL_RESET      0x2     //重置
#define INT_CTRL_AUTH_ADDRLISTDETAIL_RUNWATERDETAILS    0x4     //流水详情
//待归集列表界面按钮权限--4
//#define INT_CTRL_AUTH_WAITCOLLECTIONLIST_SEARCH         0x1     //搜索
//#define INT_CTRL_AUTH_WAITCOLLECTIONLIST_GENERATE       0x2     //生成归集充值地址清单
//创建未签名归集交易界面的子控件(按钮)的权限--4
#define INT_CTRL_AUTH_COLLECTIONCREATERAWTX_SEARCH      0x1     //查询
#define INT_CTRL_AUTH_COLLECTIONCREATERAWTX_CREATE      0x2     //根据选中地址创建归集交易并导出
//归集交易广播界面的子控件(按钮)的权限--5
#define INT_CTRL_AUTH_COLLECTXBROADCAST_QUERY       0x1     //查询
#define INT_CTRL_AUTH_COLLECTXBROADCAST_AFFIRMBROADCAST      0x2     //重置
#define INT_CTRL_AUTH_COLLECTXBROADCAST_IMPORT      0x4    //导入待广播文件
//大地址监控界面的子控件(按钮)的权限--6
#define INT_CTRL_AUTH_BIGACCOUNTMONITORING_QUERY     0x1     //查询
#define INT_CTRL_AUTH_BIGACCOUNTMONITORING_RESET     0x2     //重置
#define INT_CTRL_AUTH_BIGACCOUNTMONITORING_ADDMONITORINGADDR     0x4     //添加监控地址
#define INT_CTRL_AUTH_BIGACCOUNTMONITORING_DELETESELECTEDADDR    0x8     //删除选中地址
//归集日志界面权限--7
#define INT_CTRL_AUTH_COLLECTLOG_SEARCH     0x1     //查询
#define INT_CTRL_AUTH_COLLECTLOG_RESET      0x2     //重置
//自动提现列表界面的子控件(按钮)的权限--8
#define INT_CTRL_AUTH_WITHDRAW_SEARCH        0x1     //搜索
#define INT_CTRL_AUTH_WITHDRAW_RESET         0x2     //重置
#define INT_CTRL_AUTH_WITHDRAW_STOPWITHDRAW  0x4     //停止提现
//初审列表界面的子控件(按钮)的权限--9
#define INT_CTRL_AUTH_FIRSTTRIAL_SEARCH          0x1     //搜索
#define INT_CTRL_AUTH_FIRSTTRIAL_RESET           0x2     //重置
#define INT_CTRL_AUTH_FIRSTTRIAL_REJECT          0x4     //审核驳回
#define INT_CTRL_AUTH_FIRSTTRIAL_AUDIT           0x8     //审核
#define INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERASSERT 0x16     //查看用户资产
#define INT_CTRL_AUTH_FIRSTTRIAL_CHECKUSERRUNWATER 0x32     //查看用户流水
//复审列表界面的子控件(按钮)的权限--10
#define INT_CTRL_AUTH_RECHECK_SEARCH          0x1     //搜索
#define INT_CTRL_AUTH_RECHECK_RESET           0x2     //重置
#define INT_CTRL_AUTH_RECHECK_REJECT          0x4     //审核驳回
#define INT_CTRL_AUTH_RECHECK_CHECKUSERASSERT 0x8     //查看用户资产
#define INT_CTRL_AUTH_RECHECK_CHECKUSERRUNWATER 0x16  //查看用户流水
#define INT_CTRL_AUTH_RECHECK_SELECTEDRECHECKPASS 0x32//选中复审通过
//用户数据界面权限--11
#define INT_CTRL_AUTH_USERDATA_AFFIRM     0x1     //确认
#define INT_CTRL_AUTH_USERDATA_RESET      0x2     //重置
//用户流水界面权限--12
#define INT_CTRL_AUTH_USERRUNWATER_AFFIRM     0x1     //查询
#define INT_CTRL_AUTH_USERRUNWATER_FIRST      0x2     //初审
#define INT_CTRL_AUTH_USERRUNWATER_RECHECK    0x4     //复审
#define INT_CTRL_AUTH_USERRUNWATER_CHECKDATA  0x8     //查看数据
//创建未签名提币交易--13
#define INT_CTRL_AUTH_WITHDRAWCREATERAWTX_SEARCH 0x1    //查询
#define INT_CTRL_AUTH_WITHDRAWCREATERAWTX_CREATE 0x2    //根据选中订单创建未签名交易
#define INT_CTRL_AUTH_WITHDRAWCREATERAWTX_EXPORT 0x4    //导出已创建的未签名交易
//人工提币交易广播界面的子控件(按钮)的权限--14
#define INT_CTRL_AUTH_MANUALCHECKBROADCAST_SEARCH          0x1     //搜索
#define INT_CTRL_AUTH_MANUALCHECKBROADCAST_IMPORTBROADCASTFILE     0x2     //导入待广播文件
#define INT_CTRL_AUTH_MANUALCHECKBROADCAST_AFFIRMBROADCASTSELECTED  0x4     //确认广播选中
//人工提币列表界面--15
#define INT_PAGE_AUTH_WITHDRAWLIST_SEARCH      0x1      //搜索
#define INT_PAGE_AUTH_WITHDRAWLIST_RESET       0x2      //重置
#define INT_PAGE_AUTH_WITHDRAWLIST_REJECT      0x4      //驳回
//管理员类型管理界面的子控件(按钮)的权限--16
#define INT_CTRL_AUTH_ADMINTYPEMNG_SEARCH       0x1     //查询管理员类型
#define INT_CTRL_AUTH_ADMINTYPEMNG_ADD          0x2     //添加管理员类型
#define INT_CTRL_AUTH_ADMINTYPEMNG_EDIT         0x4     //编辑
#define INT_CTRL_AUTH_ADMINTYPEMNG_FREEZE       0x8     //冻结
#define INT_CTRL_AUTH_ADMINTYPEMNG_UNFREEZE     0x16    //解冻
#define INT_CTRL_AUTH_ADMINTYPEMNG_DELETE       0x32    //删除
//添加管理员类型界面的子控件(按钮)的权限--17
#define INT_CTRL_AUTH_ADDADMINTYPE_ADD       0x1     //添加
#define INT_CTRL_AUTH_ADDADMINTYPE_EDIT      0x2     //编辑
//管理员列表界面的子控件(按钮)的权限--18
#define INT_CTRL_AUTH_ADMINLIST_SEARCH       0x1     //搜索
#define INT_CTRL_AUTH_ADMINLIST_ADD          0x2     //新增管理员
#define INT_CTRL_AUTH_ADMINLIST_EDIT         0x4     //编辑
#define INT_CTRL_AUTH_ADMINLIST_RESET        0x8     //重置密码
#define INT_CTRL_AUTH_ADMINLIST_FREEZE       0x16    //冻结
#define INT_CTRL_AUTH_ADMINLIST_UNFREEZE     0x32    //解冻
#define INT_CTRL_AUTH_ADMINLIST_DELETE       0x64    //删除
//操作日志界面的子控件(按钮)的权限--19
#define INT_CTRL_AUTH_OPERATIONLOG_SEARCH    0x1     //搜索
#define INT_CTRL_AUTH_OPERATIONLOG_FREEZE    0x2     //冻结
//平台流水界面的子控件(按钮)的权限--20
#define INT_CTRL_AUTH_PLATFORMFLOW_QUERY       0x1     //查询
#define INT_CTRL_AUTH_PLATFORMFLOW_EXPORTEXCEL     0x2     //导出EXCEL文件
//日报-充值日报界面的子控件(按钮)的权限--21
#define INT_CTRL_AUTH_RECHARGEDAILYPAPER_QUERY      0x1     //查询
#define INT_CTRL_AUTH_RECHARGEDAILYPAPER_EXPORTEXCEL        0x2     //导出Excel文件
//日报-提现日报界面的子控件(按钮)的权限--22
#define INT_CTRL_AUTH_WITHDRAWDAILYPAPER_QUERY      0x1     //查询
#define INT_CTRL_AUTH_WITHDRAWDAILYPAPER_EXPORTEXCEL        0x2     //导出Excel文件
//添加币种界面权限--23
#define INT_CTRL_AUTH_ADDCOINTYPE_SEARCH        0x1     //查询
#define INT_CTRL_AUTH_ADDCOINTYPE_ADD           0x2     //添加币种
#define INT_CTRL_AUTH_ADDCOINTYPE_EDIT          0x4     //编辑币种
#define INT_CTRL_AUTH_ADDCOINTYPE_DELETE        0x8     //删除币种
//手动充值界面的子控件(按钮)的权限--24
#define INT_CTRL_AUTH_MANUALTOPUP_AFFIRMGENERATE       0x1         //确认充值
#define INT_CTRL_AUTH_MANUALTOPUP_CANCEL               0x2         //取消
//===================权限管理相关宏定义============================================

//===================数据库相关宏定义=============================================
//权限标志表表头
#define LIST_STR_TB_AUTH_FLAG QString("admin_type_id")<<QString("charge_list")<<QString("addr_count_list")<<QString("addr_detail") \
    <<QString("collection_create_raw_Tx")<<QString("collection_broadcast")<<QString("big_account_list")<<QString("collection_log") \
    <<QString("auto_withdraw")<<QString("manual_withdraw1")<<QString("manual_withdraw2")<<QString("manual_withdraw_create_raw_Tx") \
    <<QString("manual_withdraw_broadcast")<<QString("withdraw_list")<<QString("admin_type_list")<<QString("add_admin_type") \
    <<QString("admin_list")<<QString("admin_log")<<QString("exchange_statements")<<QString("exchange_charge_daily") \
    <<QString("exchange_withdraw_daily")<<QString("add_coin")<<QString("airdrop_charge")<<QString("user_assert_data")<<QString("user_statements")
//管理员表
#define LIST_STR_TB_ADMIN QString("admin_id")<<QString("admin_type_id")<<("login_name")<<QString("family_name")<<QString("tel") \
    <<QString("status")<<QString("create_time")
//币种表
#define LIST_STR_TB_COIN_TYPE QString("type_no")<<QString("coin_name")<<QString("tx_fee")<<QString("addr_count")<<QString("withdraw_src_addr")  \
    <<QString("encrypted_privkey")<<QString("decryptionPaw")
//地址表
#define LIST_STR_TB_ADDR QString("add_no")<<QString("lead_in_datetime")<<QString("coin_type")<<QString("operate_personal")<<QString("addr")

//充值表
//#define LIST_STR_TB_RECHARGE QString("charge_no")<<QString("order_id")<<QString("admin_id")<<QString("coin_type")<<QString("recharge_count")<<QString("recharge_status")  \
//    <<QString("upload_status")<<QString("src_addr")<<QString("receive_addr")<<QString("txid")<<QString("recharge_time")


#define LIST_STR_TB_SMS_NUM_SET QString("no")<<QString("phone_number_1")<<QString("phone_number_2")<<QString("phone_number_3")  \
    <<QString("phone_number_4")<<QString("phone_number_5")
//===================数据库相关宏定义=============================================


#endif // COMMAN_H
