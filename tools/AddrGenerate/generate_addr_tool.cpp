#pragma execution_character_set("utf-8")

#include "generate_addr_tool.h"
#include "ui_generate_addr_tool.h"
#include <Python.h>
#include <QDebug>
#include <QMessageBox>
#include <QRegExp>
#include <QDateTime>
#include <QProgressDialog>
#include <QTextCodec>
#include <QFile>
#include <QDir>
#include <QDialog>
#include <QListView>

//使用 恒星币的 SDK
#include "src/keypair.h"
#include "src/util.h"


//匹配地址数量格式
#define REGEXP_ADDRCOUNT "^[1-9]*[0-9][0-9]*$"
//匹配key
#define REGEXP_ENCODEKEY "^\\w{10,10}$"

GenerateAddrTool::GenerateAddrTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GenerateAddrTool)
{
    ui->setupUi(this);
    //设置标题
    this->setWindowTitle("地址生成工具_2020_02_17-v7.0[增加TRX(波场)地址生成]");

    ui->cbx_coinType->setView(new QListView());

    ui->checkBox_isTestNet->setChecked(false);

    __m_bIsTestNet = false;
}

GenerateAddrTool::~GenerateAddrTool()
{
    delete ui;
}

void GenerateAddrTool::__setBtnStatus(const int iFlag)
{
    if (0 == iFlag)
    {
        ui->btn_startGenAddr->setEnabled(false);
        ui->btn_startGenAddr->setText("生成地址中...");
        ui->cbx_coinType->setEnabled(false);
        ui->led_addrCount->setEnabled(false);
        if (0 != __m_vctAddrInfoEncode.size())
            __m_vctAddrInfoEncode.clear();
        if (0 != __m_vctAddrInfoSrc.size())
            __m_vctAddrInfoSrc.clear();
    }
    else
    {
        ui->btn_startGenAddr->setEnabled(true);
        ui->btn_startGenAddr->setText("开始生成");
        ui->cbx_coinType->setEnabled(true);
        ui->led_addrCount->setEnabled(true);
    }
}

int GenerateAddrTool::__parsePyResult(PyObject *pPyRet, vector<addrInfo> &vctAddrInfo)
{
    for (Py_ssize_t i = 0; i < PyList_Size(pPyRet); i++)
    {
        PyObject *poAddrTuple = PyList_GetItem(pPyRet, i);
        addrInfo tmpAddr;
        for (Py_ssize_t i = 0; i < PyTuple_Size(poAddrTuple); i++)
        {
            PyObject *item = PyTuple_GetItem(poAddrTuple, i);
            QString strTmp;
            strTmp += "listToVector_Addr --->";
            strTmp += PyString_AsString(item);
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strTmp;
            if (0 == i) tmpAddr.strPrivKey = PyString_AsString(item);
            if (1 == i) tmpAddr.strPubKey = PyString_AsString(item);
            if (2 == i) tmpAddr.strAddr = PyString_AsString(item);
            tmpAddr.strCoinType = ui->cbx_coinType->currentText();
        }
        vctAddrInfo.push_back(tmpAddr);
    }
    return 0;
}

