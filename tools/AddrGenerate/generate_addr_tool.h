
#pragma execution_character_set("utf-8")

#ifndef GENERATE_ADDR_TOOL_H
#define GENERATE_ADDR_TOOL_H

#include <QObject>
#include <QMainWindow>
#include <vector>
#include <Python.h>
using namespace std;

#define FIRST_ENCODE 'x'
#define SECOND_ENCODE 'C'
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)

namespace Ui {
class GenerateAddrTool;
}

typedef struct _ToPyParam
{
    bool        bIsTestNet;     //判断是否是测试网
    QString     strCoinType;    //币种
    QString     strAddrCount;   //地址数量
}toPyParam;

typedef struct _AddrInfo
{
    _AddrInfo()
    {
        strPubKey = "";
    }
    QString     strCoinType;    //币种
    QString     strAddr;        //地址
    QString     strPrivKey;     //私钥
    QString     strPubKey;      //公钥
}addrInfo;

class GenerateAddrTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit GenerateAddrTool(QWidget *parent = 0);
    ~GenerateAddrTool();

private:
    void __setBtnStatus(const int iFlag);
    int __parsePyResult(PyObject *pPyRet, vector<addrInfo> &vctAddrInfo);

    int __encodePrivKey();
    int __decodePrivKey();

    QString __xorEncode(QString strSrc, const QChar cKey);

    int __exportAddrToTxt();        //导出地址文件到TXT文件(不含私钥)
    int __exportAddrToText();       //导出地址文件到txt文件(含私钥)
    int __exportAddrToBin();        //导出地址到二进制文件(包括私钥)

private slots:
    void on_btn_startGenAddr_clicked();         //生成地址按钮
    void on_checkBox_isTestNet_clicked();       //是否是测试网

private:
    Ui::GenerateAddrTool *ui;

    bool        __m_bIsTestNet;
    vector<addrInfo>    __m_vctAddrInfoSrc;
    vector<addrInfo>    __m_vctAddrInfoEncode;
    QString     __m_strEncodeKey;
};

#endif // GENERATE_ADDR_TOOL_H
