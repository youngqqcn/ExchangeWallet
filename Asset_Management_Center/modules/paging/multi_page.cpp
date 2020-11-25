#include "multi_page.h"
#include "ui_multi_page.h"
#include <QtGlobal>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>

CMultiPage::CMultiPage(int iBlockSize/* = 3*/, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CMultiPage)
{
    ui->setupUi(this);

    setBlockSize(iBlockSize);
    initialize();

    m_iMaxPage = 0;
    setMaxPage(1);
}

CMultiPage::~CMultiPage()
{
    delete ui;
    delete m_pPageLabels;
}

int CMultiPage::getBlockSize() const
{
    return m_iBlockSize;
}

int CMultiPage::getMaxPage() const
{
    return m_iMaxPage;
}

int CMultiPage::getCurrentPage() const
{
    return m_iCurrentPage;
}

void CMultiPage::setMaxPage(int iMaxPage)
{
    iMaxPage = qMax(iMaxPage, 1);

    if (m_iMaxPage != iMaxPage)
    {
        m_iMaxPage = iMaxPage;
        m_iCurrentPage = 1;
        updatePageLabels();
    }
}

void CMultiPage::setCurrentPage(int iCurrentPage, bool bSignalEmited/* = false*/)
{
    iCurrentPage = qMax(iCurrentPage, 1);
    iCurrentPage = qMin(iCurrentPage, m_iMaxPage);

    if (iCurrentPage != m_iCurrentPage)
    {
        m_iCurrentPage = iCurrentPage;
        updatePageLabels();
        if (bSignalEmited)
            emit sendCurrentPageChanged(iCurrentPage);
    }
}

bool CMultiPage::eventFilter(QObject *watched, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonRelease)
    {
        int iPage = -1;
        if (watched == ui->previousPageLabel)
            iPage = getCurrentPage() - 1;

        if (watched == ui->nextPageLabel)
            iPage = getCurrentPage() + 1;

        for (int i = 0; i < m_pPageLabels->count(); ++i)
        {
            if (watched == m_pPageLabels->at(i))
            {
                iPage = m_pPageLabels->at(i)->text().toInt();
                break;
            }
        }

        if (-1 != iPage)
        {
            setCurrentPage(iPage, true);
            return true;
        }
    }

    if (watched == ui->pageLineEdit && e->type() == QEvent::KeyRelease)
    {
        QKeyEvent *key = static_cast<QKeyEvent *>(e);
        if (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return)
        {
            setCurrentPage(ui->pageLineEdit->text().toUInt(), true);
            return true;
        }
    }

    return QWidget::eventFilter(watched, e);
}

void CMultiPage::initialize()
{
    ui->pageLineEdit->installEventFilter(this);
    ui->pageLineEdit->setValidator(new QIntValidator(1, 10000000, this));

    ui->previousPageLabel->setProperty("page", "true");
    ui->nextPageLabel->setProperty("page", "true");
    ui->previousPageLabel->setCursor(Qt::PointingHandCursor);
    ui->nextPageLabel->setCursor(Qt::PointingHandCursor);
    ui->previousPageLabel->installEventFilter(this);
    ui->nextPageLabel->installEventFilter(this);

    m_pPageLabels = new QList<QLabel *>();

    QHBoxLayout *leftLayout = new QHBoxLayout();
    QHBoxLayout *centerLayout = new QHBoxLayout();
    QHBoxLayout *rightLayout = new QHBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(3);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(3);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(3);

    for (int i = 0; i < m_iBlockSize * 3; ++i)
    {
        QLabel *label = new QLabel(QString::number(i + 1));
        label->setProperty("page", "true");
        label->setCursor(Qt::PointingHandCursor);
        label->setMinimumWidth(20);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->installEventFilter(this);

        m_pPageLabels->append(label);

        if (i < m_iBlockSize)
            leftLayout->addWidget(label);
        else if (i < m_iBlockSize * 2)
            centerLayout->addWidget(label);
        else
            rightLayout->addWidget(label);
    }

    ui->leftPagesWidget->setLayout(leftLayout);
    ui->centerPagesWidget->setLayout(centerLayout);
    ui->rightPagesWidget->setLayout(rightLayout);
}

void CMultiPage::setBlockSize(int iBlockSize)
{
    //为了便于计算,blockSize最小为3,且为奇数
    iBlockSize = qMax(iBlockSize, 3);
    if (iBlockSize % 2 == 0)
        iBlockSize++;
    this->m_iBlockSize = iBlockSize;
}

void CMultiPage::updatePageLabels()
{
    ui->leftSeparateLabel->hide();
    ui->rightSeparateLabel->hide();

    if (m_iMaxPage <= m_iBlockSize * 3)
    {
        for (int i = 0; i < m_pPageLabels->count(); i += 1)
        {
            QLabel *label = m_pPageLabels->at(i);

            if (i < m_iMaxPage)
            {
                label->setText(QString::number(i + 1));
                label->show();
            }
            else
                label->hide();

            if (m_iCurrentPage - 1 == i)
            {
                label->setProperty("currentPage", "true");
                label->setCursor(Qt::PointingHandCursor);
            }
            else
            {
                label->setProperty("currentPage", "false");
                label->setCursor(Qt::PointingHandCursor);
            }
            label->setStyleSheet("/**/");
        }
        return;
    }

    /*
      以下情况是maxPage大于blockSize * 3,这种情况所有的Label都要显示
      c 为 currentPage
      b 为 blockSize
      m 为 maxPage

      1. c ∈ [1, b + b/2 + 1]: 显示前 b * 2 个, 后 b 个: 只显示右边的分隔符
      2. c ∈ [m - b - b/2, m]: 显示前 b 个, 后 b * 2 个: 只显示左边的分隔符
      3. 显示[1, b], [c - b/2, c + b/2], [m - 2*b + 1, m]: 两个分隔符都显示
    */
    int c = m_iCurrentPage;
    int b = m_iBlockSize;
    int m = m_iMaxPage;
    int iCenterStartPage = 0;

    if (c >= 1 && c <= b + b / 2 + 1)
    {
        iCenterStartPage = b + 1;
        ui->rightSeparateLabel->show();
    }
    else if (c >= m - b - b / 2 && c <= m)
    {
        iCenterStartPage = m - b - b + 1;
        ui->leftSeparateLabel->show();
    }
    else
    {
        iCenterStartPage = c - b / 2;
        ui->leftSeparateLabel->show();
        ui->rightSeparateLabel->show();
    }

    for (int i = 0; i < b; ++i)
    {
        m_pPageLabels->at(i)->setText(QString::number(i + 1));
        m_pPageLabels->at(b + i)->setText(QString::number(iCenterStartPage + i));
        m_pPageLabels->at(3 * b - i - 1)->setText(QString::number(m - i));
    }

    for (int i = 0; i < m_pPageLabels->count(); ++i)
    {
        QLabel *label = m_pPageLabels->at(i);
        int page = label->text().toInt();
        if (page == m_iCurrentPage) {
            label->setProperty("currentPage", "true");
        } else {
            label->setProperty("currentPage", "false");
        }

        label->setStyleSheet("/**/");
        label->show();
    }
}
