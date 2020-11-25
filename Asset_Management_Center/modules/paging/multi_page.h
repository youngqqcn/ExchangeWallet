#ifndef MULTI_PAGE_H
#define MULTI_PAGE_H

#include <QWidget>
#include <QList>

class QEvent;
class QLabel;

namespace Ui {
class CMultiPage;
}

class CMultiPage : public QWidget
{
    Q_OBJECT

public:
    explicit CMultiPage(int iBlockSize = 3, QWidget *parent = 0);
    ~CMultiPage();

    int getBlockSize() const;
    int getMaxPage() const;
    int getCurrentPage() const;

    //以下两个给外部调用,用以初始化该组件
    void setMaxPage(int iMaxPage);     //修改总页数时调用
    void setCurrentPage(int iCurrentPage, bool bSignalEmited = false);     //设置当前页时调用

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

signals:
    void sendCurrentPageChanged(int iPage);     //当前显示的页改变时发送信号

private:
    void initialize();
    void setBlockSize(int iBlockSize);
    void updatePageLabels();

private:
    Ui::CMultiPage *ui;
    int     m_iBlockSize;       //分页的块数 分别为左中右三块
    int     m_iCurrentPage;     //当前页
    int     m_iMaxPage;         //最大页数
    QList<QLabel *> *m_pPageLabels;     //QLabel控件
};

#endif // MULTI_PAGE_H
