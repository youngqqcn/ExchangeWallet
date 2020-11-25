#include "mainwindow.h"
#include "logger.h"
#include "config.h"
#include <QApplication>
#include <QProcess>
#include "utils.h"




int main(int argc, char *argv[])
{

#ifdef LOG_TO_FILE
    qInstallMessageHandler(OutputMessageHandler);
#endif

    QApplication a(argc, argv);

    QString strExePath = a.applicationFilePath();
    if(strExePath.contains(QRegExp("[\\x4e00-\\x9fa5]+")))
    {
        QMessageBox::warning(nullptr, QString("错误"), QString("当前程序所在路径包含中文! 请检查并修改文件夹名称为英文"), QString("确定"));
        return 0;
    }


    if(utils::Ping("114.114.114.114") )
    {
        int iSel = QMessageBox::warning(nullptr, QString("严重安全警告:"),
                        QString("检测到电脑已联网,为了保护私钥安全,请在断网隔离的专用电脑上使用本软件! "),
                        QString("我自行承担安全风险,继续使用(不推荐)"),
                        QString(" 安全退出 (推荐) "),
                        QString(),
                        1 /*默认  退出*/
                        );
        if(1 == iSel) return 0;
    }


    //加载qss样式
    CommonHelper::setStyle(":/white.qss");


    CMainWindow w;
    g_ptr_qprocess_Bitcoind = new QProcess(&w);
    g_ptr_qprocess_Omnicored = new QProcess(&w);
    g_ptr_qprocess_Litecoind = new QProcess(&w);
    g_ptr_qprocess_Bchcoind = new QProcess(&w);
    //g_ptr_qprocess_Bsvcoind = new QProcess(&w);
    g_ptr_qprocess_Dashd = new QProcess(&w);
    g_ptr_qprocess_Rippled = new QProcess(&w);
    g_ptr_qprocess_MoneroWalletRpc = new QProcess(&w);

    int iRet = 0;
    try
    {
        QString strErrMsg;
        iRet = InitEnvironment(strErrMsg);
        if(0 != iRet)
        {
            throw runtime_error(strErrMsg.toStdString());
        }


        iRet = w.Init();
        if(0 != iRet)
        {
            strErrMsg = "程序初始化失败";
            throw runtime_error(strErrMsg.toStdString());
        }

        w.show();

        iRet = a.exec();
    }
    catch(std::exception &e)
    {
        QMessageBox::warning(nullptr, QString("错误"), QString("错误:%1").arg(e.what()), QString("确定"));
    }
    catch(...)
    {
        QMessageBox::warning(nullptr, QString("错误"), QString("未知错误"));
    }


    //关闭bitcoind.exe   omnicored.exe  litecoind.exe
     //关闭bitcoind.exe   omnicored.exe
    g_ptr_qprocess_Bitcoind->kill();
    g_ptr_qprocess_Omnicored->kill();
    g_ptr_qprocess_Litecoind->kill();
    g_ptr_qprocess_Bchcoind->kill();
    //g_ptr_qprocess_Bsvcoind->kill();
    g_ptr_qprocess_Dashd->kill();
    g_ptr_qprocess_Rippled->kill();
    g_ptr_qprocess_MoneroWalletRpc->kill();
    delete g_ptr_qprocess_Bitcoind;
    delete g_ptr_qprocess_Omnicored;
    delete g_ptr_qprocess_Litecoind;
    delete g_ptr_qprocess_Bchcoind;
    //delete g_ptr_qprocess_Bsvcoind;
    delete g_ptr_qprocess_Dashd;
    delete g_ptr_qprocess_Rippled;
    delete g_ptr_qprocess_MoneroWalletRpc;
    g_ptr_qprocess_Bitcoind = NULL;
    g_ptr_qprocess_Omnicored = NULL;
    g_ptr_qprocess_Litecoind = NULL;
    g_ptr_qprocess_Bchcoind = NULL;
    //g_ptr_qprocess_Bsvcoind = NULL;
    g_ptr_qprocess_Dashd = NULL;
    g_ptr_qprocess_Rippled = NULL;
    g_ptr_qprocess_MoneroWalletRpc = NULL;


    QThread::sleep(5);

    return 0;
}
