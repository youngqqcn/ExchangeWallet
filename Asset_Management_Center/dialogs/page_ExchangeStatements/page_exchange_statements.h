/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      平台流水界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_EXCHANGE_STATEMENTS_H
#define PAGE_EXCHANGE_STATEMENTS_H

#include "am_main.h"
#include <QObject>

class CPageExchangeStatements : public CAMMain
{
    Q_OBJECT
public:
//    explicit CPageExchangeStatements(QObject *parent = nullptr);
    explicit CPageExchangeStatements(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageExchangeStatements();

    virtual int Init() override;

private:
    //查询数据库操作(公共函数)
    int __queryDB(const QString strSql1, const QString strSql2, const QString strSql3, vector<QStringList> &vctExchangeStatements);

public:
    //平台财务流水界面,查询按钮
    int StatementSearch(const vector<QString> &vctCondition, vector<QStringList> &vctExchangeStatements);
    //平台财务流水/充值日报/提现日报 三个界面 公共的导出Excel文件函数
    int ExportExcel(QTableWidget *tableWdiget);

    void SortForDate(vector<QStringList> &vctExchangeStatements);

signals:

public slots:
};

#endif // PAGE_EXCHANGE_STATEMENTS_H
