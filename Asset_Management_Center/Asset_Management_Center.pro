#-------------------------------------------------
#
# Project created by QtCreator 2019-02-21T19:21:50
#
#-------------------------------------------------

QT       += core gui xml sql websockets axcontainer
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Asset_Management_Center
TEMPLATE = app
#指定目标文件(obj)的存放目录
OBJECTS_DIR += $$PWD/bin/build/objects

#使用 Log4Qt
include(log4qt/log4qt.pri)
INCLUDEPATH += log4qt



# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

##增 恒星币XLM的 SDK 支持
DEFINES += STELLAR_ALLOW_UNSECURE_RANDOM
DEFINES += STELLAR_SKIP_LIVE_TESTS
include(StellarQtSDK/StellarQtSDK.pri)

RC_FILE += app.rc

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#在此添加你的模块的路径
INCLUDEPATH += modules\logger
INCLUDEPATH += thirdmodules\navlistview
INCLUDEPATH += dialogs\page_ChargeList

INCLUDEPATH += modules\coinCharge modules\coinCharge\AM_CoinCharge modules\coinCharge\AM_TxStatusMonitor
INCLUDEPATH += dialogs\page_AddrList dialogs\page_AddCoin
INCLUDEPATH += dialogs\page_ExchangeStatements dialogs\page_ChargeDaily dialogs\page_WithdrawDaily
INCLUDEPATH += dialogs\page_AdminTypeList dialogs\page_AddAdminType dialogs\page_ManualWithdraw
INCLUDEPATH += dialogs\page_AdminList
INCLUDEPATH += modules\coinWithdraw
INCLUDEPATH += modules\rawtx
INCLUDEPATH += dialogs\page_Collection
INCLUDEPATH += dialogs\page_OperateLog
INCLUDEPATH += modules\login
INCLUDEPATH += modules\auth_set
INCLUDEPATH += dialogs
INCLUDEPATH += modules\encryptionModule
INCLUDEPATH += modules\sms

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    thirdmodules/navlistview/navlistview.cpp \
    dialogs/am_dialog_base.cpp \
    am_main.cpp \
    dialogs/page_ChargeList/short_msgsetting_dlg.cpp \
    dialogs/page_ChargeList/page_charge_list.cpp \
    modules/coinCharge/ccoinchargemng.cpp \
    modules/coinCharge/AM_CoinCharge/coincharge.cpp \
    modules/coinCharge/AM_TxStatusMonitor/txstatusmonitor.cpp \
    dialogs/page_AddrList/import_addr_dlg.cpp \
    dialogs/page_AddrList/page_addr_list.cpp \
    dialogs/page_AddCoin/add_coin_dlg.cpp \
    dialogs/page_AddCoin/delete_coin_dlg.cpp\
    dialogs/page_AddCoin/page_add_coin.cpp \
    dialogs/page_ChargeDaily/page_charge_daily.cpp \
    dialogs/page_WithdrawDaily/page_withdraw_daily.cpp \
    dialogs/page_ExchangeStatements/page_exchange_statements.cpp \
    dialogs/page_AdminTypeList/page_admin_type_list.cpp \
    dialogs/page_AddAdminType/page_add_admin_type.cpp \
    dialogs/page_AdminList/page_admin_list.cpp \
    dialogs/page_AdminList/add_admin_dlg.cpp \
    dialogs/page_AdminList/reset_pwd_dlg.cpp \
    dialogs/page_Collection/page_collection.cpp \
    dialogs/page_ManualWithdraw/page_manual_withdraw.cpp \
    mainwindow_collection.cpp \
    mainwindow_manual_withdraw.cpp \
    modules/coinWithdraw/autowithdraw.cpp \
    modules/rawtx/btc/btc_off_sig.cpp \
    modules/rawtx/btc/btc_raw_tx_util.cpp \
    dialogs/page_OperateLog/page_operate_log.cpp \
    modules/rawtx/usdt/usdt_off_sig.cpp \
    modules/rawtx/usdt/usdt_raw_tx_util.cpp \
    modules/login/login_dlg.cpp \
    modules/login/user_info.cpp \
    modules/login/login_operate.cpp \
    modules/auth_set/auth_setting.cpp   \
    dialogs/page_ManualWithdraw/manualconfirmdialog.cpp \
    modules/rawtx/eth/eth_raw_tx_utils.cpp \
    modules/rawtx/raw_tx_utils_base.cpp \
    dialogs/page_Collection/ccollectionconfirmdialog.cpp \
    dialogs/page_Collection/add_big_addr_dlg.cpp \
    dialogs/page_Collection/delete_big_addr_dlg.cpp \
    modules/encryptionModule/encryption_module.cpp \
    modules/rawtx/cosmos/CCosmos.cpp \
    modules/rawtx/cosmos/cosmos_raw_tx_utils.cpp \
    config.cpp \
    dialogs/page_Collection/ask_paytxfee_addr_dlg.cpp \
    get_login_user.cpp \
    modules/encryptionModule/crypto_utils.cpp \
    dialogs/dialog_utils/dlg_single_lineedit.cpp \
    modules/rawtx/btc/btc_raw_tx_utils.cpp \
    modules/rawtx/usdt/usdt_raw_tx_utils.cpp \
    dialogs/page_ChargeList/audit_dlg.cpp \
    modules/paging/multi_page.cpp \
    modules/sms/sms.cpp \
    modules/rawtx/xrp/xrp_raw_tx_utils.cpp \
    modules/rawtx/eos/eos_raw_tx_utils.cpp \
    modules/rawtx/xlm/xlm_raw_tx_utils.cpp \
    modules/rawtx/trx/trx_raw_tx_utils.cpp \
    modules/rawtx/xmr/xmr_raw_tx_utils.cpp \
    modules/rawtx/cosmos/bech32/bech32.cpp



