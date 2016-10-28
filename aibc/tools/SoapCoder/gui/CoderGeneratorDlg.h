// CoderGeneratorDlg.h : 头文件
//

#pragma once

#include "../CoderGenerator/Exception.h"
#include "../CoderGenerator/Context.h"
#include "../CoderGenerator/CodeBuilder.h"
#include "../CoderGenerator/AIFile.h"

// CCoderGeneratorDlg 对话框
class CCoderGeneratorDlg : public CDialog
{
// 构造
public:
	CCoderGeneratorDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CODERGENERATOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

protected:
	bool LoadWsdlFile( );
	bool LoadXsdFile( );
	bool LoadFile( );

// 实现
protected:
	HICON m_hIcon;

	//自定义成员
	CString coStrXSDFileName;
	CString coStrWSDLFileName;
	CString coStrConfigFileName;
	TCHAR csCurrentDir[MAX_BUFF_SIZE];
	CProgressCtrl	coProgress;

	// 自定义函数
	afx_msg void OnOverViewFile( CString& aoStrFileName, const char* apsSuffix );

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpenXsd();
	afx_msg void OnBnClickedButtonOpenWsdl();
	afx_msg void OnBnClickedButtonGencoder();
	afx_msg void OnBnClickedButton1();
};
