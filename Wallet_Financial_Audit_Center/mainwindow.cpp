#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sig_confirm_dlg.h"
#include "utils.h"
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <set>
#include "config.h"

#define INT_ROW_COUNT      (50)
//sign_withdraw_tablew
#define LIST_STR_SIGN_WITHDRAW_TABLE_HEADER QString("币种")<<QString("订单编号")<<QString("源地址")<<QString("目的地址")<<QString("金额")<<QString("矿工费")
//第一次
#define FIRST_ENCODE 'x'
//第二次
#define SECOND_ENCODE 'C'
//匹配key
#define REGEXP_ENCODEKEY "^\\w{10,10}$"

#define  LIST_STR_DEFAULT_COIN_TYPE QString("BTC")<<QString("ETH")<<QString("ETC")<<QString("EOS")<<QString("XMR")<<QString("XRP")<<QString("TRX")<<QString("XLM")<<QString("LTC")<<QString("BCH")<<\
                                    QString("BSV")<<QString("DASH")<<QString("USDT")<<QString("HTDF")<<QString("HET")<<QString("USDP")


CMainWindow::CMainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CMainWindow)
{
    ui->setupUi(this);
    QString strTitle = G_STR_EXE_VERSION;
    setWindowTitle(strTitle);
    qDebug() << __FUNCTION__ << strTitle;


    QStringList strListHeader;

    strListHeader.clear();
    strListHeader << LIST_STR_SIGN_WITHDRAW_TABLE_HEADER;
    init_table_widget(ui->tbw_sign_withdraw_tx, INT_ROW_COUNT, strListHeader.size() ,strListHeader);



    ui->tabWidget->setCurrentIndex(0);
    ui->cbx_coin_type->setView(new QListView);
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

//初始化
int CMainWindow::Init()
{
    QStringList strSupportCoins;
    strSupportCoins << LIST_STR_DEFAULT_COIN_TYPE;
    ui->cbx_coin_type->addItems(strSupportCoins);
    for(auto item : g_vctERC20Tokens)
    {
        ui->cbx_coin_type->addItem(item);
    }

    //增加 HRC20 代币   2020-04-16
    for(auto item : g_vct_HRC20_Tokens)
    {
        ui->cbx_coin_type->addItem(item);
    }


    connect(ui->btn_import_tx_file, &QPushButton::clicked,  this, &CMainWindow::OnImportTxFile);
    return CMainWindow::NO_ERROR;
}


void CMainWindow::init_table_widget(QTableWidget *pTableWidget, uint32_t uRow, uint32_t uColumn, QStringList &strListHeader)
{
    QTableWidget *p_tablew = pTableWidget;
    //ui->tableWidget->resize(500, 500); //调整表格大小
    if(0 == uRow) p_tablew->setRowCount(uRow);
    p_tablew->setColumnCount(uColumn);
    p_tablew->setHorizontalHeaderLabels(strListHeader);//设置表格列头
    p_tablew->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
    //p_tablew->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
    //p_tablew->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection); //只能选中一行
    //p_tablew->verticalHeader()->setVisible(false); //隐藏行头
    //p_tablew->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//自适应拉伸
    // p_tablew->horizontalHeader()->setSectionResizeMode(0/*列*/, QHeaderView::ResizeToContents); //根据内容自动调整宽度
    //p_tablew->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


#if 0
    int iCol = 0;
    p_tablew->setColumnWidth(iCol++, 170);
    p_tablew->setColumnWidth(iCol++, 200);
    p_tablew->setColumnWidth(iCol++, 330);
    p_tablew->setColumnWidth(iCol++, 330);
    p_tablew->setColumnWidth(iCol++, 150);
    p_tablew->setColumnWidth(iCol++, 250);
#else
    int iCol = 0;
    p_tablew->setSelectionBehavior(QAbstractItemView::SelectRows);
    p_tablew->setSelectionMode(QAbstractItemView::QAbstractItemView::SingleSelection);
    p_tablew->setColumnWidth(iCol++ , 170);
    p_tablew->horizontalHeader()->setSectionResizeMode(iCol++, QHeaderView::ResizeToContents);
    p_tablew->horizontalHeader()->setSectionResizeMode(iCol++, QHeaderView::ResizeToContents);
    p_tablew->horizontalHeader()->setSectionResizeMode(iCol++, QHeaderView::ResizeToContents);
    p_tablew->horizontalHeader()->setSectionResizeMode(iCol++, QHeaderView::ResizeToContents);
    p_tablew->horizontalHeader()->setSectionResizeMode(iCol++, QHeaderView::ResizeToContents);
#endif


    //用随机数据填充表格
    //SetTableWidgetFillData(p_tablew);
}

void CMainWindow::__decodePrivKey(const QString &strEncodePrivKey, QString &strDecodePrivKey)
{
    QByteArray base64PrivKey = strEncodePrivKey.toUtf8().fromBase64(strEncodePrivKey.toUtf8());
    QString strBase64PrivKeyTmp = base64PrivKey;
    QString strSecondXor = __xorEncode(strBase64PrivKeyTmp, SECOND_ENCODE);
    QString strFirstXor = __xorEncode(strSecondXor, FIRST_ENCODE);
    base64PrivKey = strFirstXor.toUtf8().fromBase64(strFirstXor.toUtf8());
    strDecodePrivKey = base64PrivKey;
}

QString CMainWindow::__xorEncode(const QString &strEncodePrivKey, const QChar cKey)
{
    QByteArray src;
    if(strEncodePrivKey.length() > 0)
    {
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        src = codec->fromUnicode(strEncodePrivKey);
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

int CMainWindow::__ImportPrivKey(const QString &strCoinType, const QString &strDirPath, map<QString, QString> &mapAddrPrivKey)
{
    vector<QString>  vctAddrFiles;
    __FindFilesRecursive(strDirPath, vctAddrFiles);
    if(vctAddrFiles.empty())
    {
        //        qCritical() << __FUNCTION__ << QString("not found .bin files");
        //        throw runtime_error("未发现地址(.bin)文件");
        qCritical() << __FUNCTION__ << QString("not found .dat files");
        throw runtime_error("未发现地址(.dat)文件");
    }

    //------
    QDialog *inputKeyDlg = new QDialog(this);
    inputKeyDlg->setGeometry(this->x() + 500, this->y() + 275, 400, 250);
    inputKeyDlg->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    QLabel *lbl_EncodeKey = new QLabel("加密KEY:", inputKeyDlg);
    lbl_EncodeKey->setGeometry(10, 80, 70, 30);
    lbl_EncodeKey->setAlignment(Qt::AlignCenter);
    QLineEdit *led_EncodeKey = new QLineEdit(inputKeyDlg);
    led_EncodeKey->setGeometry(90, 80 ,300 ,30);
    led_EncodeKey->setEchoMode(QLineEdit::Password);
    led_EncodeKey->setPlaceholderText(QString("KEY(十位数字和字母组成)"));
    led_EncodeKey->setAttribute(Qt::WA_InputMethodEnabled, false);
    QPushButton *btn_Ok = new QPushButton("确定", inputKeyDlg);
    btn_Ok->setGeometry(70, 140, 100, 30);
    btn_Ok->setFocusPolicy(Qt::NoFocus);
    QPushButton *btn_Cancel = new QPushButton("取消", inputKeyDlg);
    btn_Cancel->setGeometry(230, 140, 100, 30);
    btn_Cancel->setFocusPolicy(Qt::NoFocus);
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
            QMessageBox::warning(this, tr("开始失败"), tr("请填写10位数字或字母组成的KEY!"));
            return;
        }
        bool use_chinese = strEncodeKey.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
        if(use_chinese)
        {
            QMessageBox::warning(this, tr("加密失败"), tr("加密密码不能输入中文!!") );
            return;
        }
        __m_strEncodePwd = led_EncodeKey->text().trimmed();
        led_EncodeKey->clear();
        inputKeyDlg->close();
    });
    connect(btn_Cancel, &QPushButton::clicked, inputKeyDlg, [=](){
        led_EncodeKey->clear();
        inputKeyDlg->close();
        throw runtime_error("取消了导入私钥");
    });
    inputKeyDlg->exec();
    //------

    //------
    //判断输入的密码是否跟选中的所有文件匹配
    for (uint i = 0; i < vctAddrFiles.size(); i++)
    {
        QFile readFile(vctAddrFiles[i]);
        if(!readFile.open(QFile::ReadOnly))
        {
            qCritical() << __FUNCTION__ << "打开地址文件失败!";
            throw runtime_error("打开文件失败");
        }

        QDataStream readStream(&readFile);
        QString strEncodePrivKey = "";
        QString strDecodePrivKey = "";
        readStream >> strEncodePrivKey;
        QStringList strlistPrivKeyTmp = strEncodePrivKey.trimmed().split("\t");
        strEncodePrivKey = strlistPrivKeyTmp.at(2);
        __decodePrivKey(strEncodePrivKey, strDecodePrivKey);
        if (0 != __m_strEncodePwd.compare(strDecodePrivKey.mid(0, 10), Qt::CaseSensitive))
        {
            qCritical() << QString("文件 %1 的密码对不上输入的密码!").arg(vctAddrFiles[i]);
            throw runtime_error("有文件的密码对不上输入的解密密码");
        }
    }
    //------

    mapAddrPrivKey.clear();
    for(QString strFilePath : vctAddrFiles)
    {
        QFile file(strFilePath);
        if(!file.open(QFile::ReadOnly))
        {
            qCritical() << __FUNCTION__ << QString("Read file %1 error : %2").arg(strFilePath).arg(file.errorString());
            throw runtime_error("打开文件失败");
        }

        QDataStream readStream(&file);
        while (!readStream.atEnd())
        {
            QString strTmp = "";
            readStream >> strTmp;
            QStringList strlstSplit = strTmp.trimmed().split("\t");

            //  币种  地址   私钥  公钥(v1.1版本有, v1.2版本及以后没有公钥)
            if(strlstSplit.size() < 3)
            {
                qCritical() << __FUNCTION__ << QString("私钥文件格式不正确, 至少需要3个字段");
                file.close();
                throw std::runtime_error("私钥文件格式不正确, 至少需要3个字段");
            }


            //USDT,BSV,BCH  和 BTC 共用同一地址
            QString strPrivFileCoinType = strlstSplit[0]; //私钥文件中的币种名
            QString strCoinSeries = strCoinType; //币系

            // USDT , BSV, BCH  可以共用 BTC的私钥文件
            if( 0 == strCoinType.compare("USDT", Qt::CaseInsensitive) || 0 == strCoinType.compare("BCH", Qt::CaseInsensitive)
                || 0 == strCoinType.compare("BSV", Qt::CaseInsensitive))
            {
                strCoinSeries = "BTC";
            }

            //ERC20代币 共用 ETH的私钥文件
            //ETC 共用 ETH 私钥
            if(utils::IsERC20Token(strCoinType) || 0 == strCoinType.compare("ETC", Qt::CaseInsensitive)) strCoinSeries = "ETH";

            //HRC20 代币共用 HTDF私钥
            if(utils::Is_HRC20_Token(strCoinType))
                strCoinSeries = "HTDF";

            if(0 != strPrivFileCoinType.compare(strCoinType, Qt::CaseInsensitive)   //直接比较
                 &&  0 != strPrivFileCoinType.compare(strCoinSeries, Qt::CaseInsensitive) //币系比较
                 )
            {
                continue; //若有其他币种的地址文件,则跳过
            }

            QString strAddr = strlstSplit[1];
            QString strPrivKey = strlstSplit[2];//禁止将私钥打印到日志文件, 如有发现, 重罚!

            QString strDecodePrivKey = "";
            __decodePrivKey(strPrivKey, strDecodePrivKey);

            mapAddrPrivKey.insert(std::make_pair(strAddr.trimmed(), strDecodePrivKey.trimmed().mid(10)));
        }
        file.close();
    }
    return 0;
}

