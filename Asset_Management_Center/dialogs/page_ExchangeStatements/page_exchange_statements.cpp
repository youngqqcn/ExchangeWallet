/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      平台流水界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "page_exchange_statements.h"
#include <QTableWidget>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>

//CPageExchangeStatements::CPageExchangeStatements(QObject *parent) : QObject(parent)
//{

//}

CPageExchangeStatements::CPageExchangeStatements(const QString &strDBFilePath, const QString &strDBConnectName)
    :CAMMain(strDBFilePath, strDBConnectName)
{

}

CPageExchangeStatements::~CPageExchangeStatements()
{

}

int CPageExchangeStatements::Init()
{
    return CAMMain::Init();
}

int CPageExchangeStatements::__queryDB(const QString strSql1, const QString strSql2,
                                       const QString strSql3, vector<QStringList> &vctExchangeStatements)
{
    QSqlQuery queryResult;
    QSqlQuery query(_m_db);

    queryResult = _m_db.exec(strSql1);
    while (queryResult.next())
    {
        if (queryResult.value(0).toString().trimmed().isEmpty())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "没有充币信息" << queryResult.lastError();
            return DB_ERR;
        }
        else
        {
            //这边的大账户地址可以写成宏定义,因为这是固定的
            //            if ("n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4" == queryResult.value(3).toString().trimmed())
            //            {
            //                QStringList strlist;
            //                strlist << queryResult.value(0).toString().trimmed()
            //                        << QString("平台入账")
            //                        << queryResult.value(1).toString().trimmed()
            //                        << QString("--") << QString("--");
            //                QDateTime datetime = QDateTime::fromTime_t(queryResult.value(2).toUInt());
            //                QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
            //                strlist << strDatetime;
            //                listExchangeStatements.push_back(strlist);
            //            }
            //            else
            //            {
            QStringList strlist;
            strlist.clear();
            QString strSql = QString("select round(sum(recharge_count), 8) from tb_recharge where recharge_time <= \'%1\' and coin_type = \'%2\' ")
                    .arg(queryResult.value(2).toUInt()).arg(queryResult.value(0).toString());
            strlist << queryResult.value(0).toString().trimmed() << QString("充币") << queryResult.value(1).toString().trimmed();
            if (!query.exec(strSql))
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "根据时间戳查询充值数量失败" << query.lastError();
                return DB_ERR;
            }
            if (query.next())
            {
                strlist << QString("%1").arg(query.value(0).toString());
            }
            QDateTime datetime = QDateTime::fromTime_t(queryResult.value(2).toUInt());
            QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
            strlist << QString("--") << strDatetime;
            vctExchangeStatements.push_back(strlist);
            //            }
        }
    }
    queryResult.clear();
    query.clear();
    queryResult = _m_db.exec(strSql2);
    while (queryResult.next())
    {
        if (queryResult.value(0).toString().trimmed().isEmpty())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "没有自动提币信息" << queryResult.lastError();
            return DB_ERR;
        }
        else
        {
            //            if ("n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4" == queryResult.value(3).toString().trimmed())
            //            {
            //                QStringList strlistAuto;    //自动提币
            //                strlistAuto << queryResult.value(0).toString().trimmed() << QString("平台出账")
            //                            << queryResult.value(1).toString().trimmed() << QString("--") << QString("--");
            //                QDateTime datetime = QDateTime::fromTime_t(queryResult.value(2).toLongLong() / 1000);
            //                QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
            //                strlistAuto << strDatetime;
            //                listExchangeStatements.push_back(strlistAuto);
            //            }
            //            else
            //            {
            QStringList strlist;    //自动提币
            strlist.clear();
            QString strSql = QString("select round(sum(amount), 8) from tb_auto_withdraw where complete_time <= \'%1\' and coin_type = \'%2\' ")
                    .arg(queryResult.value(2).toUInt()/*toLongLong() / 1000*/).arg(queryResult.value(0).toString());
            strlist << queryResult.value(0).toString().trimmed() << QString("提币")
                    << queryResult.value(1).toString().trimmed() << QString("--");
            if (!query.exec(strSql))
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "根据时间戳查询充值数量失败" << query.lastError();
                return DB_ERR;
            }
            if (query.next())
            {
                strlist << QString("%1").arg(query.value(0).toString());
            }

            QDateTime datetime = QDateTime::fromTime_t(queryResult.value(2).toUInt()/*toLongLong() / 1000*/);
            QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
            strlist << strDatetime;
            vctExchangeStatements.push_back(strlist);
            //            }
        }
    }
    queryResult.clear();
    query.clear();
    queryResult = _m_db.exec(strSql3);
    while (queryResult.next())
    {
        if (queryResult.value(0).toString().trimmed().isEmpty())
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "没有手动提币信息" << queryResult.lastError();
            return DB_ERR;
        }
        else
        {
            //            if ("n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4" == queryResult.value(3).toString().trimmed())
            //            {
            //                QStringList strlistManual;    //手动提币
            //                strlistManual << queryResult.value(0).toString().trimmed() << QString("平台出账")
            //                              << queryResult.value(1).toString().trimmed() << QString("--") << QString("--");
            //                QDateTime datetime = QDateTime::fromTime_t(queryResult.value(2).toLongLong() / 1000);
            //                QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
            //                strlistManual << strDatetime;
            //                listExchangeStatements.push_back(strlistManual);
            //            }
            //            else
            //            {
            QStringList strlist;    //手动提币
            strlist.clear();
            QString strSql = QString("select round(sum(coin_count), 8) from tb_manual_audit where complete_time <= \'%1\' and coin_type = \'%2\' ")
                    .arg(queryResult.value(2).toUInt()/*toLongLong() / 1000*/).arg(queryResult.value(0).toString());
            strlist << queryResult.value(0).toString().trimmed() << QString("提币")
                    << queryResult.value(1).toString().trimmed() << QString("--");
            if (!query.exec(strSql))
            {
                qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "根据时间戳查询充值数量失败" << query.lastError();
                return DB_ERR;
            }
            if (query.next())
            {
                strlist << QString("%1").arg(query.value(0).toString());
            }
            QDateTime datetime = QDateTime::fromTime_t(queryResult.value(2).toUInt()/*toLongLong() / 1000*/);
            QString strDatetime = datetime.toString("yyyy-MM-dd hh:mm:ss");
            strlist << QString("%1").arg(query.value(0).toString()) << strDatetime;
            vctExchangeStatements.push_back(strlist);
            //            }
        }
    }
    return NO_ERROR;
}

