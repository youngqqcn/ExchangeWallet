#ifndef RAW_TX_UTILS_BASE_H
#define RAW_TX_UTILS_BASE_H

#include "rawtx_comman.h"
#include <QObject>

namespace rawtx
{
#if !defined(Interface)
    #define Interface struct        //接口类型
#endif
#if !defined(Implement)
    #define Implement struct        //接口实现
#endif

//参数基类, 所有币种rawtx的参数都要继承此类
Interface IRawTxUtilsParamBase
{
    explicit IRawTxUtilsParamBase()  {  Init();  }
    virtual ~IRawTxUtilsParamBase() {}

    virtual void Init() = 0 {}                      //所有子类必须实现Init, 并在构造函数总调用
    virtual bool ParamsCheck(QString *pStrErr)  = 0 { *pStrErr = "";}               //参数检查
};


//所有裸交易接口类, 纯虚类(以 "I" 开头)
Interface IRawTxUtils : public QObject
{
public:
    explicit IRawTxUtils(QObject *parent = nullptr);
    virtual ~IRawTxUtils();

public:
    enum {
      NO_ERROR = 0,
      HTTP_ERR = 1,
      INVALID_DATA_ERR = 2, //无data字段
      JSON_ERR = 3 ,//解析JSon出错
      ARGS_ERR=4,//函数参数错误
      INVALID_ADDR_ERR = 5,//提币地址不合法
      FILE_NOT_EXIST= 6, //文件不存在
      FILE_OPEN_FAIL = 7, //打开文件失败
      AMOUNT_NOT_ENOUGH = 10009, //冷地址金额不足
      COSMOS_NO_TX_ADDR = 20001, //cosmos 未交易过的地址
      HTDF_USDP_LOST_TXID_BUG  = 90002, // 临时处理HTDF和USDP Txid丢失的bug, 等公链修复bug之后, 去掉此选项    2019-06-01
    };

public:

    //创建裸交易的接口
    virtual int CreateRawTx(IRawTxUtilsParamBase *pParam) = 0 ;

    //导出交易接口
    virtual int ExportRawTx(IRawTxUtilsParamBase *pParam) = 0;

    //导入交易接口
    virtual int ImportRawTx(IRawTxUtilsParamBase *pParam) = 0;

    //广播交易接口
    virtual int BroadcastRawTx(IRawTxUtilsParamBase *pParam) = 0;

public:
    //验证地址是否合法的接口
    //返回值: 合法:true   非法:false
    //pStrErrMsg:如果地址非法, 返回地址错误详细信息
    virtual bool IsValidAddr(const QString &strAddr, QString *pStrErrMsg = NULL) noexcept(true);


protected:

    //同步方式请求, 通过URL传参
    virtual int _SyncPostByURL(const QString &strInUrl, QByteArray &byteArrayRet) noexcept(false);

    //同步方式请求, 通过Json传参
    //如果  不要需要  Auth, 可以设置 strAuth为空
    virtual int _SyncPostByJson(const QString &strUrl, const QJsonObject &joPostData,
                QByteArray &byteRet, QString strContType="text/plain", QString strAuth=QString(),
                bool bShowReqData=true)noexcept(false);



    virtual int _SyncGetByURL(const QString &strInUrl, QByteArray &byteArrayRet) noexcept(false);

protected:
};


//交易信息导出导入的模板基类, 所有导入导出参数子类都必须继承此类
template< typename T = IRawTxUtilsParamBase > /*注意: T 必须是CRawTxUtilsParamBase子类*/
Implement CImpExpParamBase: public IRawTxUtilsParamBase
{
public:
    explicit CImpExpParamBase() { m_bIsCollection = false; Init();}
    virtual void Init() override {_m_vctParamItems.clear();}
    virtual bool ParamsCheck(QString *pStrErr) override
    {
        if(m_strFilePath.isEmpty()){ if(NULL != pStrErr ) *pStrErr = QString("m_strFilepath is empty."); return false; }
        if(m_strCoinType.isEmpty()){ if(NULL != pStrErr ) *pStrErr = QString("m_strCoinType is empty."); return false; }
        for(size_t i = 0; i < _m_vctParamItems.size(); i++) { if(false ==  _m_vctParamItems[i].ParamsCheck(pStrErr)) return false; }
        return true;
    }
    CImpExpParamBase(const T& param){this->operator = (param);}
    void push_back(const T& param) { _m_vctParamItems.push_back(param); }
    void clear() { _m_vctParamItems.clear(); }
    size_t size() const { return _m_vctParamItems.size(); }
    const T& operator [] (size_t pos) const { return _m_vctParamItems[pos]; }
    T& operator [] (size_t pos)
    {
        auto it = _m_vctParamItems.begin(); for(size_t i = 0 ; i < pos; i++, it++) { }
        return (*it);
    }
    T& operator = (const T& params)
    {
        _m_vctParamItems.clear();
        for(size_t i = 0; i < params.size(); i++) _m_vctParamItems.push_back(params[i]);
        this->m_strFilePath = params.m_strFilePath;
        this->m_strCoinType = params.m_strCoinType;
        this->m_bIsCollection = params.m_bIsCollection;
        return (*this);
    }

public:
    QString         m_strFilePath;
    QString         m_strCoinType;
    bool            m_bIsCollection;
protected:
    std::vector<T>  _m_vctParamItems;
};






}
#endif // RAW_TX_UTILS_BASE_H
