/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      登录界面代码
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "login_dlg.h"
#include "ui_login_dlg.h"
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QRegExp>

CLoginDlg::CLoginDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CLoginDlg)
{
    ui->setupUi(this);
    //调用父类初始化
    CAMDialogBase::_Init();
    //调用自身初始化
    _Init();

    __m_getLoginUser = NULL;
    __m_getLoginUser = new CGetLoginUser(QString("loginUIObj"));
}

CLoginDlg::~CLoginDlg()
{
    delete ui;
    if (NULL != __m_pLoginOperate)
    {
        delete __m_pLoginOperate;
        __m_pLoginOperate = NULL;
    }
    if (NULL != __m_getLoginUser)
    {
        delete __m_getLoginUser;
        __m_getLoginUser = NULL;
    }
}

int CLoginDlg::GetResultValue()
{
    return __m_iResultValue;
}

void CLoginDlg::fastToLogin(const QString strUser, const QString strPwd)
{
    __m_strUser = strUser;
    __m_strPwd = strPwd;
    __OnLoginPress();
}

void CLoginDlg::_Init()
{
    //添加需要初始化的东西
    __m_pLoginOperate = NULL;
    __m_pLoginOperate = new CLoginOperate;

    ui->le_googleCode->clear();
    ui->le_userName->clear();
    ui->le_password->clear();

    //绑定信号槽
    connect(ui->btn_Login, &QPushButton::clicked, this, &CLoginDlg::__OnLoginPress);
    connect(ui->btn_Close, &QPushButton::clicked, this, &CLoginDlg::__OnClosePress);

    //---账号单行文本输入框中添加图片
    ui->le_userName->setTextMargins(0, 0, 30, 0);
    QLabel *Account_lock = new QLabel(this);
    Account_lock->setMaximumSize(35, 32);
    Account_lock->setCursor(QCursor(Qt::ArrowCursor));
    Account_lock->setPixmap(QPixmap(":/user.png"));
    Account_lock->setStyleSheet("border:none");
    QSpacerItem *Account_spacerItem = new QSpacerItem(150, 10, QSizePolicy::Expanding);
    QHBoxLayout *Account_editLayout = new QHBoxLayout();
    Account_editLayout->setContentsMargins(17, 2, 0, 0);
    Account_editLayout->addWidget(Account_lock);
    Account_editLayout->addSpacerItem(Account_spacerItem);
    ui->le_userName->setLayout(Account_editLayout);
    //设置文本输入框中文字的输入位置，防止文字被图片遮挡
    ui->le_userName->setTextMargins(55, 1, 1, 1);
    //---

    //---密码单行文本输入框中添加图片
    ui->le_password->setTextMargins(0, 0, 30, 0);
    QLabel *Password_lock = new QLabel(this);
    Password_lock->setMaximumSize(35, 32);
    Password_lock->setCursor(QCursor(Qt::ArrowCursor));
    Password_lock->setPixmap(QPixmap(":/password.png"));
    Password_lock->setStyleSheet("border:none");
    QSpacerItem *Password_spacerItem = new QSpacerItem(150, 10, QSizePolicy::Expanding);
    QHBoxLayout *Password_editLayout = new QHBoxLayout();
    Password_editLayout->setContentsMargins(17, 2, 0, 0);
    Password_editLayout->addWidget(Password_lock);
    Password_editLayout->addSpacerItem(Password_spacerItem);
    ui->le_password->setLayout(Password_editLayout);
    //设置文本输入框中文字的输入位置，防止文字被图片遮挡
    ui->le_password->setTextMargins(55, 1, 1, 1);
    //---

    //---谷歌验证码单行文本输入框中添加图片
    ui->le_googleCode->setTextMargins(0, 0, 30, 0);
    QLabel *GoogleCode_lock = new QLabel(this);
    GoogleCode_lock->setMaximumSize(35, 32);
    GoogleCode_lock->setCursor(QCursor(Qt::ArrowCursor));
    GoogleCode_lock->setPixmap(QPixmap(":/GoogleCode.png"));
    GoogleCode_lock->setStyleSheet("border:none");
    QSpacerItem *GoogleCode_spacerItem = new QSpacerItem(150, 10, QSizePolicy::Expanding);
    QHBoxLayout *GoogleCode_editLayout = new QHBoxLayout();
    GoogleCode_editLayout->setContentsMargins(17, 2, 0, 0);
    GoogleCode_editLayout->addWidget(GoogleCode_lock);
    GoogleCode_editLayout->addSpacerItem(GoogleCode_spacerItem);
    ui->le_googleCode->setLayout(GoogleCode_editLayout);
    //设置文本输入框中文字的输入位置，防止文字被图片遮挡
    ui->le_googleCode->setTextMargins(55, 1, 1, 1);
    //---

    ui->le_googleCode->setTextMargins(55, 1, 1, 1);
}

