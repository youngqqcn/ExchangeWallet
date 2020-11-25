/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      短信设置对话框界面
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "short_msg_setting_dlg.h"
#include "ui_short_msgsetting_dlg.h"
#include "page_charge_list.h"


CShortMsgSettingDlg::CShortMsgSettingDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CShortMsgSettingDlg)
{
    ui->setupUi(this);

    CAMDialogBase::_Init(); //调用父类的默认的初始化函数

    _Init(); //调用自己的初始化

    __InitLineEdit();
}

CShortMsgSettingDlg::~CShortMsgSettingDlg()
{
    delete ui;
}


void CShortMsgSettingDlg::_Init()
{
    //绑定取消按钮
    connect(ui->btn_SettingCancel, &QPushButton::clicked, this, &CShortMsgSettingDlg::OnClosePress);

    //绑定确定按钮
    connect(ui->btn_SettingOK, &QPushButton::clicked, this, &CShortMsgSettingDlg::__OnShortMsgSetting);
}

void CShortMsgSettingDlg::OnClosePress()
{
    __InitLineEdit();
    this->close();
}

void CShortMsgSettingDlg::__OnShortMsgSetting()
{
    if (ui->le_AdminTel1->text().isEmpty() && ui->le_AdminTel2->text().isEmpty() && ui->le_AdminTel3->text().isEmpty()
            && ui->le_AdminTel4->text().isEmpty() && ui->le_AdminTel5->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("设置失败"), tr("注意:请至少输入一个手机号"));
        MESSAGE_BOX_OK;
        return;
    }
    if (ui->le_GoogleCode->text().isEmpty())
    {
        QMessageBox msgbox(QMessageBox::Warning, tr("设置失败"), tr("注意:请输入谷歌验证码"));
        MESSAGE_BOX_OK;
        return;
    }

    int iRet = -1;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("进入__OnShortMsgSetting");

    vector<QString>  vctStrTel;
    vctStrTel.push_back( ui->le_AdminTel1->text().trimmed());
    vctStrTel.push_back( ui->le_AdminTel2->text().trimmed());
    vctStrTel.push_back( ui->le_AdminTel3->text().trimmed());
    vctStrTel.push_back( ui->le_AdminTel4->text().trimmed());
    vctStrTel.push_back( ui->le_AdminTel5->text().trimmed());
    QString strGoogleCode = ui->le_GoogleCode->text().trimmed();

     //请完善参数检查
#if 0
    //1.检查电话号码
    if(0 == vctStrTel.size() || strGoogleCode.isEmpty())
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ShortMsgSetting参数检查错误";
        return ;
    }

    for(uint u = 0; u < vctStrTel.size(); u++)
    {
        //如果电话号码 不符合规定长度
        if( !(UINT_TEL_LEN_MIN <= vctStrTel[u].length()  && vctStrTel[u].length() <= UINT_TEL_LEN_MAX) )
        {
            QMessageBox::information(NULL, tr("短信提醒设置"), tr("电话号码%1 : 长度非法, 请从新设置电话号码, 长度为11至13位").arg(u));
            qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("电话号码非法");
            return ;
        }
    }

    //2.检查谷歌验证码
    if(6 != strGoogleCode.length())
    {
        //请处理错误
    }
#endif


#if 0
    //3.获取业务句柄
    //map<UINT_HANDLE, CAMMain*>::iterator itHandle = gs_mapHandles.find(UINT_HANDLE::PAGE_CHARGE_LIST);
    auto itHandle = g_mapHandles.find(UINT_HANDLE::PAGE_CHARGE_LIST);
    int iSize = g_mapHandles.size();
    if(g_mapHandles.end() == itHandle)
    {
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("获取句柄失败");
        QMessageBox::warning(this, tr("获取句柄失败"), tr("获取句柄失败"));
        return;
    }

    //将基类对象指针, 转为派生类对象指针  必须用 dynamic_cast  , 不能用原始的裸强制转换!!!
    // 如果是强制转换必须使用   static_cast, 而不用原始裸强制转换!!!
    CPageChargeList *pChargeListHandle = dynamic_cast<CPageChargeList*>(itHandle->second);
    if(nullptr == pChargeListHandle) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        return;
    }
#else
    CPageChargeList *pChargeListHandle = ::GetHandle<CPageChargeList>(UINT_HANDLE::PAGE_CHARGE_LIST);
    if(nullptr == pChargeListHandle) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        __InitLineEdit();
        return;
    }
#endif


    //4.开始使用获取到的 业务处理句柄
    iRet = pChargeListHandle->ShortMsgSetting(vctStrTel, strGoogleCode);
    if(NO_ERROR != iRet)
    {
        //处理错误
        QMessageBox msgbox(QMessageBox::Critical, tr("设置失败"), tr("设置失败,请检查数据库"));
        MESSAGE_BOX_OK;
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "ShortMsgSetting设置短信错误";
        //请根据实际业务添加其他处理....
        return;
    }

    QMessageBox msgbox(QMessageBox::Information, tr("设置成功"), tr("短信设置成功"));
    MESSAGE_BOX_OK;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("成功退出__OnShortMsgSetting");
    this->close();
}

void CShortMsgSettingDlg::__InitLineEdit()
{
    ui->le_GoogleCode->clear();
    ui->le_AdminTel1->clear();
    ui->le_AdminTel2->clear();
    ui->le_AdminTel3->clear();
    ui->le_AdminTel4->clear();
    ui->le_AdminTel5->clear();
}
