QT += core gui
CONFIG += c++11 #console
CONFIG -= app_bundle

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#使用 Log4Qt
include(log4qt/log4qt.pri)
INCLUDEPATH += log4qt


#增 恒星币XLM的 SDK 支持
DEFINES += STELLAR_ALLOW_UNSECURE_RANDOM
DEFINES += STELLAR_SKIP_LIVE_TESTS
include(StellarQtSDK/StellarQtSDK.pri)

#指定目标文件(obj)的存放目录
OBJECTS_DIR += $$PWD/bin/build/objects

CONFIG+=debug_and_release
CONFIG(debug, debug|release){
    DESTDIR = $$PWD/bin/debug
}else{
    DESTDIR = $$PWD/bin/release
}

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += -I D:\Python27_x86\include
LIBS += -LD:\Python27_x86\libs -lpython27


SOURCES += main.cpp \
    generate_addr_tool.cpp

FORMS += \
    generate_addr_tool.ui

HEADERS += \
    generate_addr_tool.h

DISTFILES += \
    btc_addr_gen.py \
    dash_addr_gen.py \
    eth_addr_gen.py \
    htdf_addr_gen.py \
    ltc_addr_gen.py \
    usdp_addr_gen.py \
    xrp_addr_gen.py \
    eos_addr_gen.py \
    trx_addr_gen.py
