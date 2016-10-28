// MidCoderGeneratorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MidCoderGenerator.h"
#include "MidCoderGeneratorDlg.h"
#include ".\midcodergeneratordlg.h"

#include "../CoderGenerator/Context.h"
#include "../CoderGenerator/CppContext.h"
#include "../CoderGenerator/XMLParser.h"
#include "../CoderGenerator/CodeBuilder.h"
#include "../CoderGenerator/AIFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMidCoderGeneratorDlg �Ի���



CMidCoderGeneratorDlg::CMidCoderGeneratorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMidCoderGeneratorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMidCoderGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text( pDX, IDC_EDIT_FILEPATH, coStrFileName );
	DDX_Control( pDX, IDC_PROGRESS, coProgress );
}

BEGIN_MESSAGE_MAP(CMidCoderGeneratorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
END_MESSAGE_MAP()


// CMidCoderGeneratorDlg ��Ϣ�������

BOOL CMidCoderGeneratorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��\������...\���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	GetCurrentDirectory( MAX_BUFF_SIZE, csCurrentDir);
	coProgress.SetRange ( 0, 100 );
	
	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

void CMidCoderGeneratorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMidCoderGeneratorDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
HCURSOR CMidCoderGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMidCoderGeneratorDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	CFileDialog loFileDlg( TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "MIB xml�ļ� (*.xml)|*.xml||" );
	if ( loFileDlg.DoModal() == IDOK )
	{
		coStrFileName = loFileDlg.GetPathName();
	}
	UpdateData(false);
}

void CMidCoderGeneratorDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData( TRUE );

	coProgress.SetPos( 0 );

	if ( coStrFileName.GetLength() < 3 )
	{
		MessageBox("��ѡ��xml�ļ���");
		return;
	}
	clsFileName loFileName(coStrFileName);
	TObjectList loObjectList;
	try
	{
		// �����ļ�
		CString loStrHConfFileName;
		CString loStrCppConfFileName;
		loStrHConfFileName.Format( "%s/../config/h.conf", csCurrentDir );
		loStrCppConfFileName.Format( "%s/../config/cpp.conf", csCurrentDir );
		// H�ļ���CPP�ļ����ɽ���������
		clsContext loHContext;
		clsCppContext loCppContext;
		loHContext.SetVariable( "FILENAME_BASE", loFileName.GetBaseName() );
		loHContext.LoadFormFile( loStrHConfFileName );
		loCppContext.SetVariable( "FILENAME_BASE", loFileName.GetBaseName() );
		loCppContext.LoadFormFile( loStrCppConfFileName );

		// ��ȡĿ���ļ�����������Ŀ���ļ�
		std::string lsDir, loStrHFileName, loStrCppFileName;
		lsDir = csCurrentDir;
		lsDir += "\\Code\\";

		::CreateDirectory( lsDir.c_str(), NULL);

		loHContext.GetOutputFileName( loStrHFileName, lsDir.c_str() );
		loCppContext.GetOutputFileName( loStrCppFileName, lsDir.c_str() );

		clsFile loHOutPut(loStrHFileName.c_str(), "w");
		clsFile loCppOutPut(loStrCppFileName.c_str(), "w");
		if ( loHOutPut.Fail() || loCppOutPut.Fail() )
		{
			MessageBox( ::Format("��Ŀ���ļ�[%s]/[%s]ʧ�ܣ�", loStrHFileName.c_str(), loStrCppFileName.c_str()) );
			return;
		}

		std::string losXml;
		const char* lpsLine = NULL;
		clsFile loInPut( coStrFileName, "r" );
		while ( ( lpsLine = loInPut.GetLine( ) ) != NULL )
		{
			losXml += lpsLine;
		}

		clsXMLParser loParser;
		loParser.Parser( losXml.c_str(), loObjectList );

		clsHCodeBuilder loHBuilder;
		clsCppCodeBuilder loCppBuilder;

		/////////////////////////////////////////
		loHOutPut << loHBuilder.Build( loObjectList, loHContext ) << "\n";
		loCppOutPut << loCppBuilder.Build( loObjectList, loCppContext ) << "\n"; 

		coProgress.SetPos( 100 );
	}
	catch ( clsException& e )
	{
		coProgress.SetPos( 0 );
		MessageBox ( e.What() );
	}
	catch ( const LuaException& e )
	{
		coProgress.SetPos( 0 );
		MessageBox ( e.GetMessage() );
	}

	clsXMLParser::Recycle( loObjectList );
}