int CPageExchangeStatements::StatementSearch(const vector<QString> &vctCondition, vector<QStringList> &vctExchangeStatements)
{
    //    QSqlQuery queryResult;
    //    QSqlQuery query(_m_db);
    QString strSql1 = QString("select coin_type,recharge_count,recharge_time,receive_addr from tb_recharge where recharge_status = 1 ");//充币
    QString strSql2 = QString("select coin_type,amount,complete_time,src_addr from tb_auto_withdraw where complete_time > 0 and order_status = 1 ");//自动提币
    QString strSql3 = QString("select coin_type,coin_count,complete_time,src_addr from tb_manual_audit where complete_time > 0 and order_status = 1 ");//手动提币

    //公司指定的大账户的充币信息(直接写死的查询那几个地址)
    //    QString strSql4 = QString("select coin_type,recharge_count,recharge_time from tb_recharge where coin_type is not null ");
    //公司大账户的自动提币信息(直接写死的查询那几个地址)
    //    QString strSql2 = QString("select coin_type,acount,complete_time,src_addr from tb_auto_withdraw where src_addr = \'等于一个或多个固定的地址\' ");//自动提币
    //和公司指定的大账户的手动提币信息(直接写死的查询那几个地址)
    //    QString strSql3 = QString("select coin_type,coin_count,complete_time,src_addr from tb_manual_audit where src_addr = \'等于一个或多个固定的地址\' ");//手动提币

    if(2 != vctCondition.size())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数有问题";
        return PARAM_ERR;
    }

    if (tr("所有币种") != vctCondition[0])
    {
        strSql1 += QString("and coin_type = \'%1\' ").arg(vctCondition[0]);
        strSql2 += QString("and coin_type = \'%1\' ").arg(vctCondition[0]);
        strSql3 += QString("and coin_type = \'%1\' ").arg(vctCondition[0]);
        if (tr("充币") == vctCondition[1])
        {
            strSql1 += QString("order by recharge_time desc ");
            strSql2 = QString("");
            strSql3 = QString("");
            int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
            return iRet;
        }
        strSql2 = QString("select coin_type,amount,complete_time,src_addr from tb_auto_withdraw where complete_time > 0 and order_status = 1 and coin_type = \'%1\' ").arg(vctCondition[0]);//自动提币
        strSql3 = QString("select coin_type,coin_count,complete_time,src_addr from tb_manual_audit where complete_time > 0 and order_status = 1 and coin_type = \'%1\' ").arg(vctCondition[0]);;//手动提币
        if (tr("提币") == vctCondition[1])
        {
            strSql1 = QString("");
            strSql2 += QString("order by complete_time desc ");
            strSql3 += QString("order by complete_time desc ");
            int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
            return iRet;
        }
        strSql1 = QString("select coin_type,recharge_count,recharge_time,receive_addr from tb_recharge where recharge_status = 1 and coin_type = \'%1\' ").arg(vctCondition[0]);//充币
        if (tr("平台入账") == vctCondition[1])
        {
            strSql1 += QString("and receive_addr = \'n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4\' order by recharge_time desc ");
            strSql2 = QString("");
            strSql3 = QString("");
            int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
            return iRet;
        }
        strSql1 = QString("select coin_type,recharge_count,recharge_time,receive_addr from tb_recharge where recharge_status = 1 and coin_type = \'%1\' ").arg(vctCondition[0]);//充币
        if (tr("平台出账") == vctCondition[1])
        {
            strSql1 = QString("");
            strSql2 += QString("and receive_addr = \'n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4\' order by complete_time desc ");
            strSql3 += QString("and receive_addr = \'n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4\' order by complete_time desc ");
            int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
            return iRet;
        }

        strSql1 += QString("order by recharge_time desc ");
        strSql2 += QString("order by complete_time desc ");
        strSql3 += QString("order by complete_time desc ");
        int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
        return iRet;
    }
    if (tr("充币") == vctCondition[1])
    {
        strSql1 += QString("order by recharge_time desc ");
        strSql2 = QString("");
        strSql3 = QString("");
        int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
        return iRet;
    }
    strSql2 = QString("select coin_type,amount,complete_time,src_addr from tb_auto_withdraw where complete_time > 0 and order_status = 1 ");//自动提币
    strSql3 = QString("select coin_type,coin_count,complete_time,src_addr from tb_manual_audit where complete_time > 0 and order_status = 1 ");//手动提币
    if (tr("提币") == vctCondition[1])
    {
        strSql1 = QString("");
        strSql2 += QString("order by complete_time desc ");
        strSql3 += QString("order by complete_time desc ");
        int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
        return iRet;
    }
    strSql1 = QString("select coin_type,recharge_count,recharge_time,receive_addr from tb_recharge where recharge_status = 1 ");//充币
    if (tr("平台入账") == vctCondition[1])
    {
        strSql1 += QString("and receive_addr = \'n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4\' order by recharge_time desc ");
        strSql2 = QString("");
        strSql3 = QString("");
        int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
        return iRet;
    }
    strSql1 = QString("select coin_type,recharge_count,recharge_time,receive_addr from tb_recharge where recharge_status = 1 ");//充币
    if (tr("平台出账") == vctCondition[1])
    {
        strSql1 = QString("");
        strSql2 += QString("and receive_addr = \'n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4\' order by complete_time desc ");
        strSql3 += QString("and receive_addr = \'n3tSpZKHpChAD9aCi7PkPrCg2Vxq1e1Sg4\' order by complete_time desc ");
        int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
        return iRet;
    }
    strSql1 += QString("order by recharge_time desc ");
    strSql2 += QString("order by complete_time desc ");
    strSql3 += QString("order by complete_time desc ");
    int iRet = __queryDB(strSql1, strSql2, strSql3, vctExchangeStatements);
    return iRet;
}

