
// GenAddrTool_MFCDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "GenAddrTool_MFC.h"
#include "GenAddrTool_MFCDlg.h"
#include "afxdialogex.h"

#include <Python.h>
#include <vector>
#include <list>
#include <map>
using namespace std;


HANDLE  g_hMutex = NULL;
HWND  g_hwdBtnGenFile = NULL;   //生成文件按钮句柄 



typedef struct _ADDR
{
	CString strCoinType;//币种
	CString strPrivKey;
	CString strPubKey;
	CString strAddr;
}ADDR;




enum COINTYPE
{
	BTC = 0,
	USDT,
	ETH,
	OWT,
	UP
};


typedef struct _MYPARAM
{
	CEdit	*pOutput;		//输入编辑框	
	CString strCoinType;	//币种
	UINT	uAddrCount;		//生成数量
	BOOL	bIsTestnet;		//是否是测试网络(BTC, USDT)
	CProgressCtrl *pwndProcessCtrl;
	CButton		*pbtnGenFile; //生成文件
	CButton		*pbtnGenAddr; //生成地址
	CStatic   *pStaticProccess; //进度静态文本
}MYPARAM;

//如果未特殊要求, BTC全部使用压缩格式
#define  UINT_BTC_PRIV_KEY_LEN			(52)	//WIF格式私钥 (压缩格式52字符, 未压缩格式51字符)
#define  UINT_BTC_PUB_KEY_LEN			(33*2)	//公钥 (压缩格式33字节 ; 未压缩格式65字节 )
#define  UINT_BTC_ADDR_LEN_MAX			(34)	//地址
#define  UINT_BTC_ADDR_LEN_MIN			(26)	//地址


#define  UINT_ETH_PRIV_KEY_LEN			(64)	//私钥
#define  UINT_ETH_PUB_KEY_LEN			(128)	//公钥
#define  UINT_ETH_ADDR_LEN				(42)	//地址

//XRP是字符长度
#define  UINT_XRP_PRIV_KEY_LEN			(29)	//私钥
#define  UINT_XRP_PUB_KEY_LEN			(52)	//公钥
#define  UINT_XRP_ADDR_LEN				(34)	//地址


//EOS 
#define  UINT_EOS_PRIV_KEY_LEN			(51)	//私钥
#define  UINT_EOS_PUB_KEY_LEN			(53)	//公钥
#define  UINT_EOS_ADDR_LEN				(12)	//地址


#define  UINT_USDP_PRIV_KEY_LEN			(64)	//私钥
#define  UINT_USDP_PUB_KEY_LEN			(33*2)	//公钥
#define  UINT_USDP_ADDR_LEN				(43)	//地址

//#define  STR_BTC_OUTPUT_FILE_PATH			_T("btc_addr_list.addr")	//btc地址导出文件
//#define  STR_ETH_OUTPUT_FILE_PATH			_T("eth_addr_list.addr")	//eth地址导出文件
//#define  STR_UP_OUTPUT_FILE_PATH			_T("up_addr_list.addr")		//up地址导出文件
//#define  STR_USDT_OUTPUT_FILE_PATH			_T("usdt_addr_list.addr")	//usdt地址导出文件
//#define  STR_OWT_OUTPUT_FILE_PATH			_T("owt_addr_list.addr")	//owt地址导出文件

//std::map<CString, CString > g_mapAddr;   //{币种,  地址}
std::list<std::vector<CString>>  g_listAddr;
vector<ADDR> g_vctPrivPubAddr;