void CLoginDlg::__OnLoginPress()
{
    if ((!__m_strUser.isEmpty()) && (!__m_strPwd.isEmpty()))        //命令行登录
    {
        if (__m_strUser.isEmpty() || __m_strPwd.isEmpty())
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>请填写完整的登陆信息!</font>"), QString("确定"));
            __m_iResultValue = 0;
            return;
        }
        LoginOperate::AdminInfo adminInfo;
        adminInfo.UserName = __m_strUser;

        //对界面输入的密码进行哈希
//        QString strPwdHash = QCryptographicHash::hash(ui->le_password->text().toUtf8(), QCryptographicHash::Sha256).toHex();

        //将密码转为base64编码,然后进行比较
        QByteArray pwdBaseEncode = __m_strPwd.toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
        QString strPwdBaseEncode = pwdBaseEncode;

        int iRet = -1;
        iRet = __m_pLoginOperate->QueryUserName(adminInfo);
        if (INVALID_USERNAME == iRet)
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>用户名或密码错误</font>"), QString("确定"));
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("用户名或密码错误");
            __m_iResultValue = 0;
            return;
        }
        if (DB_ERR == iRet)
        {
            QMessageBox::critical(this, QString("登录失败"), QString("<font color = 'white'>数据库出现问题,请检查</font>"), QString("确定"));
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询管理员信息时,数据库出现错误");
            __m_iResultValue = 0;
            return;
        }
        if ("0" == adminInfo.Status)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("登录失败,账号已被冻结");
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>该账户已被冻结,无法登录</font>"), QString("确定"));
            __m_iResultValue = 0;
            return;
        }
        if (0 != adminInfo.Passwd.compare(strPwdBaseEncode, Qt::CaseSensitive))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("用户名或密码错误");
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>用户名或密码错误</font>"), QString("确定"));
            __m_iResultValue = 0;
            return;
        }
        iRet = __m_getLoginUser->checkLoginStatus(ui->le_userName->text());
        if (INVALID_USERNAME == iRet)
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>该用户已是登录中!!</font>"), QString("确定"));
            return;
        }
        else if (NO_ERROR != iRet)
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>数据库出错,请检查</font>"), QString("确定"));
            return;
        }

        CUserInfo::loginUser().SetLoginName(adminInfo.UserName);
        CUserInfo::loginUser().SetFamilyName(adminInfo.FamilyName);
        CUserInfo::loginUser().SetPassword(adminInfo.Passwd);
        CUserInfo::loginUser().SetAdminID(adminInfo.AdminID);
        CUserInfo::loginUser().SetAdminTypeID(adminInfo.AdminTypeID);
        CUserInfo::loginUser().SetTel(adminInfo.Tel);
        CUserInfo::loginUser().SetStatus(adminInfo.Status);

        __m_iResultValue = 1;
        //登录成功设置数据库中该用户的登录状态为1
        iRet = __m_pLoginOperate->updateLoginStatus(adminInfo.AdminID);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("登录成功更新数据库中的登录状态失败!!!");
            return;
        }
//        this->close();
    }
    else        //正常登录
    {
        if (ui->le_userName->text().isEmpty() || ui->le_password->text().isEmpty() || ui->le_googleCode->text().isEmpty())
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>请填写完整的信息!</font>"), QString("确定"));
            return;
        }
        LoginOperate::AdminInfo adminInfo;
        adminInfo.UserName = ui->le_userName->text();

        //对界面输入的密码进行哈希
    //    QString strPwdHash = QCryptographicHash::hash(ui->le_password->text().toUtf8(), QCryptographicHash::Sha256).toHex();

        //将密码转为base64编码,然后进行比较
        QByteArray pwdBaseEncode = ui->le_password->text().toUtf8().toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
        QString strPwdBaseEncode = pwdBaseEncode;

        int iRet = -1;
        iRet = __m_pLoginOperate->QueryUserName(adminInfo);
        if (INVALID_USERNAME == iRet)
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>用户名或密码错误</font>"), QString("确定"));
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("用户名或密码错误");
            return;
        }
        if (DB_ERR == iRet)
        {
            QMessageBox::critical(this, QString("登录失败"), QString("<font color = 'white'>数据库出现问题,请检查</font>"), QString("确定"));
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("查询管理员信息时,数据库出现错误");
            return;
        }
        if ("0" == adminInfo.Status)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("登录失败,账号已被冻结");
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>该账户已被冻结,无法登录</font>"), QString("确定"));
            return;
        }
        if (0 != adminInfo.Passwd.compare(strPwdBaseEncode, Qt::CaseSensitive))
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("用户名或密码错误");
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>用户名或密码错误</font>"), QString("确定"));
            ui->le_password->clear();
            ui->le_password->setFocus();    //清除错误密码之后,设置鼠标的光标回到这个输入框
            return;
        }
#if 0
        iRet = __m_getLoginUser->checkLoginStatus(ui->le_userName->text());
        if (INVALID_USERNAME == iRet)
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>该用户已登录!!</font>"), QString("确定"));
            return;
        }
        else if (NO_ERROR != iRet)
        {
            QMessageBox::warning(this, QString("登录失败"), QString("<font color = 'white'>数据库出错,请检查</font>"), QString("确定"));
            return;
        }
#endif

        CUserInfo::loginUser().SetLoginName(adminInfo.UserName);
        CUserInfo::loginUser().SetFamilyName(adminInfo.FamilyName);
        CUserInfo::loginUser().SetPassword(adminInfo.Passwd);
        CUserInfo::loginUser().SetAdminID(adminInfo.AdminID);
        CUserInfo::loginUser().SetAdminTypeID(adminInfo.AdminTypeID);
        CUserInfo::loginUser().SetTel(adminInfo.Tel);
        CUserInfo::loginUser().SetStatus(adminInfo.Status);

        __m_iResultValue = 1;
#if 0
        //登录成功设置数据库中该用户的登录状态为1
        iRet = __m_pLoginOperate->updateLoginStatus(adminInfo.AdminID);
        if (NO_ERROR != iRet)
        {
            qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << QString("登录成功更新数据库中的登录状态失败!!!");
            return;
        }
#endif
        this->close();
    }
}

void CLoginDlg::__OnClosePress()
{
    int iRet = QMessageBox::question(this, tr("退出登录"), tr("<font color = 'white'>是否要退出登录?</font>"), tr("确定"), tr("取消"));
    if (0 == iRet)
    {
        __m_iResultValue = 0;
        this->close();
    }
    else
    {
        ui->le_password->clear();
//        ui->le_googleCode->clear();
        return;
    }
}
