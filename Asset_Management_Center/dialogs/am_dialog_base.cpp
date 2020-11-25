#include "am_dialog_base.h"
#include <QKeyEvent>

CAMDialogBase::CAMDialogBase(QWidget *parent):QDialog(parent)
{
}

CAMDialogBase::~CAMDialogBase()
{

}

void CAMDialogBase::_Init()
{
    //隐藏边框和标题
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
}

//事件过滤器，过滤esc键
bool CAMDialogBase::eventFilter(QObject *obj, QEvent *event)
{
    QDialog *pDialog = qobject_cast<QDialog *>(obj);
    if (pDialog != NULL)
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
        {
            QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(event);
            if (pKeyEvent->key() == Qt::Key_Escape)
            {
                return true;
            }
        }
        default:
            break;
        }
    }
    return QObject::eventFilter(obj, event);
}


//重写键盘esc键事件，使esc不响应关闭对话框事件
void CAMDialogBase::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}
