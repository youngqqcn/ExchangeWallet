/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      地址导入数据库操作
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_addr_list.h"
#include "user_info.h"
#include <QProgressDialog>
#include <QSqlQuery>

//CPangeAddrList::CPangeAddrList(QObject *parent) : QObject(parent)
//{

//}

CPageAddrList::CPageAddrList(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageAddrList::~CPageAddrList()
{

}

int CPageAddrList::Init()
{
    return CAMMain::Init();
}

int CPageAddrList::LeadInAddrStart(const QString &strFilePath, uint &uAddrNum)
{
    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ARGS_ERR;
    }

    int i = 1;
    uint uAddrNumber = 0;   //需要导入的地址数量
    QString strAddr;    //地址

    QStringList strTmpKeyList;     //存储数据库表字段
    strTmpKeyList<<LIST_STR_TB_ADDR;
    QStringList strTmpValueList;   //存储选中文件中的值

    QVector<QStringList > vctTmpValue;
    vctTmpValue.clear();

    QSqlQuery   QueryResult;      //存储查到的最大序号
    QSqlQuery   qureyResult(_m_db);        //存储查到的insert的结果
    QString     strSql1 = QString("select max(add_no) from tb_addr;");
    QString     strSql2;

    int         nTmpAddNo;          //表格中序号
    QString     strTmpCoinType;       //表格中币种名
    QString     strTmpAddr;           //表格中地址
    QString     strTmpOperatePersonal = CUserInfo::loginUser().LoginName();  //表格中操作人
    QString     strTmpLeadInDatetime;   //表格中导入时间

    QDateTime   currentDatetime = QDateTime::currentDateTime();
    strTmpLeadInDatetime = currentDatetime.toString("yyyy-MM-dd hh:mm:ss");           //获取当前时间

    //读取选中文件内容,存储到vector容器中
    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        strTmpValueList = str.split("\t");
        int iRet = __CheckAddr(strTmpValueList, strAddr);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询地址失败";
        }
        if (strAddr.isEmpty())
        {
            uAddrNumber++;
            vctTmpValue.push_back(strTmpValueList);
        }
    }
    file.close();

    //定义进度条
    QProgressDialog *progressDlg = new QProgressDialog(nullptr);
    progressDlg->setWindowModality(Qt::WindowModal);
    progressDlg->setMinimumDuration(0);
    //    progressDlg->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    progressDlg->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
    progressDlg->setLabelText(tr("正在导入地址..."));     //设置进度条对话框的正文
    progressDlg->setCancelButtonText(tr("取消"));     //设置取消按钮
    int iMaximum = vctTmpValue.size();
    progressDlg->setRange(0, iMaximum);     //设置进度条的最大值和最小值
    progressDlg->setWindowTitle(QString("请稍候"));

    if (qureyResult.exec("SET AUTOCOMMIT=0;"))
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("设置不自动提交事务成功") ;
    }
    else
    {
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("设置不自动提交事务失败, 请检查数据库") << qureyResult.lastError();
        progressDlg->close();
        return DB_ERR;
    }
    qureyResult.clear();
//    if(_m_db.transaction())
    if (!qureyResult.exec("START TRANSACTION;"))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "开启事务失败" << qureyResult.lastError();
        progressDlg->close();
        return DB_ERR;
    }

    try
    {
        QTime timeTest;
        timeTest.start();
        for(auto it : vctTmpValue)
        {
            strSql2.clear();
            strTmpValueList.clear();
            //先查询数据库中序号的最大值
            QueryResult = _m_db.exec(strSql1);
            while (QueryResult.next())
            {
                nTmpAddNo = QueryResult.value(0).toInt() + 1;
            }
            if(2 == it.size())
            {
                strTmpCoinType = it[0];
                strTmpAddr = it[1];
            }
            else
            {
                progressDlg->close();
                return DB_ERR;
            }

            QueryResult.clear();

            strSql2 = QString("insert into tb_addr(add_no,lead_in_datetime,new_charge_time,coin_type,operate_personal,addr) "
                             "values(%1,\'%2\',\'--\',\'%3\',\'%4\',\'%5\');").arg(nTmpAddNo).arg(strTmpLeadInDatetime).arg(strTmpCoinType.trimmed())
                    .arg(strTmpOperatePersonal).arg(strTmpAddr.trimmed());

//            qDebug() << "*****************" << strSql2;
            if (qureyResult.exec(strSql2))        //插入数据库
            {
                qDebug() << QString("插入成功%1行").arg(i);
            }
            else
            {
                qDebug() << QString("第%1行插入失败").arg(i);
//                _m_db.rollback();
                qureyResult.clear();
                qureyResult.exec("ROLLBACK");
                progressDlg->close();
                return DB_ERR;
            }
            progressDlg->setValue(++i);
        }

        //提交事务
//        if(_m_db.commit())
        qureyResult.clear();
        if (qureyResult.exec("COMMIT;"))
        {
            qDebug() << tr("事务提交成功") ;
        }
        else
        {
            qDebug() << tr("事务提交失败, 请检查数据库") ;
//            _m_db.rollback();
            qureyResult.clear();
            qureyResult.exec("ROLLBACK");
            //做清理工作....
            progressDlg->close();
            return DB_ERR;
        }
        qDebug() << tr("一共耗时:") << timeTest.elapsed()  ;
        //return;
    }
    catch(std::exception &e)
    {
        qDebug() << e.what() ;
//        _m_db.rollback();
        qureyResult.clear();
        qureyResult.exec("ROLLBACK");
        progressDlg->close();
        //做清理工作....
    }

    progressDlg->close();

    uAddrNum = uAddrNumber;
    return NO_ERROR;
}

