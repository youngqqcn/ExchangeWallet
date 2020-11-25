/********************************************************************
*文件作者:      xuchang
*作者邮箱:      442298344@qq.com
*创建日期:      2019-05-07  10:30
*文件说明:      地址导入对话框界面按钮业务
*修改记录:

修改日期         修改人       修改说明                      版本号
2019-05-07      xuchang     explain                     v0.0.1
********************************************************************/
#include "import_addr_dlg.h"
#include "ui_import_addr_dlg.h"
#include "page_addr_list.h"
#include <QFileDialog>
#include <QListView>

CImportAddrDlg::CImportAddrDlg(QWidget *parent) :
    CAMDialogBase(parent),
    ui(new Ui::CImportAddrDlg)
{
    ui->setupUi(this);

    CAMDialogBase::_Init(); //调用父类的默认的初始化函数

    _Init(); //调用自己的初始化

    __InitUIControl();
}

CImportAddrDlg::~CImportAddrDlg()
{
    delete ui;
}

void CImportAddrDlg::__InitUIControl()
{
    ui->led_filePath->clear();
}

void CImportAddrDlg::__OnCancelPress()
{
    __InitUIControl();
    this->close();
}

void CImportAddrDlg::__OnAffirmPress()
{
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "进入__OnAffirmPress";
    CPageAddrList *pLeadInAddrHandles = ::GetHandle<CPageAddrList>(UINT_HANDLE::PAGE_ADDR_LIST);
    if(nullptr == pLeadInAddrHandles) //必须判断转换后是否为空
    {
        //动态类型转换失败
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("动态类型转换失败");
        __InitUIControl();
        return;
    }

    int iRet = -1;
    QString strAddrFilePath = ui->led_filePath->text();
    uint uAddrNum;  //导入地址的数量

    iRet = pLeadInAddrHandles->LeadInAddrStart(strAddrFilePath, uAddrNum);
    if(NO_ERROR != iRet)
    {
        //处理错误
        qCritical() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << "LeadInAddrStart 导入地址失败";
        QMessageBox msgbox(QMessageBox::Critical, tr("导入失败"), tr("失败,检查文件格式或数据库"));
        MESSAGE_BOX_OK;
        //请根据实际业务添加其他处理....
        return;
    }

    __InitUIControl();
    QMessageBox msgbox(QMessageBox::Information, tr("确定"), tr("成功导入了 %1 个地址:\n请在地址详情查看!!").arg(uAddrNum));
    MESSAGE_BOX_OK;
    qDebug() << "[file:" << __FILENAME__ << " line:" << __LINE__ << " ]" << tr("成功退出__OnAffirmPress");
}

void CImportAddrDlg::__OnChoosePress()
{
    //打开文件选中对话框,并返回选中的文件的名称
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择文件"), ".", tr("text(*.txt)"));

    if (!fileName.isEmpty())        //如果选中一个文件
    {
        //        QFileInfo fileInfo;
        //        fileInfo = QFileInfo(fileName);
        auto fileInfo = QFileInfo(fileName);
        QString fileName = fileInfo.fileName();         //获取文件名
        qDebug() << "选中的文件名为:" << fileName;
        QString filePath = fileInfo.absolutePath();     //获取文件路径
        QString path = filePath + QString("/") + fileName;
        ui->led_filePath->setText(path);
    }
}

void CImportAddrDlg::_Init()
{
    connect(ui->btn_cancel, &QPushButton::clicked, this, &CImportAddrDlg::__OnCancelPress);

    connect(ui->btn_affirm, &QPushButton::clicked, this, &CImportAddrDlg::__OnAffirmPress);

    connect(ui->btn_selectFile, &QPushButton::clicked, this, &CImportAddrDlg::__OnChoosePress);
}
