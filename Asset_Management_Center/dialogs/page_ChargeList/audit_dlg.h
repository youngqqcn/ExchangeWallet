#ifndef AUDIT_DLG_H
#define AUDIT_DLG_H

#include "comman.h"
#include "am_dialog_base.h"
#include "page_charge_list.h"
#include "user_info.h"

namespace Ui {
class audit_dlg;
}

class audit_dlg : public CAMDialogBase
{
    Q_OBJECT

public:
    explicit audit_dlg(QWidget *parent = 0);
    ~audit_dlg();

    virtual void _Init();

    void setDlgTitle(const QString &strTitle, const uint &uFlag, const ChargeList::CHARGEINFO &selectedChargeInfo);

    uint getFlag();

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    Ui::audit_dlg *ui;
    int m_iFlag;  //标记是审核还是驳回
    uint __m_uFlag; //返回标志
    ChargeList::CHARGEINFO __m_selectedChargeInfo;
};

#endif // AUDIT_DLG_H