int CPageAddrList::SearchCoinTypeToCount(const int iCoinTypeCount, const int iCoinType, const QString strCoinType
                                          , const vector<QString> vctAllCoinType, vector<QStringList> &outPutValue)
{
    QSqlQuery       queryResult;
    queryResult.clear();

//    QString     strTabName = QString("tb_addr");        //数据库表名
//    QString     strField = QString("coin_type,count(addr)");       //需要查询的字段名
//    QString     strExpression;                         //查询条件

    QString     strSql = QString("select coin_type,count(addr) from tb_addr where coin_type is not null ");

    if (1 == iCoinType)
    {
        strSql += QString("and coin_type = \'%1\' ").arg(strCoinType);
    }
    else if (2 == iCoinType)
    {
        strSql += QString("and coin_type = \'%1\' ").arg(strCoinType);
    }
    else if (3 == iCoinType)
    {
        strSql += QString("and coin_type = \'%1\' ").arg(strCoinType);
    }
    else if (4 == iCoinType)
    {
        strSql += QString("and coin_type = \'%1\' ").arg(strCoinType);
    }
    else if (5 == iCoinType)
    {
        strSql += QString("and coin_type = \'%1\' ").arg(strCoinType);
    }

    if (0 == iCoinType)
    {
        for (int i = 0; i < iCoinTypeCount; i++)
        {
            QString strSqlTmp = strSql + QString("and coin_type = \'%1\';").arg(vctAllCoinType[i]);
            qDebug() << "************" << strSqlTmp;
            queryResult = _m_db.exec(strSqlTmp);
            while (queryResult.next())
            {
                if (queryResult.value(0).toString().isEmpty())
                {
                    continue;
                }
                QStringList strlist;
                QString strCoinTypeTmp = queryResult.value(0).toString();
                QString strCointTypeCount = queryResult.value(1).toString();
                strlist << strCoinTypeTmp << strCointTypeCount;
                outPutValue.push_back(strlist);
            }
        }
    }
    else
    {
        queryResult = _m_db.exec(strSql);
        while (queryResult.next())
        {
            if (queryResult.value(0).toString().isEmpty())
            {
                continue;
            }
            QStringList strlist;
            QString strCoinTypeTmp = queryResult.value(0).toString();
            QString strCointTypeCount = queryResult.value(1).toString();
            strlist << strCoinTypeTmp << strCointTypeCount;
            outPutValue.push_back(strlist);
        }
    }

    return NO_ERROR;
}

int CPageAddrList::LookAddrDetail(const vector<QString> vctCoinType, vector<QStringList> &vctAddrDetail)
{
    if (2 != vctCoinType.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CPangeAddrList::LookAddrDetail START";
    QSqlQuery       queryResult;
    QString strSql = QString("select * from tb_addr where coin_type is not null ");
    QSqlQuery       queryNewChargeTime(_m_db);

    if (vctCoinType[0].isEmpty())
    {
        if (tr("所有币种") != vctCoinType[1])
        {
            strSql += QString("and coin_type = \'%1\'").arg(vctCoinType[1]);
        }
    }
    else
    {
        if (!vctCoinType[0].isEmpty())
        {
            strSql += QString("and addr = \'%1\' ").arg(vctCoinType[0]);

        }
        if (tr("所有币种") != vctCoinType[1])
        {
            strSql += QString("and coin_type = \'%1\' ").arg(vctCoinType[1]);
        }
    }

    strSql += QString("order by lead_in_datetime desc,addr desc; ");

    queryResult = _m_db.exec(strSql);
    QStringList strlist;
    while (queryResult.next())
    {
        strlist.clear();
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "按照币种查询地址信息成功";

        QString strSql1 = QString("select recharge_time from tb_recharge where receive_addr = \'%1\' order by recharge_time desc;")
                .arg(queryResult.value(5).toString().trimmed());
        if (queryNewChargeTime.exec(strSql1))
        {
            if (queryNewChargeTime.next())
            {
                if (!queryNewChargeTime.value(0).toString().isEmpty())
                {
                    QDateTime       newDatetime = QDateTime::fromTime_t(queryNewChargeTime.value(0).toUInt());
                    QString         strDatetime = newDatetime.toString("yyyy-MM-dd hh:mm:ss");
                    strlist << strDatetime;
                }
                else
                {
                    strlist << QString("--");
                }
            }
            else
            {
                strlist << QString("--");
            }
        }

        strlist << queryResult.value(1).toString().trimmed() << queryResult.value(3).toString().trimmed()
                << queryResult.value(4).toString().trimmed() << queryResult.value(5).toString().trimmed();
        vctAddrDetail.push_back(strlist);
        qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "CPangeAddrList::LookAddrDetail END";
    }


    return NO_ERROR;
}

int CPageAddrList::__CheckAddr(const QStringList strlistAddr, QString &strAddr)
{
    QSqlQuery query(_m_db);
    QString strCoinType = strlistAddr.at(0);
    QString strAddress = strlistAddr.at(1);
    QString strSql = QString("select addr from tb_addr where coin_type = \'%1\' and addr = \'%2\'; ").arg(strCoinType.trimmed()).arg(strAddress.trimmed());

    if (!query.exec(strSql))
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "查询失败:" << query.lastError();
        return DB_ERR;
    }

    if (query.next())
    {
        strAddr = query.value(0).toString();
    }
    else
    {
        strAddr = QString("");
    }
    return NO_ERROR;
}
