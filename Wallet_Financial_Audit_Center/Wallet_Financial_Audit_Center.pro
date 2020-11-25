#-------------------------------------------------
#
# Project created by QtCreator 2019-02-21T19:22:40
#
#-------------------------------------------------

QT       += core gui sql
QT += network
QT += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Wallet_Financial_Audit_Center
TEMPLATE = app



#QMAKE_CXXFLAGS += -Wno-unused-parameter
#QMAKE_CXXFLAGS += -Wno-unused-variable
#QMAKE_CXXFLAGS += -Wno-unused-function
#QMAKE_CXXFLAGS += -Wno-comment
#QMAKE_CXXFLAGS += -Wno-sign-compare




#指定目标文件(obj)的存放目录
OBJECTS_DIR += $$PWD/bin/build/objects

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


##增 恒星币XLM的 SDK 支持
DEFINES += STELLAR_ALLOW_UNSECURE_RANDOM
DEFINES += STELLAR_SKIP_LIVE_TESTS
include(StellarQtSDK/StellarQtSDK.pri)

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += LOG_TO_FILE
DEFINES += AM_STR_LOG_FILE_PATH=\\\"wm_log.txt\\\"



INCLUDEPATH +=  modules\logger  dialogs\dlg_sig_confirm  modules\rawtx
INCLUDEPATH += modules\sms

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dialogs/dlg_sig_confirm/sig_confirm_dlg.cpp \
    modules/rawtx/eth/eth_raw_tx_utils.cpp \
    modules/rawtx/raw_tx_utils_base.cpp \
    modules/rawtx/btc/btc_off_sig.cpp \
    modules/rawtx/btc/btc_raw_tx_util.cpp \
    modules/rawtx/usdt/usdt_off_sig.cpp \
    modules/rawtx/usdt/usdt_raw_tx_util.cpp \
    modules/rawtx/cosmos/CCosmos.cpp \
    modules/rawtx/cosmos/cosmos_raw_tx_utils.cpp \
    config.cpp \
    modules/sms/sms.cpp \
    modules/rawtx/xrp/xrp_raw_tx_utils.cpp \
    modules/rawtx/eos/eos_raw_tx_utils.cpp \
    modules/rawtx/xlm/xlm_raw_tx_utils.cpp \
    modules/rawtx/trx/trx_raw_tx_utils.cpp \
    modules/rawtx/xmr/xmr_raw_tx_utils.cpp \
    modules/rawtx/cosmos/bech32/bech32.cpp


HEADERS += \
        mainwindow.h \
    modules/logger/logger.h \
    dialogs/dlg_sig_confirm/sig_confirm_dlg.h \
    modules/rawtx/eth/eth_raw_tx_utils.h \
    modules/rawtx/raw_tx_utils_base.h \
    modules/rawtx/rawtx_comman.h \
    modules/rawtx/btc/btc_off_sig.h \
    modules/rawtx/btc/btc_raw_tx_util.h \
    modules/rawtx/usdt/usdt_off_sig.h \
    modules/rawtx/usdt/usdt_raw_tx_util.h \
    modules/rawtx/cosmos/CCosmos.h \
    modules/rawtx/cosmos/cosmos_raw_tx_utils.h \
    config.h \
    comman.h \
    utils.h \
    modules/logger/logger.h \
    modules/sms/sms.h \
    modules/rawtx/xrp/xrp_raw_tx_utils.h \
    modules/rawtx/eos/eos_raw_tx_utils.h \
    modules/rawtx/xlm/xlm_raw_tx_utils.h \
    modules/rawtx/trx/trx_raw_tx_utils.h \
    modules/rawtx/xmr/xmr_raw_tx_utils.h \
    modules/rawtx/cosmos/bech32/bech32.h

FORMS += \
        mainwindow.ui \
    dialogs/dlg_sig_confirm/sig_confirm_dlg.ui

CONFIG += qt warn_on


DEFINES += G_STR_EXE_VERSION=\\\"WM.v0.32.1.x(dynFee)\\\"
CONFIG +=debug_and_release
CONFIG(debug, debug|release){#debug版
    #debug的结果运行程序放这里
    DEFINES += G_STR_CONFIG_FILE_PATH=\\\"../../config.ini\\\"
    DESTDIR= $$PWD/bin/debug
}else{
    #release的结果运行程序放这里
    DEFINES += G_STR_CONFIG_FILE_PATH=\\\"./config.ini\\\"
    DESTDIR = $$PWD/bin/release
}

CONFIG += C++11

#配置静态库路径, 此处用到了以太坊签名动静态库(隐式调用)
#注意: 编译发行的时候 注意将动态库放到 exe同一目录
win32:INCLUDEPATH +="$$PWD/include"
win32:INCLUDEPATH += "D:\boost_1_69_0"
win32:DEPENDPATH +="$$PWD/lib"
win32: LIBS += "-L$$PWD/lib" -llibethsig  -llibeossig -llibtrxsig -llibeay32  -llibsecp256k1 -lssleay32

RESOURCES += \
    resource/res.qrc