int CMainWindow::__FindFilesRecursive(const QString &strDirPath, vector<QString> &vctAddrFiles)
{
    QDir dir(strDirPath);
    if (!dir.exists())
    {
        return -1;
    }

    //取到所有的文件和文件名，但是去掉.和..的文件夹（这是QT默认有的）
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);

    //文件夹优先
    dir.setSorting(QDir::DirsFirst);


    //转化成一个list
    QFileInfoList list = dir.entryInfoList();
    if(list.size()< 1 )
    {
        return -1;
    }

    //递归算法的核心部分
    for(int i =0; i < list.size(); i++)
    {
        QFileInfo fileInfo = list.at(i);

        //如果是文件夹，递归
        if( fileInfo.isDir() )
        {
            __FindFilesRecursive(fileInfo.filePath(), vctAddrFiles);
        }
        else
        {
            //            if(fileInfo.fileName().endsWith(".bin"))
            if (fileInfo.fileName().endsWith(".dat"))
                vctAddrFiles.push_back(fileInfo.filePath());
        }
    }
    return 0;
}



//导入交易文件
void CMainWindow::OnImportTxFile()
{
    QString strErrMsg;


    //获取当前选择的币种(注意: 如果combox中用的是中文, 注意优化一下币种判断)
    QString strCurSelCoinType  = ui->cbx_coin_type->currentText();
    if(0 == ui->cbx_coin_type->currentIndex())
    {
        qDebug() << __FUNCTION__ << QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive)  || utils::IsERC20Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
           || utils::Is_HRC20_Token(strCurSelCoinType) //增加  HRC20 代币支持
            || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive)
           ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        qDebug() << __FUNCTION__ << QString("当前筛选的币种是\"%1\",暂不支持.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的签名功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    //如果是 XMR 则需要特殊处理
    if(  0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive) )
    {
        try
        {
            int iRet = XMR_PreworkHandler();
            if(QMessageBox::No == iRet || QMessageBox::Yes == iRet)
                return ;
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
    }


    //QFileDialog::getOpenFileName();
    QString strFilePath = QFileDialog::getOpenFileName(this, tr("选择交易文件"),tr("."), tr("josn文件(*.json);;所有文件(*.*)"));
    if(strFilePath.trimmed().isEmpty())
    {
        return;
    }

    QFile fileImport(strFilePath);
    if(!fileImport.exists())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("文件不存在, 请重新选择"));
        MESSAGE_BOX_OK;
        return;
    }
    ui->le_tx_file_path->clear();
    ui->le_tx_file_path->setText(strFilePath);


    if( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
        )
    {
        //清空历史信息
        __m_btcImportData = BTCImportData();
        __m_btcImportData.strCoinType = "";
        __m_btcImportData.uTimestamp = 0;
        __m_btcImportData.vctExportItems.clear();

        try
        {
            if( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive))
            {
                CBTCRawTxUtil().ImportRawTxFromJsonFile(strFilePath, __m_btcImportData);
            }
            else if(0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) )
            {
                CLTCRawTxUtil().ImportRawTxFromJsonFile(strFilePath, __m_btcImportData);
            }
            else if(0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) )
            {
                CBCHRawTxUtil().ImportRawTxFromJsonFile(strFilePath, __m_btcImportData);
            }
            else if(0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive) )
            {
                CBSVRawTxUtil().ImportRawTxFromJsonFile(strFilePath, __m_btcImportData);
            }
            else if(0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive) )
            {
                CDASHRawTxUtil().ImportRawTxFromJsonFile(strFilePath, __m_btcImportData);
            }
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }




        ui->tbw_sign_withdraw_tx->clearContents(); //只清除内容, 不清除列头
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->setRowCount( __m_btcImportData.vctExportItems.size());
        for(uint i = 0; i < __m_btcImportData.vctExportItems.size(); i++)
        {
            int iCol = 0;
            if(!(0 < __m_btcImportData.vctExportItems[i].vctDstAddr.size() && 0 < __m_btcImportData.vctExportItems[i].vctSrcAddr.size()))
            {
                QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("导入的数据错误, 输入地址或输出地址为空"));
                MESSAGE_BOX_OK;
                return;
            }


            QString strSrcAddrs = "";
            for(auto strSrcAddr : __m_btcImportData.vctExportItems[i].vctSrcAddr)
            {
                strSrcAddrs += strSrcAddr;
                strSrcAddrs += "\r\n";
            }
            pTbw->setRowHeight(i, 20*__m_btcImportData.vctExportItems[i].vctSrcAddr.size());

            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(__m_btcImportData.strCoinType);
            pTbw->setItem(i, iCol++, checkBox);

            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_btcImportData.vctExportItems[i].strOrderID ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem(strSrcAddrs/* __m_btcImportData.vctExportItems[i].vctSrcAddr[0]*/ ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_btcImportData.vctExportItems[i].vctDstAddr[0] ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_btcImportData.vctExportItems[i].strAmount ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_btcImportData.vctExportItems[i].strTxFee ));

        }

    }
    else if (0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        //清空历史信息
        __m_usdtImportData = USDTImportData();
        __m_usdtImportData.strCoinType = "";
        __m_usdtImportData.uTimestamp = 0;
        __m_usdtImportData.vctExportItems.clear();

        try
        {
            __m_usdtRawTxUtil.ImportRawTxFromJsonFile(strFilePath, __m_usdtImportData);
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }


        ui->tbw_sign_withdraw_tx->clearContents(); //只清除内容, 不清除列头
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->setRowCount( __m_usdtImportData.vctExportItems.size());
        for(uint i = 0; i < __m_usdtImportData.vctExportItems.size(); i++)
        {
            int iCol = 0;
            if(!(0 < __m_usdtImportData.vctExportItems[i].vctDstAddr.size() && 0 < __m_usdtImportData.vctExportItems[i].vctSrcAddr.size()))
            {
                QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("导入的数据错误, 输入地址或输出地址为空"));
                MESSAGE_BOX_OK;
                return;
            }


            QString strSrcAddrs = "";
            for(auto strSrcAddr : __m_usdtImportData.vctExportItems[i].vctSrcAddr)
            {
                strSrcAddrs += strSrcAddr;
                strSrcAddrs += "\r\n";
            }
            pTbw->setRowHeight(i, 20 * (1 + __m_usdtImportData.vctExportItems[i].vctSrcAddr.size()));

            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(__m_usdtImportData.strCoinType);
            pTbw->setItem(i, iCol++, checkBox);

            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_usdtImportData.vctExportItems[i].strOrderID ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem(strSrcAddrs/* __m_btcImportData.vctExportItems[i].vctSrcAddr[0]*/ ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_usdtImportData.vctExportItems[i].vctDstAddr[0] ));

            QString strAmountShow = __m_usdtImportData.vctExportItems[i].strAmount ;
            strAmountShow += "BTC";
            strAmountShow += "\r\n";
            strAmountShow += __m_usdtImportData.vctExportItems[i].strTokenAmount;
            strAmountShow += "USDT";
            pTbw->setItem(i, iCol++, new QTableWidgetItem(strAmountShow ));
            pTbw->setItem(i, iCol++, new QTableWidgetItem( __m_usdtImportData.vctExportItems[i].strTxFee + "BTC" ));

        }
    }
    else if( 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) ||  utils::IsERC20Token(strCurSelCoinType)
       || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive) )
    {
        //清空历史数据
        __m_ethImportRawTxParams = rawtx::ETH_ImportRawTxParam();

        //设置导入文件路径
        __m_ethImportRawTxParams.m_strFilePath = strFilePath;
        __m_ethImportRawTxParams.m_strCoinType = strCurSelCoinType;

        //导入数据
        try
        {
            __m_ethRawTxUtils.ImportRawTx( &__m_ethImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }

        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        const rawtx::ETH_ImportRawTxParam * const pEthImportData = &__m_ethImportRawTxParams;

        //显示导入的数据
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount(pEthImportData->size());

        for(size_t i = 0; i < pEthImportData->size(); i++)
        {
            int iCol = 0;

            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);

            QString strShowSymbol = strCurSelCoinType;
            if( utils::IsERC20Token(strCurSelCoinType) && !strCurSelCoinType.contains("ERC20", Qt::CaseInsensitive) )
                strShowSymbol = "ERC20-" + strShowSymbol;
            strShowSymbol = strShowSymbol.toUpper();

            checkBox->setText(strShowSymbol);
            pTbw->setItem(i, iCol++, checkBox);//币种,
            pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pEthImportData)[i].strOrderId) );      //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pEthImportData)[i].strAddrFrom) );     //源地址

            QString strShowTo = (utils::IsERC20Token(strCurSelCoinType)) ? ( (*pEthImportData)[i].strERC20Recipient ) : ((*pEthImportData)[i].strAddrTo);
            pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowTo ));       //目的地址, 如果是ERC20代币, 则显示 ERC20代币接收方的地址, 是否显示合约地址,待定

            QString strValue =  (utils::IsERC20Token(strCurSelCoinType)) ? (  (*pEthImportData)[i].strERC20TokenValue ) : ( (*pEthImportData)[i].strValue);
            strValue += strCurSelCoinType;
            pTbw->setItem(i, iCol++, new QTableWidgetItem( strValue ));        //金额

            QString strShowFee = (utils::IsERC20Token(strCurSelCoinType)) ? ("0.03") : ("0.01");
            pTbw->setItem(i, iCol++, new QTableWidgetItem( strShowFee ) );//手续费,
        }

    }
    else if(0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCurSelCoinType)
            )
    {
        //1.导入数据
        //清空历史数据
        __m_cosmosImportRawTxParams = rawtx::Cosmos_ImportRawTxParam();

        //导入文件文件路径
        __m_cosmosImportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();
        __m_cosmosImportRawTxParams.m_strFilePath = strFilePath;

        try
        {
            __m_cosmosRawTxUtils.ImportRawTx( &__m_cosmosImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }


        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        //2.显示数据
        //指针强制转换(只读指针)
        rawtx::Cosmos_ImportRawTxParam const* const pCosmosImportData =  &__m_cosmosImportRawTxParams;

        if(NULL == pCosmosImportData)
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 == pCosmosImportData->size())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), QString("文件已导入, 但是数据为空, 请检查导入文件 \"%1\"").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        //显示导入的数据
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount( pCosmosImportData->size() );

        for(size_t i = 0; i < pCosmosImportData->size(); i++)
        {
            int iCol = 0;


            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(pCosmosImportData->m_strCoinType.toUpper());
            pTbw->setItem(i, iCol++, checkBox);//币种,

            pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strOrderId) );       //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strFrom) );          //源地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strTo) );            //目的地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( (*pCosmosImportData)[i].strValue) );         //金额

            try
            {
                std::string cstrFeeAmount = (*pCosmosImportData)[i].strFeeAmount.toStdString();
                double dFeeAmount = boost::lexical_cast<double>(cstrFeeAmount) / 100000000;
                pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", dFeeAmount)));  //手续费, 以USDP, 或 HTDF为单位显示
            }
            catch(boost::bad_lexical_cast &e)
            {
                qDebug() << __FUNCTION__ << QString("boost::lexical_cast<double>(cstrFeeAmount)  error:") << e.what();
                QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("导入的数据格式错误!"));
                MESSAGE_BOX_OK;
                return;
            }


        }

    }
    else if(0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive))
    {
        //1.导入数据
        __m_xrpImportRawTxParams = rawtx::Xrp_ImportRawTxParam();
        __m_xrpImportRawTxParams.m_bIsCollection = false;
        __m_xrpImportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();
        __m_xrpImportRawTxParams.m_strFilePath = strFilePath;

        try
        {
            __m_xrpRawTxUtils.ImportRawTx( &__m_xrpImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        //2.显示数据
        rawtx::Xrp_ImportRawTxParam const * const pXrpImportData = &__m_xrpImportRawTxParams;
        if(NULL == pXrpImportData)
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 == pXrpImportData->size())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), QString("文件已导入, 但是数据为空, 请检查导入文件 \"%1\"").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        //显示导入的数据
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount( pXrpImportData->size() );

        for(size_t i = 0; i < pXrpImportData->size(); i++)
        {
            int iCol = 0;

            rawtx::Xrp_ImportRawTxItem  const &refImptItem = (*pXrpImportData)[i];

            QString strDstTag = QString::asprintf("%llu", refImptItem.uDestinationTag);

            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(pXrpImportData->m_strCoinType.toUpper());
            pTbw->setItem(i, iCol++, checkBox);             //币种,
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );                       //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAddr) );                       //源地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAddr + QString("_") + strDstTag)); //目的地址_标签
            pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", refImptItem.dAmountInXRP) ));         //金额
            pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", (double)(refImptItem.uFeeInDrop * 1.0 / 1000000))));

        }

    }
    else if(0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive))
    {
         //1.导入数据
        __m_eosImportRawTxParams = rawtx::Eos_ImportRawTxParam();
        __m_eosImportRawTxParams.m_bIsCollection = false;
        __m_eosImportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();
        __m_eosImportRawTxParams.m_strFilePath = strFilePath;

        try
        {
            __m_eosRawTxUtils.ImportRawTx( &__m_eosImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        //2.显示数据
        rawtx::Eos_ImportRawTxParam const * const pEosImportData = &__m_eosImportRawTxParams;
        if(NULL == pEosImportData)
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 == pEosImportData->size())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), QString("文件已导入, 但是数据为空, 请检查导入文件 \"%1\"").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        //显示导入的数据
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount( pEosImportData->size() );

        for(size_t i = 0; i < pEosImportData->size(); i++)
        {
            int iCol = 0;

            rawtx::Eos_ImportRawTxItem  const &refImptItem = (*pEosImportData)[i];

            QString strDstTag = refImptItem.strMemo;

            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(pEosImportData->m_strCoinType.toUpper());
            pTbw->setItem(i, iCol++, checkBox);             //币种,
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );                       //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAcct ) );                       //源地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAcct + QString("_") + strDstTag)); //目的地址_标签
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));         //金额
            pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" ));

        }

    }
    else if(0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive))
    {
        //1.导入数据
        __m_xlmImportRawTxParams = rawtx::Xlm_ImportRawTxParam();
        __m_xlmImportRawTxParams.m_bIsCollection = false;
        __m_xlmImportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();
        __m_xlmImportRawTxParams.m_strFilePath = strFilePath;

        try
        {
            __m_xlmRawTxUtils.ImportRawTx( &__m_xlmImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        //2.显示数据
        rawtx::Xlm_ImportRawTxParam const * const pXlmImportData = &__m_xlmImportRawTxParams;
        if(NULL == pXlmImportData)
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 == pXlmImportData->size())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), QString("文件已导入, 但是数据为空, 请检查导入文件 \"%1\"").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        //显示导入的数据
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount( pXlmImportData->size() );

        for(size_t i = 0; i < pXlmImportData->size(); i++)
        {
            int iCol = 0;

            rawtx::Xlm_ImportRawTxItem  const &refImptItem = (*pXlmImportData)[i];

            //QString strDstTag = QString::asprintf("%llu", refImptItem.uDestinationTag);

            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(pXlmImportData->m_strCoinType.toUpper());
            pTbw->setItem(i, iCol++, checkBox);             //币种,
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );                       //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAddr) );                       //源地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAddr + QString("_") + refImptItem.strMemo)); //目的地址_标签
            pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", refImptItem.dAmount) ));         //金额
            pTbw->setItem(i, iCol++, new QTableWidgetItem( QString::asprintf("%.8f", 0.0001)));

        }

    }
    else if(0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive))
    {
         //1.导入数据
        __m_trxImportRawTxParams = rawtx::Trx_ImportRawTxParam();
        __m_trxImportRawTxParams.m_bIsCollection = false;
        __m_trxImportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();
        __m_trxImportRawTxParams.m_strFilePath = strFilePath;

        try
        {
            __m_trxRawTxUtils.ImportRawTx( &__m_trxImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        //2.显示数据
        rawtx::Trx_ImportRawTxParam const * const pTrxImportData = &__m_trxImportRawTxParams;
        if(NULL == pTrxImportData)
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 == pTrxImportData->size())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), QString("文件已导入, 但是数据为空, 请检查导入文件 \"%1\"").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        //显示导入的数据
        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount( pTrxImportData->size() );

        for(size_t i = 0; i < pTrxImportData->size(); i++)
        {
            int iCol = 0;

            rawtx::Trx_ImportRawTxItem  const &refImptItem = (*pTrxImportData)[i];


            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(pTrxImportData->m_strCoinType.toUpper());
            pTbw->setItem(i, iCol++, checkBox);             //币种,
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );   //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAcct ) );  //源地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAcct ));    //目的地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));    //金额
            pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" ));

        }

    }
    else if(0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive))
    {
        //导入
        __m_xmrImportRawTxParams = rawtx::XMR_ImportRawTxParam();
        __m_xmrImportRawTxParams.m_bIsCollection = false;
        __m_xmrImportRawTxParams.m_strCoinType =  strCurSelCoinType.toLower().trimmed();
        __m_xmrImportRawTxParams.m_strFilePath = strFilePath;


        try
        {
            __m_xmrRawTxUtils.ImportRawTx( &__m_xmrImportRawTxParams );
        }
        catch(std::exception &e)
        {
            strErrMsg = QString("导入错误: %1").arg(e.what());
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        catch(...)
        {
            strErrMsg = QString("未知错误");
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
        qDebug() << __FUNCTION__ << QString("import signed tx file: \"%1\" successed.").arg(fileImport.fileName());


        //显示导入的数据

        rawtx::XMR_ImportRawTxParam const * const pXmrImportData = &__m_xmrImportRawTxParams;
        if(NULL == pXmrImportData)
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("确定"), QString("类型转换失败").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 == pXmrImportData->size())
        {
            QMessageBox msgbox(QMessageBox::Critical, tr("确定"), QString("文件已导入, 但是数据为空, 请检查导入文件 \"%1\"").arg(fileImport.fileName()));
            MESSAGE_BOX_OK;
            return;
        }


        QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
        pTbw->clearContents(); //只清除内容, 不清除列头
        pTbw->setRowCount( pXmrImportData->size() );

        for(size_t i = 0; i < pXmrImportData->size(); i++)
        {
            int iCol = 0;

            rawtx::XMR_ImportRawTxItem  const &refImptItem = (*pXmrImportData)[i];


            QTableWidgetItem *checkBox = new QTableWidgetItem();
            checkBox->setCheckState(Qt::Unchecked);
            checkBox->setText(pXmrImportData->m_strCoinType.toUpper());
            pTbw->setItem(i, iCol++, checkBox);             //币种,
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strOrderId) );   //订单号
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strSrcAddr ) );  //源地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strDstAddr ));    //目的地址
            pTbw->setItem(i, iCol++, new QTableWidgetItem( refImptItem.strAmount ));    //金额
            pTbw->setItem(i, iCol++, new QTableWidgetItem( "0.0001" ));

        }

    }
    else //不支持的币种
    {
        qDebug() << __FUNCTION__ << QString("not support cointype %1.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning, tr("提示"), tr("暂不支持'%1'的签名功能").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
    }

    QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("导入成功"));
    MESSAGE_BOX_OK;

    return ;
}


