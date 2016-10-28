// CoderGeneratorDlg.h : ͷ�ļ�
//

#pragma once

#include "../CoderGenerator/Exception.h"
#include "../CoderGenerator/Context.h"
#include "../CoderGenerator/CodeBuilder.h"
#include "../CoderGenerator/AIFile.h"

// CCoderGeneratorDlg �Ի���
class CCoderGeneratorDlg : public CDialog
{
// ����
public:
	CCoderGeneratorDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CODERGENERATOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

protected:
	bool LoadWsdlFile( );
	bool LoadXsdFile( );
	bool LoadFile( );

// ʵ��
protected:
	HICON m_hIcon;

	//�Զ����Ա
	CString coStrXSDFileName;
	CString coStrWSDLFileName;
	CString coStrConfigFileName;
	TCHAR csCurrentDir[MAX_BUFF_SIZE];
	CProgressCtrl	coProgress;

	// �Զ��庯��
	afx_msg void OnOverViewFile( CString& aoStrFileName, const char* apsSuffix );

	// ���ɵ���Ϣӳ�亯��
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
