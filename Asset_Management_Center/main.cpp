#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QSharedMemory>
#include <QFile>
#include <QFileInfo>
#include "dialogs/dialog_utils/dlg_single_lineedit.h"
#include "ccoinchargemng.h"
#include "autowithdraw.h"
#include "login_dlg.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/fileappender.h"

#include "utils.h"


#define ENABLE_LOGIN


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString strExePath = a.applicationFilePath();
    if(strExePath.contains(QRegExp("[\\x4e00-\\x9fa5]+")))
    {
        QMessageBox::warning(nullptr, QString("错误"), QString("当前程序所在路径包含中文! 请检查并修改文件夹名称为英文"), QString("确定"));
        return 0;
    }

    int iRet = -1;

    QString strTmpPath = argv[0];
    QFileInfo  fileTmpInfo(strTmpPath);
    g_qstr_MainExeName = fileTmpInfo.fileName();

    //加载qss样式
    CommonHelper::setStyle(":/white.qss");

    //限制程序只能启动一个
    QSharedMemory shared("assert_mng_center");//随便填个名字就行
    if (shared.attach())
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("检测到程序已运行.如果您刚关闭了程序,请稍等片刻再启动程序."), QString("确定"));
        return 0;
    }
    shared.create(1);


    QString strAppPath = QCoreApplication::applicationDirPath();
    QString strConfigLoc = strAppPath + "/LogFenPage.conf";
    if (QFile::exists(strConfigLoc))
    {
        Log4Qt::PropertyConfigurator::configureAndWatch(strConfigLoc);
    }
    Log4Qt::LogManager::setHandleQtMessages(true);


    //使用是局域网, 通过  utils::Ping("192.168.10.1") 来检查即可
    g_bIsLAN =  utils::Ping("192.168.10.1")  || utils::Ping("192.168.10.81");
    if(!g_bIsLAN && !utils::Ping("114.114.114.114"))
    {
        QMessageBox::warning(nullptr, QString("网络错误"), QString("网络错误, 请检查网络连接!"), QString("确定"));
        return 0;
    }

//    auto float111 = float100(QString("50000000000.12345678").toStdString());
//    auto float222 = float100(QString("50000000000.12345677").toStdString());
//    iRet = float111 > float222;
//    QMessageBox::information(nullptr, "dsdsd", QString("%1").arg(iRet));


#ifdef ENABLE_LOGIN

    if (3 <= argc)
    {
        //命令行传参登陆
        CLoginDlg loginDlg;
        loginDlg.fastToLogin(argv[1], argv[2]);
        int iRetLogin = loginDlg.GetResultValue();
        if (0 == iRetLogin)
        {
            return 0;
        }
    }
    else    //正常登陆
    {
        CLoginDlg loginDlg;
        loginDlg.setWindowFlags(loginDlg.windowFlags() | Qt::WindowStaysOnTopHint); //设置窗口总在最前面
        loginDlg.exec();
        int iRetLogin = loginDlg.GetResultValue();
        if (0 == iRetLogin)
        {
            return 0;
        }
    }

#endif


    CMainWindow w;
    g_ptr_qprocess_Bitcoind = new QProcess(&w);
    g_ptr_qprocess_Omnicored = new QProcess(&w);
    g_ptr_qprocess_Litecoind = new QProcess(&w);
    g_ptr_qprocess_Bchcoind = new QProcess(&w);
    //g_ptr_qprocess_Bsvcoind = new QProcess(&w);
    g_ptr_qprocess_Dashd = new QProcess(&w);
    g_ptr_qprocess_Rippled = new QProcess(&w);
    g_ptr_qprocess_MoneroWalletRpc = new QProcess(&w);

    try
    {
        QString strErrMsg;
        iRet = InitEnvironment(strErrMsg);
        if(0 != iRet)
        {
            throw runtime_error(strErrMsg.toStdString());
        }



        //初始化Mainwindow
        iRet = w.Init();
        if(0 != iRet)
        {
            strErrMsg = "程序初始化失败";
            throw runtime_error(strErrMsg.toStdString());
        }


        //如果开启了 充币 或 提币开关 ,  需要获取 pem文件的解密密码
        if(g_bEnableWithdraw || g_bEnableCharge)
        {
            std::shared_ptr<CDlg_single_lineedit> spDlg( new CDlg_single_lineedit(QString("检测到充币或提币配置已开,\r\n请输入私钥文件密码:"), QString("输入密码"), NULL,  QLineEdit::Password) );
            for(int i = 0; i < 4; i++)
            {
                spDlg->Clear();
                iRet = spDlg->exec();
                if(QDialog::Rejected == iRet)
                {
                    throw runtime_error("检测到充币或提币配置已开,但是获取私钥文件密码失败,程序启动终止.");
                }

                if(QDialog::Accepted == iRet)
                {
                    //验证密码是否正确
                    QString strPwd = spDlg->m_strLineText.trimmed();
                    if(!IsValidPEMFilePassword(strPwd, strErrMsg))
                    {
                        //三次机会
                        if(2 == i)
                        {
                            //throw runtime_error(strErrMsg.toStdString());
                            throw runtime_error("检测到充币或提币配置已开,但是获取私钥文件密码失败,程序启动终止.");
                        }
                        QMessageBox::warning(spDlg.get(), QString("错误"), strErrMsg);
                    }
                    else //密码正确
                    {
                        g_qstr_PrivFilePwd = strPwd;
                        break;
                    }
                }
            }
        }


        //优化 :  如果开启了充提币开关, 才创建 充提币线程对象, 否则不创建
        CCoinChargeMng *pCoinChargeMng = NULL;
        if(true == g_bEnableCharge)
        {
            pCoinChargeMng = new CCoinChargeMng();
            pCoinChargeMng->Init();
            pCoinChargeMng->start();
        }

        CAutoWithdraw  * pAutoWithdraw = NULL;
        if(true == g_bEnableWithdraw)
        {
            pAutoWithdraw = new CAutoWithdraw();
            pAutoWithdraw->Init();
            pAutoWithdraw->start();
        }


        w.show();
        iRet = a.exec();


        if(true == g_bEnableWithdraw)
        {
            CAutoWithdraw::ms_bExitThread = true;
            if(NULL != pAutoWithdraw)
            {
                pAutoWithdraw->wait();
                delete pAutoWithdraw;
                pAutoWithdraw = NULL;
            }
        }

        if(true == g_bEnableCharge)
        {
            // 等待 2 个 30秒(http请求超时时间)
            pCoinChargeMng->StopAll(2 * 30 * 1000);
            if(NULL != pCoinChargeMng)
            {
                delete pCoinChargeMng;
                pCoinChargeMng = NULL;
            }
        }

    }
    catch(std::exception &e)
    {
        QMessageBox::warning(nullptr, QString("错误"), QString("错误:%1").arg(e.what()), QString("确定"));
    }
    catch(...)
    {
        QMessageBox::warning(nullptr, QString("错误"), QString("未知错误"));
    }


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