//导出签名交易文件
void CMainWindow::on_btn_export_tx_file_clicked()
{
    QString strErrMsg;

    //获取当前选择的币种(注意: 如果combox中用的是中文, 注意优化一下币种判断)
    QString strCurSelCoinType  = ui->cbx_coin_type->currentText();
    if(0 == ui->cbx_coin_type->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        qDebug() << __FUNCTION__ << QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType);
        return;
    }

    if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
           || utils::Is_HRC20_Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive)
           || 0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive)
           ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        qDebug() << __FUNCTION__ << QString("当前筛选的币种是\"%1\", 暂不支持.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的签名功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }


    if(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
        if(__m_btcImportData.vctExportItems.empty() )
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if(0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        if(__m_usdtImportData.vctExportItems.empty() )
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }

    }
    else if(0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
    || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
    )
    {
        if(0 == __m_ethImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if(0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCurSelCoinType)
            )
    {
        if(0 == __m_cosmosImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if( 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive))
    {
        if(0 == __m_xrpImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }

    }
    else if( 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive))
    {
        if(0 == __m_eosImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if( 0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive))
    {
        if(0 == __m_xlmImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if( 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive))
    {
        if(0 == __m_trxImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else if( 0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive))
    {
        if(0 == __m_xmrImportRawTxParams.size())
        {
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件,并签名"));
            MESSAGE_BOX_OK;
            return;
        }
    }
    else
    {
        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("暂不支持 %1 币种").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }



    //3.导出已签名交易数据
    QString strExportFilePath;
    try
    {
        if(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
         )
        {
            strExportFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_btcImportData.bIsCollection ? ("Collection") : ("Withdraw"));

            if(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive))
            {
                CBTCRawTxUtil().ExportRawTxToJsonFile(strExportFilePath, __m_btcImportData);
            }
            else if ( 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) )
            {
                CLTCRawTxUtil().ExportRawTxToJsonFile(strExportFilePath, __m_btcImportData);
            }
            else if ( 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) )
            {
                CBCHRawTxUtil().ExportRawTxToJsonFile(strExportFilePath, __m_btcImportData);
            }
            else if ( 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive) )
            {
                CBSVRawTxUtil().ExportRawTxToJsonFile(strExportFilePath, __m_btcImportData);
            }
            else if ( 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive) )
            {
                CDASHRawTxUtil().ExportRawTxToJsonFile(strExportFilePath, __m_btcImportData);
            }
        }
        else if(0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            strExportFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_usdtImportData.bIsCollection ? ("Collection"): ("Withdraw"));

            __m_usdtRawTxUtil.ExportRawTxToJsonFile(strExportFilePath, __m_usdtImportData);
        }
        else if(0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive) )
        {
            strExportFilePath = __m_ethExportRawTxParams.m_strFilePath;
            __m_ethRawTxUtils.ExportRawTx( &__m_ethExportRawTxParams );
        }
        else if(0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
                || utils::Is_HRC20_Token(strCurSelCoinType)
                )
        {
            strExportFilePath = __m_cosmosExportRawTxParams.m_strFilePath;
            __m_cosmosRawTxUtils.ExportRawTx( &__m_cosmosExportRawTxParams );
        }
        else if(0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive))
        {
            strExportFilePath = __m_xrpExportRawTxParams.m_strFilePath;
            __m_xrpRawTxUtils.ExportRawTx( &__m_xrpExportRawTxParams );
        }
        else if(0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive))
        {
            strExportFilePath = __m_eosExportRawTxParams.m_strFilePath;
            __m_eosRawTxUtils.ExportRawTx( &__m_eosExportRawTxParams );
        }
        else if(0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive))
        {
            strExportFilePath = __m_xlmExportRawTxParams.m_strFilePath;
            __m_xlmRawTxUtils.ExportRawTx( &__m_xlmExportRawTxParams );
        }
        else if(0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive))
        {
            strExportFilePath = __m_trxExportRawTxParams.m_strFilePath;
            __m_trxRawTxUtils.ExportRawTx( &__m_trxExportRawTxParams );
        }
        else if(0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive))
        {
            strExportFilePath = __m_xmrExportRawTxParams.m_strFilePath;
            __m_xmrRawTxUtils.ExportRawTx( &__m_xmrExportRawTxParams );
        }
        else
        {
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("暂不支持%1币种").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("%1 导出成功").arg(strExportFilePath));
        MESSAGE_BOX_OK;

        QDesktopServices::openUrl(QUrl::fromLocalFile( QCoreApplication::applicationDirPath() ));
    }
    catch(std::exception &e)
    {
        strErrMsg = QString("导出错误: %1").arg(e.what());
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
    catch(...)
    {
        strErrMsg = QString("导出错误: 未知错误");
        QMessageBox msgbox(QMessageBox::Warning, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
}

//签名
void CMainWindow::on_btn_sig_sel_tx_clicked()
{

    QString strErrMsg;

    //获取当前选择的币种(注意: 如果combox中用的是中文, 注意优化一下币种判断)
    QString strCurSelCoinType  = ui->cbx_coin_type->currentText();
    if(0 == ui->cbx_coin_type->currentIndex())
    {
        qDebug() << __FUNCTION__ << QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 请在币种筛选框选择一个具体币种.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    if( !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
            ||  0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCurSelCoinType) //HRC20 支持
            || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive)

            ))
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        qCritical() << __FUNCTION__ << QString("当前筛选的币种是\"%1\", 暂不支持此币种签名.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的签名功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }



    //QMessageBox::question(this, QString("确认签名?"), QString("确认签名?"), QMessageBox::Ok, QMessageBox::Cancel);
    QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
    if(0 == pTbw->rowCount() )
    {
        qCritical() << __FUNCTION__ << QString("未发现交易, 请先导入交易文件.");
        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("未发现交易, 请先导入交易文件"));
        MESSAGE_BOX_OK;
        return;
    }


    vector<int> vctRows;
    for(int iRow = 0; iRow < pTbw->rowCount(); iRow++)
    {
        QTableWidgetItem  *pTbwItem =  pTbw->item(iRow,  0 /*获取复选框*/);
        if(nullptr == pTbwItem)
        {
            qDebug() << __FUNCTION__ << "获取失败";
            QMessageBox msgbox(QMessageBox::Warning, tr("错误"), tr("内部错误"));
            MESSAGE_BOX_OK;
            return;
        }

        if(Qt::Checked == pTbwItem->checkState())
        {
            QTableWidgetItem *pTmp = pTbw->item(iRow, 0 /*币种*/); //获取币种
            if(NULL == pTmp)
            {
                QMessageBox msgbox(QMessageBox::Warning, tr("提示"), tr("内部错误-获取币种失败."));
                MESSAGE_BOX_OK;
                return;
            }

            if(! pTmp->text().contains(strCurSelCoinType, Qt::CaseInsensitive))
            {
                QMessageBox msgbox(QMessageBox::Warning, tr("提示"), tr("选择的数据, 与当前操作币种不一致, 重新选择币种."));
                MESSAGE_BOX_OK;
                return;
            }

            vctRows.push_back(iRow);
        }
    }

    if(vctRows.size() > 1  &&
            !( 0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
              ||  0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) ||  utils::IsERC20Token(strCurSelCoinType)
              || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
              || utils::Is_HRC20_Token(strCurSelCoinType) //HRC20
              || 0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive)
              || 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive)
              /*|| 0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive)*/  // XMR 暂时只考虑支持  1对1  不支持  1对多
              )) //TODO:暂不支持多个
    {
        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("暂不支持批量签名"));
        MESSAGE_BOX_OK;
        return;
    }


    //获取所有选择范围
    if(vctRows.empty())
    {
        qCritical() << __FUNCTION__ << QString("请选择交易,进行签名.");
        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("请选择交易,进行签名"));
        MESSAGE_BOX_OK;
        return;
    }


    //检查是否全选了
    if(0 == strCurSelCoinType.compare("HTDF", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCurSelCoinType) //HRC20
            || 0 == strCurSelCoinType.compare("HET", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("USDP", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("ETH", Qt::CaseInsensitive) || utils::IsERC20Token(strCurSelCoinType)
            || 0 == strCurSelCoinType.compare("ETC", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("XRP", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("EOS", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("TRX", Qt::CaseInsensitive)
            )
    {
        if(vctRows.size() != ui->tbw_sign_withdraw_tx->rowCount())
        {
            strErrMsg = QString("operation error: NOTE: EOS, TRX, XRP, XLM, ETH, ETC,ERC20 USDP, HTDF, HET, HRC20-Tokens must be selected all to make signature."
                                "(注意:EOS, TRX, XRP, XLM, ETH, ETC,ERC20,USDP,HTDF,HET, HRC20代币 的批量签名必须要全部选中进行签名)");
            qCritical() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << strErrMsg;
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }
    }



    //检查界面选择的数据个数 与  实际导入的数据的个数 是否一致,  防止数组越界
    if(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
    )
    {
        if(vctRows.size() > __m_btcImportData.vctExportItems.size() )
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >= __m_btcImportData.vctExportItems.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_btcImportData.vctExportItems[iRow].vctDstAddr.empty() || __m_btcImportData.vctExportItems[iRow].vctSrcAddr.empty())
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }
    }
    else if(  0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive))
    {
        if(vctRows.size() > __m_usdtImportData.vctExportItems.size() )
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >= __m_usdtImportData.vctExportItems.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_usdtImportData.vctExportItems[iRow].vctDstAddr.empty() || __m_usdtImportData.vctExportItems[iRow].vctSrcAddr.empty())
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }
    }
    else if(0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) ||  utils::IsERC20Token(strCurSelCoinType)
       || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive) )
    {
        if(vctRows.size() > __m_ethImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 != __m_ethImportRawTxParams.m_strCoinType.compare(strCurSelCoinType, Qt::CaseInsensitive))
        {
            strErrMsg = QString("界面选择的币种(%1)与交易文件中的币种名(%2)不相符!")
                            .arg(__m_ethImportRawTxParams.m_strCoinType).arg(strCurSelCoinType);
            qCritical() << __FUNCTION__ << strErrMsg;
            QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_ethImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_ethImportRawTxParams[iRow].strAddrFrom.isEmpty() || __m_ethImportRawTxParams[iRow].strAddrTo.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }


            //TODO:其他检查规则
        }
    }
    else if(0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
            || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive)
            || utils::Is_HRC20_Token(strCurSelCoinType)
            )
    {
        if(vctRows.size() > __m_cosmosImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(vctRows.size() != __m_cosmosImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        if(0 != __m_cosmosImportRawTxParams.m_strCoinType.compare(strCurSelCoinType, Qt::CaseInsensitive))
        {
            strErrMsg = QString("界面选择的币种(%1)与交易文件中的币种名(%2)不相符!")
                            .arg(__m_cosmosImportRawTxParams.m_strCoinType).arg(strCurSelCoinType);
            qCritical() << __FUNCTION__ << strErrMsg;
            QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
            MESSAGE_BOX_OK;
            return;
        }


        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_cosmosImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_cosmosImportRawTxParams[iRow].strFrom.isEmpty() || __m_cosmosImportRawTxParams[iRow].strTo.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            //TODO:其他检查规则
        }
    }
    else if( 0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive) )
    {
        if(vctRows.size() > __m_xrpImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(vctRows.size() != __m_xrpImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_xrpImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_xrpImportRawTxParams[iRow].strSrcAddr.isEmpty() || __m_xrpImportRawTxParams[iRow].strDstAddr.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }
    }
    else if( 0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive) )
    {
        if(vctRows.size() > __m_eosImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(vctRows.size() != __m_eosImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_eosImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_eosImportRawTxParams[iRow].strSrcAcct.isEmpty() || __m_eosImportRawTxParams[iRow].strDstAcct.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }
    }
    else if( 0 == strCurSelCoinType.compare("XLM", Qt::CaseInsensitive))
    {

        if(vctRows.size() > __m_xlmImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(vctRows.size() != __m_xlmImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_xlmImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_xlmImportRawTxParams[iRow].strSrcAddr.isEmpty() || __m_xlmImportRawTxParams[iRow].strDstAddr.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }
    }
    else if( 0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive) )
    {
        if(vctRows.size() > __m_trxImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(vctRows.size() != __m_trxImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_trxImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_trxImportRawTxParams[iRow].strSrcAcct.isEmpty() || __m_trxImportRawTxParams[iRow].strDstAcct.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }
    }
    else if( 0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive) )
    {
        if(vctRows.size() > __m_xmrImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1. 所选择的行超出范围.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行超出范围"));
            MESSAGE_BOX_OK;
            return;
        }

        if(vctRows.size() != __m_xmrImportRawTxParams.size())
        {
            qCritical() << __FUNCTION__ << QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType);
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), QString("%1批量签名, 必须全部选中.").arg(strCurSelCoinType));
            MESSAGE_BOX_OK;
            return;
        }

        for(size_t iRow : vctRows) //防止后面操作的数组越界
        {
            if(iRow >=  __m_xmrImportRawTxParams.size())
            {
                qCritical() << __FUNCTION__ << QString("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("所选择的行号与导入的数据不匹配,超出数据范围, 请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }

            if(__m_xmrImportRawTxParams[iRow].strSrcAddr.isEmpty() || __m_xmrImportRawTxParams[iRow].strDstAddr.isEmpty() )
            {
                qCritical() << __FUNCTION__ << QString("输入输出的地址为空,  请重新导入数据并选择.");
                QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("输入输出的地址为空,  请重新导入数据并选择"));
                MESSAGE_BOX_OK;
                return;
            }
        }

    }
    else
    {
        //币种暂不支持, 或币种不明(当前选择的是`所有币种`)
        qCritical() << __FUNCTION__ << QString("当前筛选的币种是\"%1\", 暂不支持此币种的签名功能.").arg(strCurSelCoinType);
        QMessageBox msgbox(QMessageBox::Warning,  tr("确定"), QString("当前筛选的币种是\"%1\", 暂不支持此币种的签名功能.").arg(strCurSelCoinType));
        MESSAGE_BOX_OK;
        return;
    }

    try
    {
        //开始获取私钥  进行签名
        if(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive)
        || 0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive)
        )
        {

            //bool bGetPrivKeyNeeded = true;
            vector<QString> vctPrivKeys;
            for(auto iRow : vctRows)
            {
                BTCImportItem  &item = __m_btcImportData.vctExportItems[iRow];
                bool bIsCollections = __m_btcImportData.bIsCollection;

                GetPrivKey(bIsCollections, item.vctSrcAddr, item.strAmount, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QString strRetSignedTx;
                if(0 == strCurSelCoinType.compare("btc", Qt::CaseInsensitive))
                {
                    CBTCOffSig().TxOfflineSignature(bIsCollections, strRetSignedTx, item.vctSrcAddr, item.mapTxOut, item.strTxHex, vctPrivKeys, item.vctUTXOs);
                }
                else if(0 == strCurSelCoinType.compare("ltc", Qt::CaseInsensitive))
                {
                    CLTCOffSig().TxOfflineSignature(bIsCollections, strRetSignedTx, item.vctSrcAddr, item.mapTxOut, item.strTxHex, vctPrivKeys, item.vctUTXOs);
                }
                else if(0 == strCurSelCoinType.compare("bch", Qt::CaseInsensitive))
                {
                    CBCHOffSig().TxOfflineSignature(bIsCollections, strRetSignedTx, item.vctSrcAddr, item.mapTxOut, item.strTxHex, vctPrivKeys, item.vctUTXOs);
                }
                else if(0 == strCurSelCoinType.compare("bsv", Qt::CaseInsensitive))
                {
                    CBSVOffSig().TxOfflineSignature(bIsCollections, strRetSignedTx, item.vctSrcAddr, item.mapTxOut, item.strTxHex, vctPrivKeys, item.vctUTXOs);
                }
                else if(0 == strCurSelCoinType.compare("dash", Qt::CaseInsensitive))
                {
                    CDASHOffSig().TxOfflineSignature(bIsCollections, strRetSignedTx, item.vctSrcAddr, item.mapTxOut, item.strTxHex, vctPrivKeys, item.vctUTXOs);
                }

                item.strTxHex  = strRetSignedTx; //用签名后的交易, 替换未签名的交易
                item.bComplete = true; //签名完成
            }
        }
        else if (0 == strCurSelCoinType.compare("usdt", Qt::CaseInsensitive))
        {
            vector<QString> vctPrivKeys;
            for(auto iRow : vctRows)
            {
                QString strRet;
                USDTImportItem &item = __m_usdtImportData.vctExportItems[iRow];
                bool bIsCollections = __m_usdtImportData.bIsCollection;


                QString strShowAmount = item.strAmount + "BTC," + item.strTokenAmount + "USDT";
                GetPrivKey(bIsCollections, item.vctSrcAddr, strShowAmount, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                __m_usdtOffSig.TxOfflineSignature(strRet, item.strAmount.toDouble(), item.vctSrcAddr,  item.mapTxOut, item.strTxHex,vctPrivKeys, item.vctUTXOs);

                item.strTxHex  = strRet; //用签名后的交易, 替换未签名的交易
                item.bComplete = true; //签名完成
            }
        }
        else if(0 == strCurSelCoinType.compare("eth", Qt::CaseInsensitive) ||  utils::IsERC20Token(strCurSelCoinType)
           || 0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive) )
        {
            //动态创建一个对话框, 获取用户的输入的私钥

            __m_ethExportRawTxParams = __m_ethImportRawTxParams;

            QString strTmpPath = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
            strTmpPath += (0 == strCurSelCoinType.compare("etc", Qt::CaseInsensitive)) ? ("_ETC_") : ("_ETH_");
            if( utils::IsERC20Token(strCurSelCoinType ))
            {
                QString strTmpStr = "";
                if(strCurSelCoinType.contains("ERC20", Qt::CaseInsensitive)) strTmpStr = strCurSelCoinType;
                else strTmpStr = "ERC20-" + strCurSelCoinType;

                strTmpPath += strTmpStr;
            }


            strTmpPath += __m_ethExportRawTxParams.m_bIsCollection ? ("Collection") : ("Withdraw");
            strTmpPath += "_export_signed.json";
            __m_ethExportRawTxParams.m_strFilePath = strTmpPath;
            __m_ethExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            //bool bGetPrivKeyNeeded = true;
            vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;

            for(size_t iRow : vctRows)
            {
                rawtx::ETH_ExportRawTxItem  &item = __m_ethImportRawTxParams[iRow];
                bool bIsCollections = __m_ethExportRawTxParams.m_bIsCollection;

                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(item.strAddrFrom); //源地址, 需要获取源地址的私钥

                GetPrivKey(bIsCollections, vctSrcAddrs, item.strValue, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }


                //ETH 只有一对一
                const size_t i = iRow;
                const rawtx::ETH_ImportRawTxParam &impParams =  __m_ethImportRawTxParams;
                const rawtx::ETH_ImportRawTxItem &impItem = impParams[i];
                rawtx::ETH_CreateRawTxParam  crtParam;
                crtParam.uChainId           = impItem.uChainId;
                crtParam.uNonce             = impItem.strNonce.toULongLong();
                crtParam.strAddrFrom        = impItem.strAddrFrom.toStdString();
                crtParam.ubigintGasPrice    = ubigint(impItem.strGasPrice.toStdString().c_str());
                crtParam.ubigintGasStart    = ubigint(impItem.strGasStart.toStdString().c_str());
                crtParam.strAddrTo          = impItem.strAddrTo.toStdString();//如果是ERC20, 则是合约的地址
                crtParam.float100Value      = float100( impItem.strValue.toStdString().c_str() );
                crtParam.strPrivKey = vctPrivKeys[0].toStdString();//私钥

                //构造inputdata字段
                if(utils::IsERC20Token(impItem.strSymbol))
                {
                    crtParam.float100Value  = 0;//金额, 如果是ERC20代币, 此值必须为 0

                    std::string cstrRetData = "";
                    std::string cstrValue = "";
                    int nDecimals = utils::GetERC20Decimals( impItem.strSymbol  );
                    double dTokenErc20Value = boost::lexical_cast<double>( impItem.strERC20TokenValue.toStdString() );
                    rawtx::CETHRawTxUtilsImpl::ToWeiStr(dTokenErc20Value, nDecimals, cstrValue);
                    QString strValue = QString::fromStdString( cstrValue );

                    //impItem.strERC20Recipient 是 ERC20代币的接收方
                    rawtx::CETHRawTxUtilsImpl::MakeStdERC20TransferData(impItem.strERC20Recipient, strValue, cstrRetData);

                    memcpy(crtParam.szData, cstrRetData.c_str(), cstrRetData.size());
                    crtParam.uDataLen = cstrRetData.size();
                }

                //创建交易并签名
                __m_ethRawTxUtils.CreateRawTx(&crtParam);


                __m_ethExportRawTxParams[iRow].bComplete = true;
                __m_ethExportRawTxParams[iRow].strSignedRawTxHex = QString(rawtx::Bin2HexString(crtParam.szRetBuf, crtParam.uRetDataLen).c_str());
            }
        }
        else if(0 == strCurSelCoinType.compare("usdp", Qt::CaseInsensitive) || 0 == strCurSelCoinType.compare("htdf", Qt::CaseInsensitive)
                || 0 == strCurSelCoinType.compare("het", Qt::CaseInsensitive))
        {
            //TODO: usdp , htdf 签名
            __m_cosmosExportRawTxParams = __m_cosmosImportRawTxParams;
            __m_cosmosExportRawTxParams.m_strFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_cosmosExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_cosmosExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;
            for(size_t iRow : vctRows)
            {
                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(__m_cosmosExportRawTxParams[iRow].strFrom); //源地址, 需要获取源地址的私钥

                rawtx::Cosmos_ImportRawTxItem &item =  __m_cosmosExportRawTxParams[iRow];

                GetPrivKey(__m_cosmosExportRawTxParams.m_bIsCollection, vctSrcAddrs, item.strValue, vctPrivKeys);


                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());

                }

                QString strPrivKey = vctPrivKeys[0];

                if(!utils::IsValidPrivKey(strCurSelCoinType.trimmed(), strPrivKey, g_bBlockChainMainnet))
                {
                    strErrMsg = QString("地址:%1 的私钥无效,请检查!").arg(item.strFrom);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }


                //const 引用
                const size_t i = iRow;
                const rawtx::Cosmos_ImportRawTxParam &impParams = __m_cosmosImportRawTxParams;

                cosmos::CsRawTx  csRawTx;
                strcpy_s(csRawTx.szChainId, impParams[i].strChainId.toStdString().c_str());
                strcpy_s(csRawTx.szFeeDenom, STR_SATOSHI);
                strcpy_s(csRawTx.szMemo, impParams[i].strMemo.toStdString().c_str()); //把memo设置订单号
                strcpy_s(csRawTx.szMsgDenom, STR_SATOSHI);
                strcpy_s(csRawTx.szMsgFrom, impParams[i].strFrom.toStdString().c_str());
                strcpy_s(csRawTx.szMsgTo, impParams[i].strTo.toStdString().c_str());

                try
                {
                    //以下涉及到金额的, 统一使用  satoshi 作为单位,  即需要将浮点数转为  uint64_t
                    csRawTx.uGas       = boost::lexical_cast<uint64_t>(impParams[i].strFeeGas.toStdString() );//默认: 200000
                    csRawTx.uAccountNumber = boost::lexical_cast<uint64_t>( impParams[i].strAccountNumber.toStdString() );
                    csRawTx.uSequence  = boost::lexical_cast<uint64_t>(impParams[i].strSequence.toStdString() );
                    csRawTx.uFeeAmount = boost::lexical_cast<uint64_t>(impParams[i].strFeeAmount.toStdString() );//默认:20
                    csRawTx.uMsgAmount = (uint64_t)(100000000 * boost::lexical_cast<double>(impParams[i].strValue.toStdString() ) );//注意整型溢出
                }
                catch(boost::bad_lexical_cast &e)
                {
                    strErrMsg = QString("无法解析导入的json数据非法, %1").arg(QString(e.what()));
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                rawtx::Cosmos_CreateRawTxParam param;
                param.csRawTx = csRawTx;
                param.strPrivKey = strPrivKey.trimmed();

                //2.检查参数
                QString strErrMsg;
                if(false == param.ParamsCheck( &strErrMsg ))
                {
                    strErrMsg = QString("参数错误:%1").arg(strErrMsg);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                //3.调用接口创建
                __m_cosmosRawTxUtils.CreateRawTx( &param );

                qDebug() << __FUNCTION__ << "__m_cosmosRawTxUtils.CreateRawTx() successed.";
                __m_cosmosExportRawTxParams[i].bComplete = true;
                __m_cosmosExportRawTxParams[i].strSignedRawTxHex = param.strHexData;
            }
        }
        else if( utils::Is_HRC20_Token(strCurSelCoinType)  )
        {
            //TODO: usdp , htdf 签名
            __m_cosmosExportRawTxParams = __m_cosmosImportRawTxParams;
            __m_cosmosExportRawTxParams.m_strFilePath = QString("%1_HRC20_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_cosmosExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_cosmosExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;
            for(size_t iRow : vctRows)
            {
                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(__m_cosmosExportRawTxParams[iRow].strFrom); //源地址, 需要获取源地址的私钥

                rawtx::Cosmos_ImportRawTxItem &item =  __m_cosmosExportRawTxParams[iRow];

                GetPrivKey(__m_cosmosExportRawTxParams.m_bIsCollection, vctSrcAddrs, item.strValue, vctPrivKeys);


                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());

                }

                QString strPrivKey = vctPrivKeys[0];

                if(!utils::IsValidPrivKey(strCurSelCoinType.trimmed(), strPrivKey, g_bBlockChainMainnet))
                {
                    strErrMsg = QString("地址:%1 的私钥无效,请检查!").arg(item.strFrom);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }


                //const 引用
                const size_t i = iRow;
                const rawtx::Cosmos_ImportRawTxParam &impParams = __m_cosmosImportRawTxParams;

                cosmos::CsRawTx  csRawTx;
                strcpy_s(csRawTx.szChainId, impParams[i].strChainId.toStdString().c_str());
                strcpy_s(csRawTx.szFeeDenom, STR_SATOSHI);
                strcpy_s(csRawTx.szMemo, impParams[i].strMemo.toStdString().c_str()); //把memo设置订单号
                strcpy_s(csRawTx.szMsgDenom, STR_SATOSHI);
                strcpy_s(csRawTx.szMsgFrom, impParams[i].strFrom.toStdString().c_str());

                //注意: 此处填写 HRC20合约的地址
                //strcpy_s(csRawTx.szMsgTo, impParams[i].strTo.toStdString().c_str());
                QString  strContractAddr = utils::Get_HRC20_ContractAddr(strCurSelCoinType);
                strcpy_s(csRawTx.szMsgTo, strContractAddr.toStdString().c_str());

                try
                {
                    //以下涉及到金额的, 统一使用  satoshi 作为单位,  即需要将浮点数转为  uint64_t
                    csRawTx.uGas       = boost::lexical_cast<uint64_t>(impParams[i].strFeeGas.toStdString() );//默认: 200000
                    csRawTx.uAccountNumber = boost::lexical_cast<uint64_t>( impParams[i].strAccountNumber.toStdString() );
                    csRawTx.uSequence  = boost::lexical_cast<uint64_t>(impParams[i].strSequence.toStdString() );
                    csRawTx.uFeeAmount = boost::lexical_cast<uint64_t>(impParams[i].strFeeAmount.toStdString() );//默认:20

                    //注意:  这里的 value 必须为 0
                    csRawTx.uMsgAmount = 0;

                    //构造 Data 字段 (HRC20 交易)
                    std::string cstrRetHexStrData = "";
                    std::string cstrValue = "";
                    int nDecimals = utils::Get_HRC20_Decimals( strCurSelCoinType );
                    double dValue =  boost::lexical_cast<double>(impParams[i].strValue.toStdString() );
                    rawtx::CCosmosRawTxUtilsImpl::ToWeiStr( dValue , nDecimals, cstrValue );

                    QString strAmountInWei_Hex = QString::fromStdString(cstrValue);
                    rawtx::CCosmosRawTxUtilsImpl::Make_HRC20_Transfer_Data(impParams[i].strTo , strAmountInWei_Hex, cstrRetHexStrData);

                    memset(csRawTx.szData, 0, sizeof(csRawTx.szData));
                    memcpy( csRawTx.szData, cstrRetHexStrData.c_str(), cstrRetHexStrData.size());

                }
                catch(boost::bad_lexical_cast &e)
                {
                    strErrMsg = QString("无法解析导入的json数据非法, %1").arg(QString(e.what()));
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                rawtx::Cosmos_CreateRawTxParam param;
                param.csRawTx = csRawTx;
                param.strPrivKey = strPrivKey.trimmed();

                //2.检查参数
                QString strErrMsg;
                if(false == param.ParamsCheck( &strErrMsg ))
                {
                    strErrMsg = QString("参数错误:%1").arg(strErrMsg);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                //3.调用接口创建
                __m_cosmosRawTxUtils.CreateRawTx( &param );

                qDebug() << __FUNCTION__ << "__m_cosmosRawTxUtils.CreateRawTx() successed.";
                __m_cosmosExportRawTxParams[i].bComplete = true;
                __m_cosmosExportRawTxParams[i].strSignedRawTxHex = param.strHexData;
            }

        }
        else if(  0 == strCurSelCoinType.compare("xrp", Qt::CaseInsensitive) )
        {
            //1.初始化导出信息
            __m_xrpExportRawTxParams = __m_xrpImportRawTxParams;
            __m_xrpExportRawTxParams.m_strFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_xrpExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_xrpExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            //2.批量签名
            //   2.1.获取私钥
            //   2.2.构造创建交易的参数
            //   2.3.调用xrp裸交易工具类接口
            //   2.4.保存返回信息
             vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;
            for(size_t iRow : vctRows)
            {
                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(__m_xrpImportRawTxParams[iRow].strSrcAddr); //源地址, 需要获取源地址的私钥

                rawtx::Xrp_ExportRawTxItem &expItem =  __m_xrpExportRawTxParams[iRow];


                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                GetPrivKey(__m_xrpExportRawTxParams.m_bIsCollection, vctSrcAddrs, expItem.strAmountInDrop, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QString strPrivKey = vctPrivKeys[0];

                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                if(!utils::IsValidPrivKey(strCurSelCoinType.trimmed(), strPrivKey, g_bBlockChainMainnet))
                {
                    strErrMsg = QString("地址:%1 的私钥无效,请检查!").arg(__m_xrpImportRawTxParams[iRow].strSrcAddr);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                const size_t  i = iRow;
                rawtx::Xrp_ImportRawTxParam const &impParams = __m_xrpImportRawTxParams;
                rawtx::Xrp_ImportRawTxItem const &impItem = impParams[i];


                //a.组装参数
                rawtx::Xrp_CreateRawTxParam  param;
                param.bSigned           = false;
                param.strChainNet       = impItem.strChainNet;
                param.strSrcAddr        = impItem.strSrcAddr;
                param.dAmountInXRP      = impItem.dAmountInXRP; //金额 以XRP为单位
                param.strAmountInDrop   = impItem.strAmountInDrop; //金额 以drop为单位
                param.strDstAddr        = impItem.strDstAddr;
                param.uDestinationTag   = impItem.uDestinationTag;
                param.uFeeInDrop        = impItem.uFeeInDrop;
                param.uSequence         = impItem.uSequence;
                param.strPrivKey        =  strPrivKey;  //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                param.strSignedRawTx    = "";

                //b.调用裸交易工具类的接口进行交易创建
                if(false == param.ParamsCheck(&strErrMsg))
                {
                     strErrMsg = QString("参数错误:%1").arg(strErrMsg);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                 __m_xrpRawTxUtils.CreateRawTx( &param );

                 qDebug() << __FUNCTION__ << "__m_xrpRawTxUtils.CreateRawTx() successed.";
                 __m_xrpExportRawTxParams[i].bComplete = param.bSigned; //true
                 __m_xrpExportRawTxParams[i].strSignedRawTx = param.strSignedRawTx;
            }
        }
        else if(  0 == strCurSelCoinType.compare("eos", Qt::CaseInsensitive) )
        {
            //
            //1.初始化导出信息
            __m_eosExportRawTxParams = __m_eosImportRawTxParams;
            __m_eosExportRawTxParams.m_strFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_eosExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_eosExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            //2.批量签名
            //   2.1.获取私钥
            //   2.2.构造创建交易的参数
            //   2.3.调用xrp裸交易工具类接口
            //   2.4.保存返回信息
            vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;
            for(size_t iRow : vctRows)
            {
                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(__m_eosImportRawTxParams[iRow].strSrcAcct); //源地址, 需要获取源地址的私钥

                rawtx::Eos_ExportRawTxItem &expItem =  __m_eosExportRawTxParams[iRow];


                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                GetPrivKey(__m_eosExportRawTxParams.m_bIsCollection, vctSrcAddrs, expItem.strAmount, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QString strPrivKey = vctPrivKeys[0];

                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                if(!utils::IsValidPrivKey(strCurSelCoinType.trimmed(), strPrivKey, g_bBlockChainMainnet))
                {
                    strErrMsg = QString("地址:%1 的私钥无效,请检查!").arg(__m_eosImportRawTxParams[iRow].strSrcAcct);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                const size_t  i = iRow;
                rawtx::Eos_ImportRawTxParam const &impParams = __m_eosImportRawTxParams;
                rawtx::Eos_ImportRawTxItem const &impItem = impParams[i];


                QString strTxDigest = impItem.strTxDigestHex;


                //3.签名交易
                QString  strSignature = "";
                __m_eosRawTxUtils.SignTxDigestWithPrivKey(strTxDigest, strPrivKey, strSignature);

                //将 签名结果  嵌入   param.strSignedRawTx 中的 signatures:[] 数组中
                QString  strEmbedSigRawTxJson = impItem.strSignedRawTx;
                if(strEmbedSigRawTxJson.contains("\"signatures\": []"))
                {
                    strEmbedSigRawTxJson.replace("\"signatures\": []",  "\"signatures\": [\"" + strSignature + "\"]");
                }
                else if( strEmbedSigRawTxJson.contains("\"signatures\":[]"))
                {
                    strEmbedSigRawTxJson.replace("\"signatures\":[]",  "\"signatures\": [\"" + strSignature + "\"]");
                }


                 qDebug() << __FUNCTION__ << "__m_eosRawTxUtils.CreateRawTx() successed.";
                 __m_eosExportRawTxParams[i].bComplete = true; //true
                 __m_eosExportRawTxParams[i].strSignedRawTx = strEmbedSigRawTxJson;
            }

        }
        else if(  0 == strCurSelCoinType.compare("xlm", Qt::CaseInsensitive) )
        {
            //1.初始化导出信息
            __m_xlmExportRawTxParams = __m_xlmImportRawTxParams;
            __m_xlmExportRawTxParams.m_strFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_xlmExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_xlmExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            //2.批量签名
            //   2.1.获取私钥
            //   2.2.构造创建交易的参数
            //   2.3.调用xrp裸交易工具类接口
            //   2.4.保存返回信息
             vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;
            for(size_t iRow : vctRows)
            {
                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(__m_xlmExportRawTxParams[iRow].strSrcAddr); //源地址, 需要获取源地址的私钥

                rawtx::Xlm_ExportRawTxItem &expItem =  __m_xlmExportRawTxParams[iRow];


                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                GetPrivKey(__m_xlmExportRawTxParams.m_bIsCollection, vctSrcAddrs, expItem.strAmount, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QString strPrivKey = vctPrivKeys[0];

                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                if(!utils::IsValidPrivKey(strCurSelCoinType.trimmed(), strPrivKey, g_bBlockChainMainnet))
                {
                    strErrMsg = QString("地址:%1 的私钥无效,请检查!").arg(__m_xlmImportRawTxParams[iRow].strSrcAddr);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                const size_t  i = iRow;
                rawtx::Xlm_ImportRawTxParam const &impParams = __m_xlmImportRawTxParams;
                rawtx::Xlm_ImportRawTxItem const &impItem = impParams[i];


                //a.组装参数
                rawtx::Xlm_CreateRawTxParam  param;
                param.bSigned           = false;
                param.strChainNet       = impItem.strChainNet;
                param.strSrcAddr        = impItem.strSrcAddr;
                param.dAmount           = impItem.dAmount;
                param.strAmount         = impItem.strAmount;
                param.strDstAddr        = impItem.strDstAddr;
                param.strMemo           = impItem.strMemo;
                param.uBaseFee          = impItem.uBaseFee;
                param.uSequence         = impItem.uSequence;
                param.strPrivKey        =  strPrivKey;  //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                param.strSignedRawTx    = "";
                param.bDstAccountExists = impItem.bDstAccountExists;

                //b.调用裸交易工具类的接口进行交易创建
                if(false == param.ParamsCheck(&strErrMsg))
                {
                     strErrMsg = QString("参数错误:%1").arg(strErrMsg);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                 __m_xlmRawTxUtils.CreateRawTx( &param );

                 qDebug() << __FUNCTION__ << "__m_xlmRawTxUtils.CreateRawTx() successed.";
                 __m_xlmExportRawTxParams[i].bComplete = param.bSigned; //true
                 __m_xlmExportRawTxParams[i].strSignedRawTx = param.strSignedRawTx;
            }
        }
        else if(  0 == strCurSelCoinType.compare("trx", Qt::CaseInsensitive) )
        {
            //
            //1.初始化导出信息
            __m_trxExportRawTxParams = __m_trxImportRawTxParams;
            __m_trxExportRawTxParams.m_strFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_trxExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_trxExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();

            //2.批量签名
            //   2.1.获取私钥
            //   2.2.构造创建交易的参数
            //   2.3.调用xrp裸交易工具类接口
            //   2.4.保存返回信息
            vector<QString> vctPrivKeys;
            vector<QString> vctSrcAddrs;
            for(size_t iRow : vctRows)
            {
                vctSrcAddrs.clear();
                vctSrcAddrs.push_back(__m_trxImportRawTxParams[iRow].strSrcAcct); //源地址, 需要获取源地址的私钥

                rawtx::Trx_ExportRawTxItem &expItem =  __m_trxExportRawTxParams[iRow];


                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                GetPrivKey(__m_trxExportRawTxParams.m_bIsCollection, vctSrcAddrs, expItem.strAmount, vctPrivKeys);

                if(0 == vctPrivKeys.size())
                {
                    strErrMsg = QString("获取私钥为空");
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                QString strPrivKey = vctPrivKeys[0];

                //!!!!!!!! 警告: 禁止打印或输出任何形式的私钥 !!!!!!!!!!!!!
                if(!utils::IsValidPrivKey(strCurSelCoinType.trimmed(), strPrivKey, g_bBlockChainMainnet))
                {
                    strErrMsg = QString("地址:%1 的私钥无效,请检查!").arg(__m_trxImportRawTxParams[iRow].strSrcAcct);
                    qCritical() << __FUNCTION__ << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString());
                }

                const size_t  i = iRow;
                rawtx::Trx_ImportRawTxParam const &impParams = __m_trxImportRawTxParams;
                rawtx::Trx_ImportRawTxItem const &impItem = impParams[i];


                QString strTxDigest = impItem.strTxid;


                //3.签名交易
                QString  strSignature = "";
                __m_trxRawTxUtils.SignTxidWithPrivKey(strTxDigest, strPrivKey, strSignature);


                //将签名嵌入   param.strSignedRawTx 中的 signatures:[] 数组中对应的位置
                // 设计的占位符:   "signatures":["this_is_placeholder_for_signature"]

                const QString strPlaceholder = "this_is_placeholder_for_signature";
                QString   strEembedSigRawTrxJson = impItem.strSignedRawTx;
                if( ! strEembedSigRawTrxJson.contains( strPlaceholder ) )
                {
                    strErrMsg = QString("raw tx json does not contains \"this_is_placeholder_for_signature\"");
                    qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("params check :") << strErrMsg;
                    throw runtime_error(strErrMsg.toStdString() );
                }

                strEembedSigRawTrxJson.replace( strPlaceholder,  strSignature );


                 qDebug() << __FUNCTION__ << "__m_trxRawTxUtils.CreateRawTx() successed.";
                 __m_trxExportRawTxParams[i].bComplete = true;
                 __m_trxExportRawTxParams[i].strSignedRawTx = strEembedSigRawTrxJson;
            }

        }
        else if(  0 == strCurSelCoinType.compare("xmr", Qt::CaseInsensitive) )
        {
             //
            //1.初始化导出信息
            __m_xmrExportRawTxParams = __m_xmrImportRawTxParams;
            __m_xmrExportRawTxParams.m_strFilePath = QString("%1_%2_%3_export_signed.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))\
                    .arg(strCurSelCoinType).arg(__m_xmrExportRawTxParams.m_bIsCollection?"Collection":"Withdraw");
            __m_xmrExportRawTxParams.m_strCoinType = strCurSelCoinType.toLower().trimmed();


            for( size_t  iRow : vctRows )
            {
                //rawtx::XMR_ExportRawTxItem  &expItem = __m_xmrExportRawTxParams[iRow];

                const size_t  i = iRow;
                rawtx::XMR_ImportRawTxParam const &impParams = __m_xmrImportRawTxParams;
                rawtx::XMR_ImportRawTxItem const &impItem = impParams[i];


                //TODO: 使用 describe_transfer 确认一下交易是否正确(目的地址, 金额),  防止恶意行为


                QString  strUnsignedTxSet = impItem.strSignedRawTx;
                QString strSignedTxSet = "";
                __m_xmrRawTxUtils.SignTx(strUnsignedTxSet, strSignedTxSet );
                qInfo() << __FUNCTION__ << __LINE__ << strSignedTxSet;

                //用已签名的交易替换未签名的交易
                __m_xmrExportRawTxParams[i].bComplete = true;
                __m_xmrExportRawTxParams[i].strSignedRawTx = strSignedTxSet;

            }



        }
        else
        {
            strErrMsg = QString("操作错误: 本版本暂不支持%1").arg(strCurSelCoinType);
            qCritical() << __FUNCTION__ << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("签名成功."));
        MESSAGE_BOX_OK;
    }
    catch(std::exception &e)
    {
        strErrMsg = QString(e.what());
        qCritical() << __FUNCTION__ << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
    catch(...)
    {
        strErrMsg = QString("错误:未知错误");
        qCritical() << __FUNCTION__ << strErrMsg;
        QMessageBox msgbox(QMessageBox::Critical, tr("错误"), strErrMsg);
        MESSAGE_BOX_OK;
        return;
    }
}

//注意异常捕获
int CMainWindow::GetPrivKey(bool bIsCollection, const vector<QString>& vctSrcAddr, const QString& strAmount, vector<QString>& vctPrivKey) noexcept(false)
{
        vctPrivKey.clear();
        if(0 == vctSrcAddr.size())
        {
            throw runtime_error("传入参数vctSrcAddr为空");
        }
        if(strAmount.isEmpty())
        {
            throw runtime_error("传入参数strAmount为空");
        }

    //    if(false == bIsCollection)         //提币
    //    {
    //        QStringList lstSrcAddr;
    //        for(auto srcAddr : vctSrcAddr)
    //        {
    //            lstSrcAddr << srcAddr;
    //        }
    //        //动态创建一个对话框, 获取用户的输入的私钥
    //        QString strTempAmount = strAmount;

    //        //使用智能指针, 防止内存泄露
    //        //CSigConfirmDlg  *pSigConfirmDlg = new CSigConfirmDlg(lstSrcAddr, strTempAmount);
    //        std::shared_ptr<CSigConfirmDlg> pSigConfirmDlg ( new CSigConfirmDlg(lstSrcAddr, strTempAmount) );
    //        pSigConfirmDlg->setGeometry(this->x() + 460, this->y() + 250, 480, 300);
    //        pSigConfirmDlg->setParent(this);
    //        pSigConfirmDlg->exec();
    //        QString strPrivKey = pSigConfirmDlg->m_strPrivKey.trimmed();
    //        if(strPrivKey.isEmpty())
    //        {
    //            throw runtime_error("私钥为空");
    //        }


    //        //TODO: 私钥如何导入, 待优化
    //        //vector<QString> vctPrivKeys;
    //        QStringList lstPrivKeys = strPrivKey.split(",");
    //        for(auto strKey:lstPrivKeys)
    //        {
    //            vctPrivKey.push_back(strKey);
    //        }
    //    }
    //    else //归集
    //    {
    if(0 == __m_mapAddrPrivkey.size())
    {
        QString strErrMsg = QString("请先选择私钥文件并导入!");
        throw runtime_error(strErrMsg.toStdString());
    }

    //使用set 防止重复
    std::set<QString> setVctPrivKeys;
    for(auto srcAddr : vctSrcAddr)
    {
        if(__m_mapAddrPrivkey.end() == __m_mapAddrPrivkey.find(srcAddr))
        {
            QString strErrMsg = QString("%1的私钥未找到").arg(srcAddr);
            throw runtime_error(strErrMsg.toStdString());
        }

        setVctPrivKeys.insert(__m_mapAddrPrivkey[srcAddr]);
    }

    //拷贝到返回vector
    for(auto item : setVctPrivKeys) vctPrivKey.push_back(item);



    //    }

    //------
    qDebug() << bIsCollection << strAmount;
    //------

    if(0 == vctPrivKey.size())
    {
        throw runtime_error("未获取到任何私钥");
    }
    return NO_ERROR;
}

//导入私钥文件
void CMainWindow::on_btn_ImportPrivKey_clicked()
{
    if(0 == ui->cbx_coin_type->currentIndex())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("请先选择币种"));
        MESSAGE_BOX_OK;
        qCritical() << __FUNCTION__ << tr("文件路径为空, 请重新选择");
        return ;
    }

    QString strCoinType = ui->cbx_coin_type->currentText().trimmed();

    //XMR 特殊处理,
    //XMR不需要从加密文件中获取私钥, 而且需要将钱包文件放入 运行目录下的  daemon/monerod/manual_withdraw/cold_wallet_file
    // 其中   cold_wallet_file 是钱包文件
    // 钱包文件密码,暂时为:  123456
    if( 0 == strCoinType.compare("XMR", Qt::CaseInsensitive))
    {
        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("XMR比较特殊,不需要导入私钥,请确保目录 daemon/monerod/manual_withdraw/ 有钱包文件 cold_wallet_file"));
        MESSAGE_BOX_OK;
        return;
    }



    QString strDirPath = QFileDialog::getExistingDirectory();

    if(strDirPath.trimmed().isEmpty())
    {
        qCritical() << __FUNCTION__ << tr("文件路径为空");
        return;
    }

    QDir  tmpDir(strDirPath);
    if(!tmpDir.exists())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("文件夹不存在, 请重新选择"));
        MESSAGE_BOX_OK;
        qCritical() << __FUNCTION__ << tr("文件不存在, 请重新选择");
        return;
    }


    //根据币种分支
    if(
        !(  0 == strCoinType.compare("btc", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("ltc", Qt::CaseInsensitive) || 0 == strCoinType.compare("bch", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("bsv", Qt::CaseInsensitive)  || 0 == strCoinType.compare("dash", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("eth", Qt::CaseInsensitive) || utils::IsERC20Token(strCoinType)
             || 0 == strCoinType.compare("etc", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("htdf", Qt::CaseInsensitive) || utils::Is_HRC20_Token(strCoinType)
             || 0 == strCoinType.compare("het", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("usdp", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("usdt", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("xrp", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("eos", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("xlm", Qt::CaseInsensitive)
             || 0 == strCoinType.compare("trx", Qt::CaseInsensitive)
        )
    )
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("暂不支持币种: %1 ").arg(strCoinType));
        MESSAGE_BOX_OK;
        qCritical() << __FUNCTION__ << tr("暂不支持此币种: %1, 导入私钥失败").arg(strCoinType);
        return;
    }

    try
    {
        __ImportPrivKey(strCoinType, strDirPath, __m_mapAddrPrivkey);
    }
    catch(std::exception &e)
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("警告"), QString(e.what()));
        MESSAGE_BOX_OK;
        qCritical() << __FUNCTION__ << tr("导入私钥失败");
        return;
    }


    qDebug() << __FUNCTION__ << "一共导入 " <<  __m_mapAddrPrivkey.size() << "个地址私钥" ;
    QMessageBox msgbox(QMessageBox::Warning, tr("警告"), tr("成功导入 %1个%2地址的私钥").arg(__m_mapAddrPrivkey.size()).arg(strCoinType));
    MESSAGE_BOX_OK;
}

//全选
void CMainWindow::on_checkBox_SelAll_clicked()
{
    //如果当前是选中  则取消全选
    Qt::CheckState checkState = ( ui->checkBox_SelAll->isChecked() ) ? (Qt::Checked) : (Qt::Unchecked) ;


    QTableWidget *pTbw = ui->tbw_sign_withdraw_tx;
    for(int iRow = 0; iRow < pTbw->rowCount(); iRow++ )
    {
        QTableWidgetItem  *pTbwItem =  pTbw->item(iRow, 0);
        if(nullptr == pTbwItem)
        {
            qDebug() << "[ 文件名:" << __FILE__ << " 行号:" << __LINE__ << " ]" << "获取失败";
            QMessageBox msgbox(QMessageBox::Warning, tr("获取失败"), tr("内部错误"));
            MESSAGE_BOX_OK;
            return;
        }

        pTbwItem->setCheckState(checkState);
    }

    return;

}


#include "sms.h"

void CMainWindow::on_btn_sendSMS_clicked()
{
#if 1

    try
    {
        QString strTel = "18565659593,15727780717";
        srand(time(NULL));
        QString strContent = tr("【Hetbi】尊敬的管理员，BTC自动出币地址余额已低于警戒值(12dkyDPWXizgELYrA8C3XgrhXWhJMYAgs8)，当前余额为%1BTC，请及时充币以保证自动提币通道的畅通，谢谢您的配合！ ").arg(rand() % 100);
        int iRet = CShortMsg::SendShortMsg(strTel , strContent);
        if(0 == iRet)
        {
            QMessageBox::information(this, "提示", "发送成功!");
        }
    }
    catch(std::exception &e)
    {
        QMessageBox::warning(this, "错误", QString(e.what()));
    }
    catch(...)
    {
        QMessageBox::warning(this, "错误",  "未知错误" );
    }
#endif

}




// XMR提币比较特殊, 操作流程较多
int CMainWindow::XMR_PreworkHandler()
{
    QString strErrMsg = "";

    QMessageBox msgbox(QMessageBox::Question, tr("XMR操作类型选择"), tr("由于XMR的特殊,A1和B2操作在AM端进行,请选择操作类型:\r\n  (A2): 导入交易输出(tx_outputs)\r\n  (B1): 导出密钥镜像(key_images)\r\n  (C2): 此轮操作中已完成A1,A2和B1,B2,继续导入未签名交易文件"));
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgbox.setButtonText(QMessageBox::Yes, "A2");//导出交易输出
    msgbox.setButtonText(QMessageBox::No, "B1");//导入key-images
    msgbox.setButtonText(QMessageBox::Cancel, "C2"); //继续创建
    msgbox.setWindowFlags(Qt::FramelessWindowHint);

    int iRet = msgbox.exec();

    if(  QMessageBox::Cancel == iRet )
        return iRet;

    //导入交易输出 export txoutputs
    if( QMessageBox::Yes == iRet)
    {
        QString strImportFilePath = QFileDialog::getOpenFileName(this, tr("选择交易文件"), tr("."), tr("josn文件(*.json);;所有文件(*.*)"));
        if(strImportFilePath.trimmed().isEmpty())
        {
            strErrMsg = QString("文件路径为空").arg(strImportFilePath);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QFile  inFile(strImportFilePath);
		if(!inFile.exists())
		{
			strErrMsg = QString::asprintf("args error: import-file %s is not exists.", strImportFilePath);
			qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
			throw runtime_error( strErrMsg.toStdString() );
		}

		if(!inFile.open(QIODevice::ReadOnly))
		{
			strErrMsg = QString::asprintf("%s open failed.", strImportFilePath);
			qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
			throw runtime_error(strErrMsg.toStdString());
		}

		QByteArray byteArray = inFile.readAll();
		inFile.close();

		qInfo() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "导入数据: " << byteArray;

		QJsonParseError error;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(byteArray, &error);
		if(error.error != QJsonParseError::NoError)
		{
			strErrMsg = QString::asprintf("parse json error:%s", error.errorString());
			qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
			throw runtime_error(strErrMsg.toStdString());
		}

		QJsonObject joRoot  =  jsonDoc.object();
		if(!(!joRoot.isEmpty() && joRoot.contains("outputs_data_hex")  ))
		{
			strErrMsg = " import file DOES NOT contains `outputs_data_hex`, please check it. ";
			qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
			throw runtime_error(strErrMsg.toStdString());
		}

		QString strTxOutputs =  joRoot.value("outputs_data_hex").toString();

		if(!boost::all(strTxOutputs.toStdString(),   boost::is_xdigit()))
		{
			strErrMsg = " import file  `outputs_data_hex` is invalid hexstring, please check it. ";
			qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
			throw runtime_error(strErrMsg.toStdString());
		}

		__m_xmrRawTxUtils.ImportTxOutputs(strTxOutputs);

		//返回导出文件的路径, 用于显示

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("%1 导入成功").arg(strImportFilePath));
        MESSAGE_BOX_OK;
    }

    //导出已 key-images
    if( QMessageBox::No == iRet )
    {
        QString strSignedKeyImages = "";
        __m_xmrRawTxUtils.ExportKeyImages( strSignedKeyImages);

		QString strExportFilePath = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss_") + + "XMR_signed_key_images.json";


		QJsonParseError jErr;
		QJsonDocument  jDoc = QJsonDocument::fromJson(strSignedKeyImages.toLatin1(), &jErr);
        if(jErr.error != QJsonParseError::NoError)
		{
			strErrMsg = QString::asprintf("parse json error:%s", jErr.errorString());
			qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
			throw runtime_error(strErrMsg.toStdString());
		}

		QJsonObject  joOut;
		joOut.insert("signed_key_images", jDoc.array() );


		QFile outFile(strExportFilePath);
        if(!outFile.open(QIODevice::WriteOnly))
        {
            strErrMsg = QString::asprintf("%s open failed.", strExportFilePath);
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << strErrMsg;
            throw runtime_error(strErrMsg.toStdString());
        }

        QJsonDocument jDocOut;
        jDocOut.setObject( joOut );

		outFile.write(  jDocOut.toJson()  );
        outFile.close();

        QMessageBox msgbox(QMessageBox::Information, tr("确定"), QString("singed_key_images文件:%1 导出成功").arg(strExportFilePath));
        MESSAGE_BOX_OK;

        QDesktopServices::openUrl(QUrl::fromLocalFile( QCoreApplication::applicationDirPath() ));
    }

    return iRet;
}
