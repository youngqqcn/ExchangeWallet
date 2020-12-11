
// GenAddrTool_MFCDlg.h: 头文件
//

#pragma once


// CGenAddrToolMFCDlg 对话框
class CGenAddrToolMFCDlg : public CDialogEx
{
// 构造
public:
	CGenAddrToolMFCDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GENADDRTOOL_MFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持




// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	// //币种
	CComboBox m_cbCoinType;
	afx_msg void OnBnClickedGenFile();
	// 生成地址的进度条
	CProgressCtrl m_wndProcessCtrl;
};
