#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QListView>
#include <QMessageBox>
#include "rawtx_comman.h"
#include "btc/btc_off_sig.h"
#include "btc/btc_raw_tx_util.h"
#include "usdt/usdt_off_sig.h"
#include "usdt/usdt_raw_tx_util.h"
#include "eth/eth_raw_tx_utils.h"
#include "cosmos/cosmos_raw_tx_utils.h"
#include "xrp/xrp_raw_tx_utils.h"
#include "eos/eos_raw_tx_utils.h"
#include "xlm/xlm_raw_tx_utils.h"
#include "trx/trx_raw_tx_utils.h"
#include "xmr/xmr_raw_tx_utils.h"


#define MESSAGE_BOX_OK msgbox.setStandardButtons(QMessageBox::Ok); \
    msgbox.setButtonText(QMessageBox::Ok, "确定");    \
    msgbox.setWindowFlags(Qt::FramelessWindowHint); \
    msgbox.exec()

namespace Ui {
class CMainWindow;
}




class CMainWindow : public QWidget
{
    Q_OBJECT

public:
    enum ErrCode{
        NO_ERROR = 0,
    };


public:
    explicit CMainWindow(QWidget *parent = 0);
    ~CMainWindow();

public:
    //获取私钥
    int GetPrivKey(bool bIsCollection, const vector<QString>& vctSrcAddr, const QString& strAmount, vector<QString>& vctPrivKey) noexcept(false);
    //初始化tablewidget
    void init_table_widget(QTableWidget *pTableWidget, uint32_t uRow, uint32_t uColumn, QStringList &strListHeader);

private:
    //解密私钥
    void __decodePrivKey(const QString &strEncodePrivKey, QString &strDecodePrivKey);
    //异或
    QString __xorEncode(const QString &strEncodePrivKey, const QChar cKey);
    //私钥导入
    int __ImportPrivKey(const QString& strCoinType, const QString& strDirPath, map<QString, QString>& mapAddrPrivKey);
    //遍历文件夹
    int __FindFilesRecursive(const QString& strDirPath, vector<QString> &vctAddrFiles);

public:
    int  Init();


protected:
    int XMR_PreworkHandler();

private slots:

    void OnImportTxFile();

private slots:
    void on_btn_export_tx_file_clicked();
    void on_btn_sig_sel_tx_clicked();
    void on_btn_ImportPrivKey_clicked();
    void on_checkBox_SelAll_clicked();

    void on_btn_sendSMS_clicked();

protected:

private:
    BTCImportData __m_btcImportData;
    USDTImportData __m_usdtImportData;
//    CBTCOffSig  __m_btcOffSig;
//    CLTCOffSig  __m_ltcOffSig;
//    CBTCRawTxUtil __m_btcRawTxUtil;
//    CLTCRawTxUtil __m_ltcRawTxUtil;

    CUSDTOffSig  __m_usdtOffSig;
    CUSDTRawTxUtil __m_usdtRawTxUtil;


    //eth导入, 导出数据
    rawtx::CETHRawTxUtilsImpl       __m_ethRawTxUtils;              //eth工具对象
    rawtx::ETH_ExportRawTxParam     __m_ethExportRawTxParams;       //导出的数据
    rawtx::ETH_ImportRawTxParam     __m_ethImportRawTxParams;       //导入的数据


    //addby by yqq  2019-05-14
    //cosmos导入, 导出
    //USDP 和 HTDF 裸交易
    rawtx::CCosmosRawTxUtilsImpl    __m_cosmosRawTxUtils;
    rawtx::Cosmos_ExportRawTxParam  __m_cosmosExportRawTxParams;
    rawtx::Cosmos_ImportRawTxParam  __m_cosmosImportRawTxParams;


    //xrp  2019-12-16  add by yqq
    rawtx::CXrpRawTxUtilsImpl       __m_xrpRawTxUtils;
    rawtx::Xrp_ExportRawTxParam     __m_xrpExportRawTxParams;
    rawtx::Xrp_ImportRawTxParam     __m_xrpImportRawTxParams;

    //eos  2019-12-16  add by yqq
    rawtx::CEosRawTxUtilsImpl       __m_eosRawTxUtils;
    rawtx::Eos_ExportRawTxParam     __m_eosExportRawTxParams;
    rawtx::Eos_ImportRawTxParam     __m_eosImportRawTxParams;

    //xlm 2020-02-12  add by yqq
    rawtx::CXlmRawTxUtilsImpl       __m_xlmRawTxUtils;
    rawtx::Xlm_ExportRawTxParam     __m_xlmExportRawTxParams;
    rawtx::Xlm_ImportRawTxParam     __m_xlmImportRawTxParams;


     //trx  2020-03-05  add by yqq
    rawtx::CTrxRawTxUtilsImpl       __m_trxRawTxUtils;
    rawtx::Trx_ExportRawTxParam     __m_trxExportRawTxParams;
    rawtx::Trx_ImportRawTxParam     __m_trxImportRawTxParams;


     //xmr  2020-04-09  yqq
    rawtx::CXmrRawTxUtilsImpl       __m_xmrRawTxUtils;
    rawtx::XMR_ExportRawTxParam     __m_xmrExportRawTxParams;
    rawtx::XMR_ImportRawTxParam     __m_xmrImportRawTxParams;

    map<QString, QString> __m_mapAddrPrivkey;


    QString __m_strEncodePwd;

private:
    Ui::CMainWindow *ui;
};

//用来加载qss样式文件
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
