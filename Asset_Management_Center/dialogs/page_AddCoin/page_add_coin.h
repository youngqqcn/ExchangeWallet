/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      添加币种业务代码模块
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef SYS_MNG_ADD_COIN_TYPE_H
#define SYS_MNG_ADD_COIN_TYPE_H

#include "am_main.h"

class CPageAddCoin : public CAMMain
{
    Q_OBJECT
public:
//    explicit CSysMngAddCoinType(QObject *parent = nullptr);
    explicit CPageAddCoin(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageAddCoin();

public:
    virtual int Init() override;

public:
    //添加币种对话框上的确认按钮
    int AddCoin(const vector<QString> &vctCondition, const QString &strGoogleCode);
    //删除币种对话框上的确认按钮
    int DeleteCoin(const QString &strWithdrawAddr, const QString &strCoinType, const QString &strGoogleCode);
    //添加币种界面搜索按钮
    int Search(const vector<QString> &vctCondition, vector<QStringList> &vctAddCoin);
    //添加币种界面编辑按钮
    int EditCoin(const vector<QString> &vctCondition);

    //查询币种表中所有的币种(不重复)
    int queryAllCoinType(QMap<QString, QString> &mapCoinAndBlockBrowser);

    //上传提币额度配置到 交易所管理台  2020-04-17 yqq
    int UploadAllConfigToExAmdin() noexcept(false);

signals:

public slots:
};

#endif // SYS_MNG_ADD_COIN_TYPE_H