/*
* @Name:加密
* 算法:先对私钥进行base64 --> 异或一次 --> 异或第二次
*/
int GenerateAddrTool::__encodePrivKey()
{
    QString strSrcAddrPrivKey = "";
    if (0 <= __m_vctAddrInfoSrc.size())
    {
        for (uint i = 0; i < __m_vctAddrInfoSrc.size(); i++)
        {
            strSrcAddrPrivKey.clear();
            strSrcAddrPrivKey = __m_strEncodeKey + __m_vctAddrInfoSrc[i].strPrivKey;
            QByteArray base64PrivKey = strSrcAddrPrivKey.toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
            QString strBase64PrivKey = base64PrivKey;
            QString strFirstXor = __xorEncode(strBase64PrivKey, FIRST_ENCODE);
//            qDebug() << "第一次异或之后的私钥: " << strFirstXor;
            QString strSecondXor = __xorEncode(strFirstXor, SECOND_ENCODE);
//            qDebug() << "第二次异或之后的私钥: " << strFirstXor;
            base64PrivKey = strSecondXor.toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
            QString strLastEncodePrivKey = base64PrivKey;
            addrInfo encodedAddrInfo;
            encodedAddrInfo.strCoinType = __m_vctAddrInfoSrc[i].strCoinType;
            encodedAddrInfo.strAddr = __m_vctAddrInfoSrc[i].strAddr;
            encodedAddrInfo.strPrivKey = strLastEncodePrivKey;
            encodedAddrInfo.strPubKey = __m_vctAddrInfoSrc[i].strPubKey;
            __m_vctAddrInfoEncode.push_back(encodedAddrInfo);
//            qDebug() << "加密之后的地址私钥信息: " << strSecondXor;
        }
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请先生成地址!";
        return -1;
    }
    return 0;
}

int GenerateAddrTool::__decodePrivKey()
{
    if (0 < __m_vctAddrInfoEncode.size())
    {
        for (uint i = 0; i < __m_vctAddrInfoEncode.size(); i++)
        {
            QByteArray base64PrivKey = __m_vctAddrInfoEncode[i].strPrivKey.toUtf8().fromBase64(__m_vctAddrInfoEncode[i].strPrivKey.toUtf8());
            QString strBase64PrivKeyTmp = base64PrivKey;
            QString strSecondXor = __xorEncode(strBase64PrivKeyTmp, SECOND_ENCODE);
            QString strFirstXor = __xorEncode(strSecondXor, FIRST_ENCODE);
            base64PrivKey = strFirstXor.toUtf8().fromBase64(strFirstXor.toUtf8());
            QString strPrivKey = base64PrivKey;
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "解密之后的私钥: " << strPrivKey.mid(10);
        }
    }
    else
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "请先生成地址!";
        return -1;
    }
    return 0;
}

QString GenerateAddrTool::__xorEncode(QString strSrc, const QChar cKey)
{
    QByteArray src;
    if(strSrc.length() > 0)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        src = codec->fromUnicode(strSrc);
    }

    for (int i = 0; i < src.size(); i++)
    {
        src[i] = src[i] ^ cKey.toLatin1();
    }

    QString strResult;
    if(src.size() > 0)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        strResult = codec->toUnicode(src);
    }

//    for(int i = 0; i < strSrc.count(); i++)
//    {
//        strSrc[i] = strSrc.at(i).toLatin1() ^ cKey.toLatin1();
//    }
//    qDebug() << "异或之后的私钥: " << strResult;
    return strResult;
}

int GenerateAddrTool::__exportAddrToTxt()
{
    QDateTime currentDatetime = QDateTime::currentDateTime();
    QString strCurrentDatetime = currentDatetime.toString("yyyy-MM-dd HH-mm-ss");
    QString strIsTestNet = "";
    if (false == __m_bIsTestNet)
    {
        strIsTestNet = QString("主网");
    }
    else if (true == __m_bIsTestNet)
    {
        strIsTestNet = QString("测试网");
    }
    QFile writeFile(QString("./%1/%2_%3地址_%4(不含私钥).txt").arg(ui->cbx_coinType->currentText()).arg(ui->cbx_coinType->currentText())
                    .arg(strIsTestNet).arg(strCurrentDatetime));
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "打开TXT文件失败!";
        return 1;
    }
    for (uint i = 0; i < __m_vctAddrInfoEncode.size(); i++)
    {
        QString strAddrInfo = "";
        if (0 == __m_vctAddrInfoEncode[i].strCoinType.compare("EOS", Qt::CaseInsensitive))
            strAddrInfo = __m_vctAddrInfoEncode[i].strCoinType + "\t" + __m_vctAddrInfoEncode[i].strAddr + "\t"
                    + __m_vctAddrInfoEncode[i].strPubKey + "\n";
        else
            strAddrInfo = __m_vctAddrInfoEncode[i].strCoinType + "\t" + __m_vctAddrInfoEncode[i].strAddr + "\n"/*"\t"
                + __m_vctAddrInfoEncode[i].strPrivKey + "\n"*/;
        QTextStream txtWrite(&writeFile);
        txtWrite << strAddrInfo;
    }
    writeFile.close();

    return 0;
}