int CPageExchangeStatements::ExportExcel(QTableWidget *tableWdiget)
{
    if (NULL == tableWdiget)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "参数错误";
        return PARAM_ERR;
    }

    //导出CSV文件
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("保存为..."), QString(), tr("Files (*.csv)"));
    if (fileName.isEmpty())
        return FILE_OPEN_ERR;

    //打开CSV文件
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return FILE_OPEN_ERR;

    QTextStream out(&file);
    //获取表格内容
    int iRow = tableWdiget->rowCount();
    int iCol = tableWdiget->columnCount();
    QString strHeadRow = "";

    for (int m = 0; m < iCol; m++)
    {
        if (m == iCol - 1)
            strHeadRow += tableWdiget->horizontalHeaderItem(m)->text() + "\n";
        else
            strHeadRow += tableWdiget->horizontalHeaderItem(m)->text() + ",";
    }
    out << strHeadRow;
    out.flush();

    for (int i = 0; i < iRow; i++)
    {
        for (int j = 0; j < iCol; j++)
        {
            QString string = tableWdiget->item(i, j)->text().trimmed();
            //处理带有\r\n的数据,使用"/"替换"\r\n"
            string = string.replace("\r\n", "/");
            out << "\t" << string << ",";// 写入文件
        }
        out << "\n";
    }
    file.close();


    //导出Excel文件
