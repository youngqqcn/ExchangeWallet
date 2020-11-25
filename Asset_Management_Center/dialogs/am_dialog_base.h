#ifndef DIALOG_BASE_H
#define DIALOG_BASE_H

/********************************************************************************
*资产管理所有的对话的基类
*
********************************************************************************/
#include "comman.h"
#include <QDialog>

struct CAMDialogBase : public QDialog
{
public:
    explicit CAMDialogBase(QWidget *parent = 0);
    virtual ~CAMDialogBase();


protected:
     //纯虚函数,  子类继承, 必须实现Init方法
    virtual void _Init() = 0;



public:
    //事件过滤器，过滤esc键
    virtual bool eventFilter(QObject *obj, QEvent *event);

    //重写键盘esc键事件，使esc不响应关闭对话框事件
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // DIALOG_BASE_H