// 将Python返回的  [("privkey", "pubkey", "addr"), ("privkey", "pubkey", "addr") ]  转为 c++的  vector<ADDR>
int MyPyTupleListToCPPVector(PyObject* poTupleList, vector<ADDR> &vctAddr)
{
	if (PyList_Check(poTupleList))
	{
		for (Py_ssize_t i = 0; i < PyList_Size(poTupleList); i++)
		{
			PyObject *poAddrTuple = PyList_GetItem(poTupleList, i);
			ADDR tmpADDR;
			if (PyTuple_Check(poAddrTuple))
			{
				for (Py_ssize_t i = 0; i < PyTuple_Size(poAddrTuple); i++)
				{
					PyObject *item = PyTuple_GetItem(poAddrTuple, i);
					//QString strTmp = item;
					//qDebug() << "listToVector_Addr --->" << PyString_AsString(item);
					CString strTmp;
					strTmp += _T("listToVector_Addr --->");
					strTmp += PyString_AsString(item);
#ifdef DEBUG
					OutputDebugString(strTmp);
#endif // DEBUG
					if (0 == i) tmpADDR.strPrivKey = PyString_AsString(item);
					if (1 == i) tmpADDR.strPubKey = PyString_AsString(item);
					if (2 == i) tmpADDR.strAddr = PyString_AsString(item);
				}
				vctAddr.push_back(tmpADDR);
			}
			else
			{
				OutputDebugString(_T("Python返回的list中不是tuple类型"));
				return -1;
			}
		}
	}
	else
	{
		//qDebug() << "返回值类型不是 list";
		OutputDebugString(_T("python返回的数据类型不是 list"));
		return -2;
	}
	return 0;
}




#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGenAddrToolMFCDlg 对话框



CGenAddrToolMFCDlg::CGenAddrToolMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GENADDRTOOL_MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGenAddrToolMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COIN_TYPE, m_cbCoinType);
	DDX_Control(pDX, IDC_PROGRESS_GENADDR, m_wndProcessCtrl);
}

BEGIN_MESSAGE_MAP(CGenAddrToolMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CGenAddrToolMFCDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CGenAddrToolMFCDlg::OnBnClickedGenFile)
END_MESSAGE_MAP()


// CGenAddrToolMFCDlg 消息处理程序

BOOL CGenAddrToolMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//SetWindowText(_T("交易所地址生成工具v1.2  released by yqq at 2019-06-05  tel:18565659593"));
	SetWindowText(_T("交易所地址生成工具v1.6  released by yqq at 2019-12-27  tel:18565659593"));
	g_hMutex = ::CreateMutex(NULL, FALSE, NULL); //创建互斥锁
	m_cbCoinType.InsertString(0, _T("BTC"));
	m_cbCoinType.InsertString(1, _T("ETH"));
	m_cbCoinType.InsertString(2, _T("XRP"));
	m_cbCoinType.InsertString(3, _T("EOS"));
	m_cbCoinType.InsertString(4, _T("USDP"));
	m_cbCoinType.InsertString(5, _T("HTDF"));
	m_cbCoinType.InsertString(6, _T("LTC"));
	m_cbCoinType.InsertString(7, _T("DASH"));
	m_cbCoinType.InsertString(8, _T("HET"));

	m_cbCoinType.SetCurSel(0);

#ifdef _DEBUG
	GetDlgItem(IDC_CHECK_TESTNET)->ShowWindow(SW_SHOW);
#else RELEASE
	GetDlgItem(IDC_CHECK_TESTNET)->ShowWindow(SW_HIDE);
#endif



	g_hwdBtnGenFile = GetDlgItem(IDC_BUTTON1)->m_hWnd;  //获取按钮的句柄


//#ifndef _DEBUG //release版本, 不显示"测试网络"复选框
	//GetDlgItem(IDC_CHECK_TESTNET)->EnableWindow(FALSE);
	//GetDlgItem(IDC_CHECK_TESTNET)->ShowWindow(SW_HIDE);
//#endif




	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGenAddrToolMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGenAddrToolMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGenAddrToolMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}