//    QString filepath = QFileDialog::getSaveFileName(nullptr, tr("保存为..."),
//                                                    QString(), tr("EXCEL files (*.xls);;HTML-Files (*.txt);;"));

//    if (filepath == "")
//    {
//        return FILE_OPEN_ERR;
//    }
//    else
//    {
//        int row = tableWdiget->rowCount();
//        int col = tableWdiget->columnCount();
//        QList<QString> list;
//        //添加列标题
//        QString HeaderRow;
//        for(int i = 0; i < col; i++)
//        {
//            HeaderRow.append(tableWdiget->horizontalHeaderItem(i)->text() + "\t");
//        }
//        list.push_back(HeaderRow);
//        for(int i = 0; i < row; i++)
//        {
//            QString rowStr = "";
//            for(int j = 0; j < col; j++)
//            {
//                rowStr += tableWdiget->item(i, j)->text().trimmed() + "\t";
//            }
//            //处理带有\r\n的数据,使用","替换"\r\n"
//            rowStr = rowStr.replace("\r\n", ",");
//            qDebug() << QString("第 %1 行数据: ").arg(i) << rowStr;
//            list.push_back(rowStr);
//        }
//        QTextEdit textEdit;
//        for(int i=0;i<list.size();i++)
//        {
//            textEdit.append(list.at(i));
//        }

//        QFile file(filepath);
//        if(file.open(QFile::WriteOnly | QIODevice::Text))
//        {
//            QTextStream ts(&file);
//            ts.setCodec("ANSI");
//            ts << textEdit.document()->toPlainText();
//            file.close();
//        }
//    }
    return NO_ERROR;
}

void CPageExchangeStatements::SortForDate(vector<QStringList> &vctExchangeStatements)
{
    vector<QStringList> vctTmpCopy ;
    QStringList strlistTemp;
    for(auto item : vctExchangeStatements)
    {
        vctTmpCopy.push_back(item);
    }

    //选择排序
    for (uint i = 0; i < vctTmpCopy.size(); i++)
    {
        for (uint j = i + 1; j < vctTmpCopy.size(); j++)
        {
            QDateTime datetime = QDateTime::fromString(vctTmpCopy[i].at(5), "yyyy-MM-dd hh:mm:ss");
            uint uDatetime = datetime.toTime_t();
            QDateTime datetime1 = QDateTime::fromString(vctTmpCopy[j].at(5), "yyyy-MM-dd hh:mm:ss");
            uint uDatetime1 = datetime1.toTime_t();
            if (uDatetime < uDatetime1)
            {
                strlistTemp.clear();
                strlistTemp = vctTmpCopy[i];
                vctTmpCopy[i] = vctTmpCopy[j];
                vctTmpCopy[j] = strlistTemp;
            }
        }
    }

    vctExchangeStatements.clear();
    for (auto ite : vctTmpCopy)
    {
        vctExchangeStatements.push_back(ite);
    }
}
