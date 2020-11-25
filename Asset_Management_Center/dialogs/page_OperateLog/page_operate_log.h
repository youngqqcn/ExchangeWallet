/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      操作日志界面业务代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#ifndef PAGE_OPERATE_LOG_H
#define PAGE_OPERATE_LOG_H

#include "am_main.h"
#include "user_info.h"

namespace OperateLogSpace {

//搜索条件
typedef struct _Condition
{
    QString     strFamilyName;      //姓名
    QString     strTel;             //管理员电话号码
    QString     strOperateType;     //操作类型
    QString     strAdminId;         //管理员ID
}Condition;

//插入操作日志所需参数
typedef struct _OperateLog
{
    _OperateLog()
    {
        QDateTime currentDatetime = QDateTime::currentDateTime();
        uint uCurrentTimestamp = currentDatetime.toTime_t();
        strAdminID = CUserInfo::loginUser().AdminID().trimmed();
        strAdminLoginName = CUserInfo::loginUser().LoginName().trimmed();
        strAdminFamilyName = CUserInfo::loginUser().FamilyName().trimmed();
        strAdminTel = CUserInfo::loginUser().Tel().trimmed();
        strDatetime = QString("%1").arg(uCurrentTimestamp);
    }

    QString     strDatetime;        //操作时间
    QString     strAdminID;         //管理员ID
    QString     strAdminLoginName;  //管理员登录名
    QString     strAdminFamilyName; //管理员姓名
    QString     strAdminTel;        //管理员手机号码
    QString     strOperateType;     //操作类型
}OperateLog;

}

class CPageOperateLog : public CAMMain
{
    Q_OBJECT
public:
//    explicit CPageOperateLog(QObject *parent = nullptr);
    explicit CPageOperateLog(const QString &strDBFilePath, const QString &strDBConnectName);
    virtual ~CPageOperateLog();

    virtual int Init() override;

    //操作日志界面搜索按钮业务
    int SearchOperateLog(const OperateLogSpace::Condition &condition, vector<QStringList> &vctOperteLog);

    //公共的插入操作日志的函数(在各个页面的按钮触发事件下调用)
    int InsertOperateLog(OperateLogSpace::OperateLog &insertData);

private:
    int __QueryMaxLogNo(uint &uLogNo);
};

#endif // PAGE_OPERATE_LOG_H
