// MidCoderGeneratorDlg.h : 头文件
//

#pragma once

#define MAX_BUFF_SIZE 4096


// CMidCoderGeneratorDlg 对话框
class CMidCoderGeneratorDlg : public CDialog
{
// 构造
public:
	CMidCoderGeneratorDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MIDCODERGENERATOR_DIALOG };

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
	afx_msg void OnBnClickedButton1();

protected:
	CString coStrFileName;
	TCHAR   csCurrentDir[MAX_BUFF_SIZE];
	CProgressCtrl coProgress;
public:
	afx_msg void OnBnClickedButton2();
};