int GenerateAddrTool::__exportAddrToText()
{
    QDateTime currentDatetime = QDateTime::currentDateTime();
    QString strCurrentDatetime = currentDatetime.toString("yyyy-MM-dd HH-mm-ss");
    QString strIsTestNet = "";
    if (false == __m_bIsTestNet)
    {
        strIsTestNet = QString("主网");
    }
    else if (true == __m_bIsTestNet)
    {
        strIsTestNet = QString("测试网");
    }

    QString strPath = QString("./%1/%2_%3地址_%4(含私钥).txt").arg(ui->cbx_coinType->currentText()).arg(ui->cbx_coinType->currentText())
            .arg(strIsTestNet).arg(strCurrentDatetime);
    QStringList strlistPath = strPath.split("/");
    QDir *dir = new QDir();
    if (!dir->exists(strlistPath.at(1)))
    {
        dir->mkpath(strlistPath.at(1));
    }

    QFile writeFile(strPath);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "打开TXT文件失败!";
        return 1;
    }
    for (uint i = 0; i < __m_vctAddrInfoSrc.size(); i++)
    {
        QString strAddrInfo = __m_vctAddrInfoSrc[i].strCoinType + "\t" + __m_vctAddrInfoSrc[i].strAddr + "\t"
                + __m_vctAddrInfoSrc[i].strPrivKey + "\n";
        QTextStream txtWrite(&writeFile);
        txtWrite << strAddrInfo;
    }
    writeFile.close();
    delete dir;
    dir = NULL;

    return 0;
}

int GenerateAddrTool::__exportAddrToBin()
{
    //写二进制文件
    QDateTime currentDatetime = QDateTime::currentDateTime();
    QString strCurrentDatetime = currentDatetime.toString("yyyy-MM-dd HH-mm-ss");
    QString strIsTestNet = "";
    if (false == __m_bIsTestNet)
    {
        strIsTestNet = QString("主网");
    }
    else if (true == __m_bIsTestNet)
    {
        strIsTestNet = QString("测试网");
    }
    QFile writeFile(QString("./%1/%2_%3地址_%4(含私钥).dat").arg(ui->cbx_coinType->currentText()).arg(ui->cbx_coinType->currentText())
                    .arg(strIsTestNet).arg(strCurrentDatetime));
    if (!writeFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "二进制文件打开失败!";
        return 1;
    }
    for (uint i = 0; i < __m_vctAddrInfoEncode.size(); i++)
    {
        QDataStream write(&writeFile);
        QString strAddrInfo = __m_vctAddrInfoEncode[i].strCoinType + "\t" + __m_vctAddrInfoEncode[i].strAddr + "\t"
                + __m_vctAddrInfoEncode[i].strPrivKey + "\n";
        write << strAddrInfo;
    }
    writeFile.close();

    return 0;
}