//比特币地址生成函数
DWORD WINAPI GenAddr(LPVOID lpThreadParameter)
{
	if (NULL == lpThreadParameter)
		return -1;

	MYPARAM *pParams = static_cast<MYPARAM *>(lpThreadParameter);
	UINT uAddrCount = pParams->uAddrCount;
	BOOL bIsTestnet = pParams->bIsTestnet;
	CEdit *pEditOutput = pParams->pOutput;
	CString strCoinType = pParams->strCoinType;
	CProgressCtrl *pwndProcessCtrl = pParams->pwndProcessCtrl;
	CStatic  *pwndProccessStatic = pParams->pStaticProccess;
	CButton *pbtnGenFile = pParams->pbtnGenFile; //生成文件的按钮
	CButton *pbtnGenAddr = pParams->pbtnGenAddr; //生成地址

	if (pParams) 
	{
		delete(pParams);
		pParams = NULL;
	}

	if (0 == uAddrCount || NULL == pEditOutput || strCoinType.IsEmpty())
		return -1;



	char *pszPyFileName;
	char *pszFuncName = "GenMultiAddr";
	if (0 == strCoinType.CompareNoCase(_T("BTC")) || 0 == strCoinType.CompareNoCase(_T("USDT")))
		pszPyFileName = "btc_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("ETH")))
		pszPyFileName = "eth_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("XRP")))
		pszPyFileName = "xrp_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("EOS")))
		pszPyFileName = "eos_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("USDP")))
		pszPyFileName = "usdp_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("HTDF")))
		pszPyFileName = "htdf_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("LTC")))
		pszPyFileName = "ltc_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("DASH")))
		pszPyFileName = "dash_addr_gen";
	else if (0 == strCoinType.CompareNoCase(_T("HET")))
		pszPyFileName = "het_addr_gen";
	else
	{
		return -1;
	}



	::WaitForSingleObject(g_hMutex, INFINITE); //等待互斥锁

	
	//CRect rect;
	//GetDlgItem(IDC_PROGRESS_GENADDR)->GetWindowRect(&rect);
	//GetDlgItem(pEditOutput->m_hWnd, IDC_PROGRESS_GENADDR);
	

	pwndProcessCtrl->SetRange(0, uAddrCount);
	pwndProcessCtrl->ShowWindow(TRUE);

	CString strShowStaticText;
	strShowStaticText.Format(_T("%u/%u"), 0, uAddrCount);
	pwndProccessStatic->SetWindowText(strShowStaticText);
	pwndProccessStatic->ShowWindow(TRUE);

	CRect rect;
	pwndProcessCtrl->GetWindowRect(&rect);
	//pwndProcessCtrl->SetWindowPos(NULL, 200, 280, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	//::SetWindowPos(pwndProcessCtrl->m_hWnd, HWND_TOPMOST, 200, 280, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	//::SetWindowPos(pwndProcessCtrl->m_hWnd, HWND_TOPMOST, 200, 280, 0, 0, SWP_NOZORDER | SWP_NOSIZE);


	DWORD dwStartTime =  GetTickCount(); //程序开始计时

	UINT uSum = 0;
	//for (UINT i = 0; i < uAddrCount / 100; i++)

	CString strTmpOutput;
	pEditOutput->GetWindowText(strTmpOutput);
	CString strTmpTip, strTmp;
	{
		SYSTEMTIME st;
		CString strDate, strTime;
		::GetLocalTime(&st);
		strDate.Format(_T("%4d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
		strTime.Format(_T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
		strTmpTip += strDate + _T(" ") + strTime + _T(" | ");
	}
	strTmp.Format(_T("现在开始生成 %d 个%s %s个地址, 大约需要 %.2f 分钟, 请耐心等待....\r\n\r\n"),
				uAddrCount, strCoinType.GetBuffer(), (TRUE == bIsTestnet)?(_T("测试网")):(_T("主网")), ((double)uAddrCount * 1.0 / 1000.0) * 300.0 / 60.0  );
	strTmpTip += strTmp;
	strTmpOutput += strTmpTip;
	pEditOutput->SetWindowText(strTmpOutput);
	while(uSum < uAddrCount)
	{
		CString strShowStaticText;
		strShowStaticText.Format(_T("%u/%u"), uSum, uAddrCount);
		pwndProccessStatic->SetWindowText(strShowStaticText);
		pwndProccessStatic->UpdateWindow();

		pwndProcessCtrl->SetPos(uSum);
		pwndProcessCtrl->UpdateWindow();
		pbtnGenFile->EnableWindow(FALSE);
		pbtnGenAddr->EnableWindow(FALSE);
		

		//Py_SetPythonHome("C:\\Users\\Administrator\\Desktop\\test\\python27");
		Py_SetPythonHome("./python27");
		Py_Initialize();

		PyObject* pModule = NULL;
		pModule = PyImport_ImportModule(pszPyFileName);
		PyRun_SimpleString("sys.path.append('./')");
		if (NULL == pModule)
		{
			pbtnGenAddr->EnableWindow(TRUE);
			pbtnGenFile->EnableWindow(TRUE);
			pwndProcessCtrl->ShowWindow(FALSE);
			pwndProccessStatic->ShowWindow(FALSE);
			::ReleaseMutex(g_hMutex);//释放锁
			Py_Finalize();

			OutputDebugString(_T("不能导入Python文件, 检查Python文件路径是否正确"));
			AfxMessageBox(_T("不能导入Python文件, 检查Python文件路径是否正确"));
			return -1;
		}


		PyObject* pPyFuncObj = NULL;
		pPyFuncObj = PyObject_GetAttrString(pModule, pszFuncName);
		if (NULL == pPyFuncObj)
		{
			pbtnGenAddr->EnableWindow(TRUE);
			pbtnGenFile->EnableWindow(TRUE);
			pwndProcessCtrl->ShowWindow(FALSE);
			pwndProccessStatic->ShowWindow(FALSE);
			::ReleaseMutex(g_hMutex);//释放锁
			Py_Finalize();

			OutputDebugString(_T("获取函数失败"));
			AfxMessageBox(_T("获取函数失败"));
			return -1;
		}
		
		PyObject *pRet = NULL;
		if (uAddrCount > 300) //如果大于300个, 每次100个
		{

			if (uAddrCount - uSum >= 100)
			{
				pRet = PyObject_CallFunction(pPyFuncObj, (char *)"(ii)", 100, bIsTestnet); //100个一次
				uSum += 100;
			}
			else
			{
				pRet = PyObject_CallFunction(pPyFuncObj, (char *)"(ii)", uAddrCount - uSum, bIsTestnet); //剩余的
				uSum += uAddrCount - uSum;
			}
		}
		else if(60 < uAddrCount && uAddrCount < 100 ) //如果少于300个, 每次二十个即可
		{
			UINT uCountTmp = 20;
			pRet = PyObject_CallFunction(pPyFuncObj, (char *)"(ii)", uCountTmp, bIsTestnet); 
			uSum += uCountTmp;
		}
		else // 小于60个
		{
			UINT uCountTmp = 1;
			pRet = PyObject_CallFunction(pPyFuncObj, (char *)"(ii)", uCountTmp, bIsTestnet); 
			uSum += uCountTmp;
		}
		
		if (NULL == pRet)
		{
			pbtnGenAddr->EnableWindow(TRUE);
			pbtnGenFile->EnableWindow(TRUE);
			pwndProcessCtrl->ShowWindow(FALSE);
			pwndProccessStatic->ShowWindow(FALSE);
			::ReleaseMutex(g_hMutex);//释放锁
			Py_Finalize();
			OutputDebugString(_T("函数返回为空, 请检查"));
			AfxMessageBox(_T("函数返回为空, 请检查"));
			return -1;
		}

	
		std::vector<ADDR> vctAddr;
		int iRet = MyPyTupleListToCPPVector(pRet, vctAddr);
		if (0 != iRet)
		{
			pbtnGenAddr->EnableWindow(TRUE);
			pbtnGenFile->EnableWindow(TRUE);
			pwndProcessCtrl->ShowWindow(FALSE);
			pwndProccessStatic->ShowWindow(FALSE);
			Py_Finalize();
			::ReleaseMutex(g_hMutex);//释放锁
			return iRet;
		}
		
		if (0 == vctAddr.size())
		{
			pbtnGenAddr->EnableWindow(TRUE);
			pbtnGenFile->EnableWindow(TRUE);
			pwndProcessCtrl->ShowWindow(FALSE);
			pwndProccessStatic->ShowWindow(FALSE);
			Py_Finalize();
			::ReleaseMutex(g_hMutex);//释放锁
			AfxMessageBox(_T("Python返回的地址为空"));
			return -1;
		}

		Py_XDECREF(pRet); //减少引用计数
		pRet = NULL;

		

		//g_lstPrivPubAddr += vctAddr;


		CString strOutput;
#ifdef DEBUG_SHOW
		pEditOutput->GetWindowText(strOutput);
#endif
		for (auto item : vctAddr)
		{
			CString strPrivKey(item.strPrivKey);
			CString strPubKey(item.strPubKey);
			CString strAddr(item.strAddr);

			if (0 == strCoinType.CompareNoCase(_T("BTC"))
				|| 0 == strCoinType.CompareNoCase(_T("USDT")))
			{

				if (UINT_BTC_PRIV_KEY_LEN != strPrivKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("私钥长度不对"));
					return -1;
				}

				/*if (UINT_BTC_PUB_KEY_LEN != strPubKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					::ReleaseMutex(g_hMutex);//释放锁
					Py_Finalize();
					AfxMessageBox(_T("公钥长度不对"));
					return -1;
				}*/

				//3开头的比特币地址，34个字符
				//1开头的比特币地址，长度26到34字符
				if (!(UINT_BTC_ADDR_LEN_MIN <= strAddr.GetLength() && strAddr.GetLength() <= UINT_BTC_ADDR_LEN_MAX))
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("地址长度不对"));
					strAddr += _T("$$$$$");
					return -1;
				}
			}
			else if (0 == strCoinType.CompareNoCase(_T("ETH")))
			{
				if (UINT_ETH_PRIV_KEY_LEN != strPrivKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("私钥长度不对"));
					return -1;
				}

				/*if (UINT_ETH_PUB_KEY_LEN != strPubKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("公钥长度不对"));
					return -1;
				}*/

				if (UINT_ETH_ADDR_LEN != strAddr.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("地址长度不对"));
					strAddr += _T("$$$$$");
					return -1;
				}
			}
			else if (0 == strCoinType.CompareNoCase(_T("EOS")))
			{
				if (UINT_EOS_PRIV_KEY_LEN != strPrivKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("私钥长度不对"));
					return -1;
				}

				if (UINT_EOS_ADDR_LEN != strAddr.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("地址长度不对"));
					strAddr += _T("$$$$$");
					return -1;
				}
			}
			else if (0 == strCoinType.CompareNoCase(_T("XRP")))
			{
				if (UINT_XRP_PRIV_KEY_LEN != strPrivKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("私钥长度不对"));
					return -1;
				}

				if (UINT_XRP_ADDR_LEN != strAddr.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("地址长度不对"));
					strAddr += _T("$$$$$");
					return -1;
				}
			}
			else if (0 == strCoinType.CompareNoCase(_T("USDP")) || 0 == strCoinType.CompareNoCase(_T("HTDF")))
			{
				if (UINT_USDP_PRIV_KEY_LEN != strPrivKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("私钥长度不对"));
					return -1;
				}

				/*if (UINT_USDP_PUB_KEY_LEN != strPubKey.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					AfxMessageBox(_T("公钥长度不对"));
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					return -1;
				}*/

				if (UINT_USDP_ADDR_LEN != strAddr.GetLength())
				{
					pbtnGenAddr->EnableWindow(TRUE);
					pbtnGenFile->EnableWindow(TRUE);
					pwndProcessCtrl->ShowWindow(FALSE);
					pwndProccessStatic->ShowWindow(FALSE);
					Py_Finalize();
					::ReleaseMutex(g_hMutex);//释放锁
					AfxMessageBox(_T("地址长度不对"));
					strAddr += _T("$$$$$");
					return -1;
				}

			}


			strOutput += strCoinType + _T("私钥: ") + strPrivKey + _T("\r\n");
			if(0 == strCoinType.CompareNoCase(_T("EOS")))
				strOutput += strCoinType + _T("公钥: ") + strPubKey + _T("\r\n");
			strOutput += strCoinType + _T("地址: ") + strAddr + _T("\r\n");
			strOutput += _T("============================\r\n");
			
			item.strCoinType = strCoinType;
			g_vctPrivPubAddr.push_back(item);

			//插入地址map中
			//g_mapAddr.insert(std::make_pair(strCoinType, strAddr));
			std::vector<CString> vctTmp;
			vctTmp.push_back(strCoinType);
			//vctTmp.push_back(strPrivKey);
			//vctTmp.push_back(strPubKey);
			vctTmp.push_back(strAddr);
			g_listAddr.push_back(vctTmp);