HEADERS += \
        mainwindow.h \
    comman.h \
    thirdmodules/navlistview/navlistview.h \
    dialogs/am_dialog_base.h \
    am_main.h \
    dialogs/page_ChargeList/short_msg_setting_dlg.h \
    dialogs/page_ChargeList/page_charge_list.h \
    modules/coinCharge/ccoinchargemng.h \
    modules/coinCharge/coinchargecomman.h \
    modules/coinCharge/AM_CoinCharge/coincharge.h \
    modules/coinCharge/AM_TxStatusMonitor/txstatusmonitor.h \
    dialogs/page_AddrList/import_addr_dlg.h \
    dialogs/page_AddrList/page_addr_list.h \
    dialogs/page_AddCoin/add_coin_dlg.h \
    dialogs/page_AddCoin/delete_coin_dlg.h\
    dialogs/page_AddCoin/page_add_coin.h \
    dialogs/page_ChargeDaily/page_charge_daily.h \
    dialogs/page_WithdrawDaily/page_withdraw_daily.h \
    dialogs/page_ExchangeStatements/page_exchange_statements.h \
    dialogs/page_AdminTypeList/page_admin_type_list.h \
    dialogs/page_AddAdminType/page_add_admin_type.h \
    dialogs/page_AdminList/page_admin_list.h \
    dialogs/page_AdminList/add_admin_dlg.h \
    dialogs/page_AdminList/reset_pwd_dlg.h \
    dialogs/page_ManualWithdraw/page_manual_withdraw.h \
    dialogs/page_Collection/page_collection.h \
    modules/coinWithdraw/autowithdraw.h \
    modules/rawtx/btc/btc_off_sig.h \
    modules/rawtx/btc/btc_raw_tx_util.h \
    modules/rawtx/rawtx_comman.h \
    dialogs/page_OperateLog/page_operate_log.h \
    modules/rawtx/usdt/usdt_off_sig.h \
    modules/rawtx/usdt/usdt_raw_tx_util.h \
    modules/login/login_dlg.h \
    modules/login/user_info.h \
    modules/login/login_operate.h \
    modules/auth_set/auth_setting.h \
    dialogs/page_ManualWithdraw/manualconfirmdialog.h \
    modules/rawtx/eth/eth_raw_tx_utils.h \
    modules/rawtx/raw_tx_utils_base.h \
    dialogs/page_Collection/ccollectionconfirmdialog.h \
    dialogs/page_Collection/add_big_addr_dlg.h \
    dialogs/page_Collection/delete_big_addr_dlg.h \
    modules/encryptionModule/encryption_module.h \
    modules/rawtx/cosmos/CCosmos.h \
    modules/rawtx/cosmos/cosmos_raw_tx_utils.h \
    config.h \
    utils.h \
    dialogs/page_Collection/ask_paytxfee_addr_dlg.h \
    get_login_user.h \
    modules/encryptionModule/crypto_utils.h \
    dialogs/dialog_utils/dlg_single_lineedit.h \
    modules/rawtx/btc/btc_raw_tx_utils.h \
    modules/rawtx/usdt/usdt_raw_tx_utils.h \
    dialogs/page_ChargeList/audit_dlg.h \
    modules/paging/multi_page.h \
    modules/sms/sms.h \
    modules/rawtx/xrp/xrp_raw_tx_utils.h \
    modules/rawtx/eos/eos_raw_tx_utils.h \
    modules/rawtx/xlm/xlm_raw_tx_utils.h \
    modules/rawtx/trx/trx_raw_tx_utils.h \
    modules/rawtx/xmr/xmr_raw_tx_utils.h \
    modules/rawtx/cosmos/bech32/bech32.h