void GenerateAddrTool::on_btn_startGenAddr_clicked()
{
    __setBtnStatus(0);
    if (ui->led_addrCount->text().isEmpty() || 0 == ui->cbx_coinType->currentIndex())
    {
        QMessageBox::warning(this, tr("开始失败"), tr("地址数量为空或未选择币种!"));
        __setBtnStatus(1);
        return;
    }
    if (0 == ui->led_addrCount->text().toInt())
    {
        QMessageBox::warning(this, tr("开始失败"), tr("请填写地址数量至少为1!"));
        __setBtnStatus(1);
        return;
    }
    QRegExp expAddrCount(REGEXP_ADDRCOUNT);
    QString strAddrCount = ui->led_addrCount->text().trimmed();
    if (!expAddrCount.exactMatch(strAddrCount))
    {
        QMessageBox::warning(this, tr("开始失败"), tr("请填写正确的地址数量格式!"));
        __setBtnStatus(1);
        return;
    }

    const char *pszPyFileName;
    if (0 == ui->cbx_coinType->currentText().compare("btc", Qt::CaseInsensitive))
        pszPyFileName = "btc_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("eth", Qt::CaseInsensitive))
        pszPyFileName = "eth_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("usdp", Qt::CaseInsensitive))
        pszPyFileName = "usdp_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("htdf", Qt::CaseInsensitive))
        pszPyFileName = "htdf_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("ltc", Qt::CaseInsensitive))
        pszPyFileName = "ltc_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("dash", Qt::CaseInsensitive))
        pszPyFileName = "dash_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("het", Qt::CaseInsensitive))
        pszPyFileName = "het_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("xrp", Qt::CaseInsensitive))
        pszPyFileName = "xrp_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("eos", Qt::CaseInsensitive))
        pszPyFileName = "eos_addr_gen";
    if (0 == ui->cbx_coinType->currentText().compare("xlm", Qt::CaseInsensitive))  //恒星币, 使用社区提供的QTSDK生成地址, 而不是使用python脚本
        pszPyFileName = "xlm";
    if (0 == ui->cbx_coinType->currentText().compare("trx", Qt::CaseInsensitive))  //波场
        pszPyFileName = "trx_addr_gen";


    //开始生产地址
    QDateTime startCurrentDatetime = QDateTime::currentDateTime();
    QString strStartDatetime = startCurrentDatetime.toString("yyyy-MM-dd HH:mm:ss");
    uint uStartDatetime = startCurrentDatetime.toTime_t();
    ui->ted_info->append(QString("开始生成币种 %1 的一共 %2 个地址....(开始时间: %3 )\n")
                         .arg(ui->cbx_coinType->currentText()).arg(ui->led_addrCount->text().trimmed())
                         .arg(strStartDatetime));
    //定义进度条
    QProgressDialog *progressDlg = new QProgressDialog(this);
    progressDlg->setWindowModality(Qt::WindowModal);
    progressDlg->setMinimumDuration(0);
    progressDlg->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
    progressDlg->setLabelText(tr("正在生成地址..."));     //设置进度条对话框的正文
//    progressDlg->setCancelButtonText(tr("取消"));     //设置取消按钮
    progressDlg->setCancelButton(0);
    uint iMaximum = ui->led_addrCount->text().toUInt();
    progressDlg->setRange(0, iMaximum);     //设置进度条的最大值和最小值
    progressDlg->setWindowTitle(QString("请稍候"));

    uint uAddrCount = ui->led_addrCount->text().trimmed().toUInt();
    uint uSum = 0;
    bool bIsTestnet = __m_bIsTestNet;

    //XLM(恒星币)  使用   StellarQtSDK  进行地址生成
    if(0 == ui->cbx_coinType->currentText().compare("xlm", Qt::CaseInsensitive)  )
    {
        //while (uSum < uAddrCount)
        for(uint i = 0; i < uAddrCount; i++)
        {
            //KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(QString(SEED)));
            //qDebug() << "secreteSeed: " << keypair->getSecretSeed() ;
            //qDebug() << "publicKey: " << keypair->getPublicKey(); //获取原始的pubkey
            //qDebug() << "accountId: " << keypair->getAccountId(); //获取账户id,  即对pubkey进行编码

            KeyPair*  pKeyPair = KeyPair::random();
            //qDebug() << "secreteSeed: " << pKeyPair->getSecretSeed() ;
            //qDebug() << "publicKey: " << pKeyPair->getPublicKey(); //获取原始的pubkey
            //qDebug() << "accountId: " << pKeyPair->getAccountId(); //获取账户id,  即对pubkey进行编码

            addrInfo  tmpAddrInfo;
            tmpAddrInfo.strAddr = pKeyPair->getAccountId();
            tmpAddrInfo.strPrivKey = pKeyPair->getSecretSeed();
            //tmpAddrInfo.strPubKey = QByteArray::toHex( pKeyPair->getPublicKey() );
            tmpAddrInfo.strPubKey =  pKeyPair->getPublicKey().toHex();
            tmpAddrInfo.strCoinType = "XLM";

            __m_vctAddrInfoSrc.emplace_back(tmpAddrInfo);

        }
    }
    else
    {
        Py_SetPythonHome((char *)"./python27");

        //1.初始化Python模块
        Py_Initialize();
        if (!Py_IsInitialized())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "初始化Python模块失败!";
            __setBtnStatus(1);
            progressDlg->close();
            return;
        }
        //2.导入写好的Python文件
        PyObject *pRet = NULL;
        PyObject *pModule = PyImport_ImportModule(pszPyFileName);
        if(!pModule)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("调用Python的\'%1\'生成地址函数失败!")
                           .arg(ui->cbx_coinType->currentText());
            __setBtnStatus(1);
            progressDlg->close();
            QMessageBox::warning(this, "error", "init module failed.");
            return;
        }

        //3.获取Python文件中的函数(相当于获取函数句柄)
        PyObject *pPythonFun = PyObject_GetAttrString(pModule, "GenMultiAddr");
        if (!pPythonFun)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("调用Python的\'%1\'生成地址函数失败!")
                           .arg(ui->cbx_coinType->currentText());
            __setBtnStatus(1);
            progressDlg->close();
            return;
        }

        while (uSum < uAddrCount)
        {
            //4.调用GenMultiAddr函数
            if (uAddrCount > 300)
            {
                if (uAddrCount - uSum >= 5)
                {
                    uint uCountTmp = 5;
                    pRet = PyObject_CallFunction(pPythonFun, (char *)"ii", uCountTmp, bIsTestnet);
                    uSum += uCountTmp;
                }
                else
                {
                    pRet = PyObject_CallFunction(pPythonFun, (char *)"ii", uAddrCount - uSum, bIsTestnet);
                    uSum += uAddrCount - uSum;
                }
            }
            else
            {
                uint uCountTmp = 1;
                pRet = PyObject_CallFunction(pPythonFun, (char *)"ii", uCountTmp, bIsTestnet);
                uSum += uCountTmp;
            }

            if (NULL == pRet)
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "Python函数返回为空!";
                Py_Finalize();
                __setBtnStatus(1);
                progressDlg->close();
                return;
            }

            int iRet = __parsePyResult(pRet, __m_vctAddrInfoSrc);
            if (0 != iRet)
            {
                //5.结束,释放Python
                Py_Finalize();
                __setBtnStatus(1);
                progressDlg->close();
                return;
            }
            progressDlg->setValue(uSum);
            Py_XDECREF(pRet);
            QApplication::processEvents();
        }

        Py_XDECREF(pModule);
        Py_XDECREF(pPythonFun);
        pModule = NULL;
        pPythonFun = NULL;
        pRet = NULL;
        //5.结束,释放Python
        Py_Finalize();

    }

    progressDlg->close();

    QDialog *inputKeyDlg = new QDialog(this);
    inputKeyDlg->setGeometry(this->x() + 200, this->y() + 175, 400, 250);
    inputKeyDlg->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    inputKeyDlg->setStyleSheet("QDialog{background-color: rgb(197, 197, 197);}");
    QLabel *lbl_EncodeKey = new QLabel("加密KEY:", inputKeyDlg);
    lbl_EncodeKey->setGeometry(10, 80, 60, 30);
    lbl_EncodeKey->setAlignment(Qt::AlignCenter);
    QLineEdit *led_EncodeKey = new QLineEdit(inputKeyDlg);
    led_EncodeKey->setGeometry(80, 80 ,310 ,30);
    led_EncodeKey->setEchoMode(QLineEdit::Password);
    led_EncodeKey->setPlaceholderText(QString("KEY(十位数字和字母组成)"));
    led_EncodeKey->setAttribute(Qt::WA_InputMethodEnabled, false);
    led_EncodeKey->clear();
    QPushButton *btn_Ok = new QPushButton("确定", inputKeyDlg);
    btn_Ok->setGeometry(150, 140, 100, 30);
    btn_Ok->setFocusPolicy(Qt::NoFocus);
    QLabel *lbl_warning = new QLabel("注意:加密KEY必须输入,否则无法生成地址文件!!", inputKeyDlg);
    lbl_warning->setGeometry(10, 205, 380, 30);
    lbl_warning->setStyleSheet("QLabel{font: 12pt \"微软雅黑\";color: rgb(255, 0, 0);}");
    connect(btn_Ok, &QPushButton::clicked, inputKeyDlg, [=](){
        if (led_EncodeKey->text().isEmpty())
        {
            QMessageBox::warning(inputKeyDlg, tr("无法进行下一步"), tr("必须输入加密KEY,才能生成地址文件!"));
            return;
        }
        QRegExp expEncodeKey(REGEXP_ENCODEKEY);
        QString strEncodeKey = led_EncodeKey->text().trimmed();
        if (!expEncodeKey.exactMatch(strEncodeKey))
        {
            QMessageBox::warning(this, tr("开始失败"), tr("请填写10位字符组成的KEY!"));
            return;
        }

        bool use_chinese = strEncodeKey.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
        if(use_chinese)
        {
            QMessageBox::warning(this, tr("加密失败"), tr("加密密码不能输入中文!!") );
            return;
        }
        __m_strEncodeKey = led_EncodeKey->text().trimmed();
        inputKeyDlg->close();
    });
    inputKeyDlg->exec();
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "输入的key: " << __m_strEncodeKey;

    int iRet = __exportAddrToText();
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "导出地址信息失败(__exportAddrToText)";
        return;
    }
    //加密
    __encodePrivKey();
    //解密