#ifdef _DEBUG
			OutputDebugString(strOutput);
#endif

			Py_XDECREF(pPyFuncObj);
			Py_XDECREF(pModule);
			Py_Finalize();
		}
#ifdef DEBUG_SHOW	
		pEditOutput->SetWindowText(strOutput);
		pEditOutput->SetSel(-1, -1);
#endif
		//pEditOutput->LineScroll(pEditOutput->GetLineCount() - 1, 0);  //滚动到最后一行
	}


	pbtnGenAddr->EnableWindow(TRUE);
	pbtnGenFile->EnableWindow(TRUE);
	pwndProcessCtrl->ShowWindow(FALSE);

	pwndProccessStatic->ShowWindow(FALSE);

	::ReleaseMutex(g_hMutex);//释放锁


	if(TRUE)
	{

		
		DWORD dwOverTime =  GetTickCount(); //程序结束时间

		CString strTmpOutput, strFormatTime, strTmp;
		{
			SYSTEMTIME st;
			CString strDate, strTime;
			::GetLocalTime(&st);
			strDate.Format(_T("%4d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
			strTime.Format(_T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
			strFormatTime += strDate + _T(" ") + strTime + _T(" | ");
		}
		strTmp.Format(_T("已成功向地址缓冲区追加 %d 个%s %s地址.  实际耗时 %.2f 分钟.  \r\n\r\n"),
			uAddrCount, (TRUE == bIsTestnet) ? (_T("测试")) : (_T("主网")), strCoinType.GetBuffer(), (double)(dwOverTime - dwStartTime)/1000.0/60.0 );


		pEditOutput->GetWindowText(strTmpOutput);
		strTmpOutput += strFormatTime + strTmp;

		pEditOutput->SetWindowText(strTmpOutput);
	}
	

	CString strTip;
	if (bIsTestnet)
	{
		strTip.Format(_T("成功! 生成了 %d 个 %s 测试网络地址!"), uAddrCount, strCoinType.GetBuffer());
	}
	else 
	{
		strTip.Format(_T("成功! 生成了 %d 个 %s 主网地址!"), uAddrCount, strCoinType.GetBuffer());
	}


	::SendMessage(g_hwdBtnGenFile, BM_CLICK, 0, 0);
	//AfxMessageBox(strTip);

	return 0;
}



void CGenAddrToolMFCDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();

	CString strCoinType;

	m_cbCoinType.GetLBText(m_cbCoinType.GetCurSel(), strCoinType); //获取当前选择的币种文本
	if (strCoinType.IsEmpty()) {
		AfxMessageBox(_T("请填写币种"));
		return;
	}
	//AfxMessageBox(strCoinType);


	CString strAddrCount;
	UINT uAddrCount = 0;
	//GetDlgItem(IDC_EDIT_TYPE)->SetWindowText()
	GetDlgItem(IDC_EDIT_COUNT)->GetWindowText(strAddrCount);
	if (strAddrCount.IsEmpty()) {
		AfxMessageBox(_T("请填写地址数量"));
		return;
	}

	for (INT iCh = 0; iCh < strAddrCount.GetLength(); iCh++)
	{
		if (!('0' <= strAddrCount.GetAt(iCh) && strAddrCount.GetAt(iCh) <= '9'))
		{
			AfxMessageBox(_T("地址数量格式错误, 请重新输入"));
			return;
		}
	}


	TRY 
	{
		//uAddrCount = _atoi64(strAddrCount);
		uAddrCount = (UINT)_ttoi64(strAddrCount);
	}
	CATCH (CException, e)
	{
		AfxMessageBox(_T("地址数量格式错误, 请重新输入"));
	}
	END_CATCH;

		

	if (uAddrCount <= 0) 
	{
		return;
	}
	if (uAddrCount > 1000)
	{
		CString strTmp;
		strTmp.Format(_T("生成时间很长, 大概需要 %.2f 分钟, 是否继续生成?"), ((double)uAddrCount * 1.0 / 1000.0) * 300.0 / 60.0);
		INT iRet = AfxMessageBox(strTmp, MB_YESNO);
		if (IDNO == iRet)
			return;
	}

	if (g_vctPrivPubAddr.size() > 0)
	{
		CString strTmpCoinType = g_vctPrivPubAddr[0].strCoinType;
		strTmpCoinType = strTmpCoinType.Trim();

		if (0 != strCoinType.CompareNoCase(strTmpCoinType))
		{
			CString strTmp;
			strTmp.Format(_T("发现地址缓冲区有其他币种(%s)地址,如继续生成%s地址, 则会自动清空缓冲区现有的%d个%s地址, 是否继续?"),
						strTmpCoinType, strCoinType, g_vctPrivPubAddr.size(), strTmpCoinType);
			
			INT iRet = AfxMessageBox( strTmp, MB_YESNO);
			if (IDNO == iRet)
				return;

			g_vctPrivPubAddr.clear();
		}
	}



#ifdef _DEBUG
	CString strTmp;
	strTmp.Format(_T("一共 %u 个地址\n"), uAddrCount);
	OutputDebugString(strTmp);
#endif // _DEBUG


	//GetDlgItem(IDC_PROGRESS_GENADDR)->ShowWindow(TRUE);
	

	strCoinType =  strCoinType.MakeUpper();
	if (0 == strCoinType.CompareNoCase(_T("BTC")) 
		||0 == strCoinType.CompareNoCase(_T("ETH"))
		|| 0 == strCoinType.CompareNoCase( _T("XRP"))
		|| 0 == strCoinType.CompareNoCase( _T("EOS"))
		|| 0 == strCoinType.CompareNoCase(_T("USDP"))
		|| 0 == strCoinType.CompareNoCase(_T("HTDF"))
		|| 0 == strCoinType.CompareNoCase(_T("LTC"))
		|| 0 == strCoinType.CompareNoCase(_T("DASH"))
		|| 0 == strCoinType.CompareNoCase(_T("HET"))
		)
	{
		MYPARAM *pParams = new MYPARAM;
		pParams->pOutput = static_cast<CEdit *>( GetDlgItem(IDC_EDIT_OUTPUT));
		pParams->uAddrCount = uAddrCount;
	 	pParams->bIsTestnet =  ((CButton *)GetDlgItem(IDC_CHECK_TESTNET))->GetCheck();
		pParams->strCoinType = strCoinType;
		pParams->pwndProcessCtrl = &m_wndProcessCtrl;
		pParams->pbtnGenFile = (CButton *)GetDlgItem(IDC_BUTTON1);
		pParams->pbtnGenAddr = (CButton *)GetDlgItem(IDOK);
		pParams->pStaticProccess = (CStatic *)GetDlgItem(IDC_STATIC_PROCCESS);
		CreateThread(NULL, 0, GenAddr, (LPVOID)pParams, NULL, NULL);  //创建线程, 生成地址
	}
	else 
	{
		AfxMessageBox(_T("暂不支持") + strCoinType + _T("地址生成"));
	}



}







//生成导出文件
void CGenAddrToolMFCDlg::OnBnClickedGenFile()
{
	// TODO: 在此添加控件通知处理程序代码
	::WaitForSingleObject(g_hMutex, INFINITE); //等待互斥锁




	CStdioFile fileOutputAdmin; //给管理员用的, 包含了私钥
	CStdioFile fileOutput;// (_T("addr.txt"), CFile::modeCreate | CFile::modeWrite);

	if (0 == g_vctPrivPubAddr.size())
	{
		AfxMessageBox(_T("地址缓冲区的地址数量为:0, 请先生成地址!"));
		::ReleaseMutex(g_hMutex);//释放锁
		fileOutput.Close();
		return;
	}

	SYSTEMTIME time;
	GetLocalTime(&time);
	TCHAR tszDate[128] = { 0 };
	_stprintf(tszDate, _T("%d年%02d月%02d日_%02d时%02d分%02d秒"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	CString strCurTime(tszDate);



	CString strTmpCoinType = g_vctPrivPubAddr[0].strCoinType;
	strTmpCoinType = strTmpCoinType.Trim();


	//如果币种目录目录不存在则创建目录
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strTmpCoinType))
	{
		if (FALSE == ::CreateDirectory(strTmpCoinType, NULL))
		{
			AfxMessageBox(_T("创建目录失败!"));
			::ReleaseMutex(g_hMutex);//释放锁
		}
	}


	CString strAddrCount;
	strAddrCount.Format(_T("含%d个地址"), g_vctPrivPubAddr.size());

	CString strExportFilePathAdmin = strTmpCoinType + CString(_T("_私钥文件_")) + strCurTime + CString(_T("_")) + strAddrCount + CString(_T(".bin"));
	CString strExportFilePathNormal = strTmpCoinType + CString(_T("_地址文件_")) + strCurTime + CString(_T("_")) + strAddrCount + CString(_T(".bin"));

	strExportFilePathAdmin = strTmpCoinType + _T("/") + strExportFilePathAdmin;
	strExportFilePathNormal = strTmpCoinType + _T("/") + strExportFilePathNormal;

	//TODO:文件是否加密, 后期考虑
	if (FALSE == fileOutputAdmin.Open(strExportFilePathAdmin, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)) //追加方式, 保留历史数据
	{
		::ReleaseMutex(g_hMutex);//释放锁
		AfxMessageBox(strExportFilePathAdmin + _T(", 打开文件失败,  请检查"));
		return;
	}


	if (FALSE == fileOutput.Open(strExportFilePathNormal, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)) //此文件用于Java导入地址, 故 清掉历史数据
	{
		::ReleaseMutex(g_hMutex);//释放锁
		AfxMessageBox(strExportFilePathNormal + _T(", 打开文件失败,  请检查"));
		return;
	}


	CButton *pBtnGenAddr =  (CButton *)GetDlgItem(IDOK);
	pBtnGenAddr->EnableWindow(FALSE);

	

#ifdef _DEBUG 
	CString strTmp; strTmp.Format(_T("%d\n"), g_vctPrivPubAddr.size());
	OutputDebugString(strTmp);
#endif
	{
		CString strTmpOutput, strFormatTime, strTmp;
		{
			SYSTEMTIME st;
			CString strDate, strTime;
			::GetLocalTime(&st);
			strDate.Format(_T("%4d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
			strTime.Format(_T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
			strFormatTime += strDate + _T(" ") + strTime + _T(" | ");
		}

		CEdit  *pEdOutput = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

		strTmp.Format(_T("开始导出 %d 个地址到文件, 请等待.....\r\n\r\n"), g_vctPrivPubAddr.size());
		pEdOutput->GetWindowText(strTmpOutput);
		strTmpOutput += strFormatTime;
		strTmpOutput += strTmp;
		pEdOutput->SetWindowText(strTmpOutput);
	}
	

	for (auto it : g_vctPrivPubAddr)
	{
		CString strLineAdmin = _T("");
		CString strLine = _T("");

		strLineAdmin += it.strCoinType + _T("\t");
		strLineAdmin += it.strAddr+ _T("\t");
		strLineAdmin += it.strPrivKey + _T("\t");
		if (0 == strTmpCoinType.CompareNoCase(_T("EOS")))
		{
			strLineAdmin += it.strPubKey+ _T("\t"); // 公钥
		}
		strLineAdmin.Trim();
		strLineAdmin += _T("\n");


		strLine+= it.strCoinType + _T("\t");
		strLine+= it.strAddr+ _T("\t");


		strLine.Trim();
		strLine += _T("\n");
			
		fileOutputAdmin.WriteString(strLineAdmin); //包含私钥
		fileOutput.WriteString(strLine); //币种和地址
	}


	{
		CString strTmpOutput, strFormatTime, strTmp;
		{
			SYSTEMTIME st;
			CString strDate, strTime;
			::GetLocalTime(&st);
			strDate.Format(_T("%4d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
			strTime.Format(_T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
			strFormatTime += strDate + _T(" ") + strTime + _T(" | ");
		}

		CEdit  *pEdOutput = (CEdit *)GetDlgItem(IDC_EDIT_OUTPUT);

		pEdOutput->GetWindowText(strTmpOutput);
		strTmpOutput += strFormatTime;

		strTmp.Format(_T("-------成功!----- 生成并导出成功!  已导出 %d 个地址到 %s 文件夹下:\r\n\t\t\t \"%s\" \r\n\t\t\t \"%s\" \r\n\t\t  请自行查看. \r\n\r\n"),
				g_vctPrivPubAddr.size(), strTmpCoinType, strExportFilePathAdmin, strExportFilePathNormal);
		strTmpOutput += strTmp;
		pEdOutput->SetWindowText(strTmpOutput);

	}

	g_vctPrivPubAddr.clear(); //清空缓存区
	


	fileOutput.Close();
	fileOutputAdmin.Close();

	pBtnGenAddr->EnableWindow(TRUE);
	::ReleaseMutex(g_hMutex);//释放锁
	//AfxMessageBox(_T("文件生成成功"));
}



