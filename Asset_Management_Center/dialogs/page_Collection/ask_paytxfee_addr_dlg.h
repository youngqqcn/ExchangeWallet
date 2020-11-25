#ifndef ASK_PAYTXFEE_ADDR_DLG_H
#define ASK_PAYTXFEE_ADDR_DLG_H

#include <QDialog>

namespace Ui {
class CAskPayTxFeeAddrDlg;
}

class CAskPayTxFeeAddrDlg :  public QDialog
{
    Q_OBJECT

public:
    explicit CAskPayTxFeeAddrDlg(const QString &strCoinType, const QString &strCollectAddr, bool bEnableEdit = false, QWidget *parent = 0);
    ~CAskPayTxFeeAddrDlg();

private slots:
    void on_btn_Query_clicked();

    void on_btn_Back_clicked();


public:
    QString  m_strPayTxFeeAddr;//支付手续费的地址
    uint     m_nOnceCount; //本次能归集的数量(多少个地址)  取决于支付手续费的地址的目前拥有的utxo数量(utxo的金额大于0.0001)

private:
    Ui::CAskPayTxFeeAddrDlg *ui;
};

#endif // ASK_PAYTXFEE_ADDR_DLG_H
