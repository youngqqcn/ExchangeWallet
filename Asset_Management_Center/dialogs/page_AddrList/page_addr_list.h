/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      地址导入业务代码(主要操作数据库)
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef CPANGEADDRLIST_H
#define CPANGEADDRLIST_H

#include "am_main.h"
#include <QObject>

class CPageAddrList : public CAMMain     //继承自资产基类
{
    Q_OBJECT
public:
//    explicit CPangeAddrList(QObject *parent = nullptr);
    explicit CPageAddrList(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageAddrList();

public:
    virtual int Init() override;

public:
    //导入地址对话框上的确认按钮点击业务代码
    int LeadInAddrStart(const QString &strFilePath, uint &uAddrNum);

    //地址列表 搜索按钮业务代码
    int SearchCoinTypeToCount(const int iCoinTypeCount, const int iCoinType, const QString strCoinType, vector<QString> vctAllCoinType, vector<QStringList> &outPutValue);

    //地址列表 查看详情按钮业务代码
    int LookAddrDetail(const vector<QString> vctCoinType, vector<QStringList> &vctAddrDetail);

private:
    int __CheckAddr(const QStringList strlistAddr, QString &strAddr);
};

#endif // CPANGEADDRLIST_H