//    __decodePrivKey();
    //导出地址文件到TXT文件
    iRet = __exportAddrToTxt();
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "导出地址信息失败(__exportAddrToTxt)";
        return;
    }
    //导出地址文件到二进制文件
    iRet = __exportAddrToBin();
    if (0 != iRet)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "导出地址信息失败(__exportAddrToBin)";
        return;
    }

    QDateTime endCurrentDatetime = QDateTime::currentDateTime();
    QString strEndDatetime = endCurrentDatetime.toString("yyyy-MM-dd HH:mm:ss");
    uint uEndDatetime = endCurrentDatetime.toTime_t();
    uint uTimeToGen = uEndDatetime - uStartDatetime;

    ui->ted_info->append(QString("......\n\n成功生成币种 \'%1\' 的地址,个数为: %2 个...(结束时间: %3)")\
                         .arg(ui->cbx_coinType->currentText()).arg(__m_vctAddrInfoSrc.size()).arg(strEndDatetime));
    ui->ted_info->append(QString("\n花费的时间为: %1 s\n\n================分割线================\n\n").arg(uTimeToGen));

    __setBtnStatus(1);
}

void GenerateAddrTool::on_checkBox_isTestNet_clicked()
{
    if (Qt::Checked == ui->checkBox_isTestNet->checkState())
    {
        __m_bIsTestNet = true;
    }
    else if (Qt::Unchecked == ui->checkBox_isTestNet->checkState())
    {
        __m_bIsTestNet = false;
    }
}