FORMS += \
        mainwindow.ui \
    dialogs/page_ChargeList/short_msgsetting_dlg.ui \
    dialogs/page_AddrList/import_addr_dlg.ui \
    dialogs/page_AddCoin/add_coin_dlg.ui \
    dialogs/page_AddCoin/delete_coin_dlg.ui \
    dialogs/page_AdminList/add_admin_dlg.ui \
    dialogs/page_AdminList/reset_pwd_dlg.ui \
    modules/login/login_dlg.ui \
    dialogs/page_ManualWithdraw/manualconfirmdialog.ui \
    dialogs/page_Collection/ccollectionconfirmdialog.ui \
    dialogs/page_Collection/add_big_addr_dlg.ui \
    dialogs/page_Collection/delete_big_addr_dlg.ui \
    dialogs/page_Collection/ask_paytxfee_addr_dlg.ui \
    dialogs/dialog_utils/dlg_single_lineedit.ui \
    dialogs/page_ChargeList/audit_dlg.ui \
    modules/paging/multi_page.ui

CONFIG += qt warn_on


CONFIG+=debug_and_release
DEFINES += G_STR_EXE_VERSION=\\\"AM-v0.32.1.x(2min+rand+special)\\\"
#DEFINES += BTC_SERIES_EX
CONFIG(debug, debug|release){#debug版
    DEFINES += G_STR_CONFIG_FILE_PATH=\\\"../../config.ini\\\"
    DEFINES += G_STR_A_PRIV_PEM_FILE_PATH=\\\"../../A_priv.pem\\\"
    DEFINES += G_STR_B_PUB_PEM_FILE_PATH=\\\"../../B_pub.pem\\\"
    DESTDIR= $$PWD/bin/debug
}else{#release版,可在此添加宏定义
    DEFINES += G_STR_CONFIG_FILE_PATH=\\\"./config.ini\\\"
    DEFINES += G_STR_A_PRIV_PEM_FILE_PATH=\\\"./A_priv.pem\\\"
    DEFINES += G_STR_B_PUB_PEM_FILE_PATH=\\\"./B_pub.pem\\\"
    DESTDIR= $$PWD/bin/release
}

#配置静态库路径, 此处用到了以太坊签名动静态库(隐式调用)
#注意: 编译发行的时候 注意将动态库放到 exe同一目录
win32:INCLUDEPATH +="$$PWD/include"
win32:INCLUDEPATH += "D:\boost_1_69_0"
win32:DEPENDPATH +="$$PWD/lib"
win32: LIBS += "-L$$PWD/lib" -llibethsig -llibeossig -llibtrxsig -llibeay32  -llibsecp256k1 -lssleay32

#尝试捕获  LIBEAY32.dll 崩溃
win* {
QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
QMAKE_CXXFLAGS_STL_ON = /EHa
}


RESOURCES += \
    resource/res.qrc
