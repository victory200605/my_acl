// MidCoderGeneratorDlg.h : ͷ�ļ�
//

#pragma once

#define MAX_BUFF_SIZE 4096


// CMidCoderGeneratorDlg �Ի���
class CMidCoderGeneratorDlg : public CDialog
{
// ����
public:
	CMidCoderGeneratorDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MIDCODERGENERATOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
