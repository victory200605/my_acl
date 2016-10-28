// CoderGeneratorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CoderGenerator.h"
#include "CoderGeneratorDlg.h"

#include "../CoderGenerator/Service.h"
#include "../CoderGenerator/CodeBuilder.h"
#include "../CoderGenerator/XMLParser.h"
#include "../CoderGenerator/Context.h"
#include ".\codergeneratordlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#if !defined(EOL)
#define EOL "\n"
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


// CCoderGeneratorDlg �Ի���



CCoderGeneratorDlg::CCoderGeneratorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCoderGeneratorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoderGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text( pDX, IDC_XSD_FILE_NAME, coStrXSDFileName );
	DDX_Text( pDX, IDC_WSDL_FILE_NAME, coStrWSDLFileName );
	DDX_Text( pDX, IDC_CONFIG_FILE_NAME, coStrConfigFileName );
	DDX_Control( pDX, IDC_PROGRESS, coProgress );
}

BEGIN_MESSAGE_MAP(CCoderGeneratorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_OPEN_XSD, OnBnClickedButtonOpenXsd)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_WSDL, OnBnClickedButtonOpenWsdl)
	ON_BN_CLICKED(IDC_BUTTON_GENCODER, OnBnClickedButtonGencoder)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// CCoderGeneratorDlg ��Ϣ�������

BOOL CCoderGeneratorDlg::OnInitDialog()
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
	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
	
	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

void CCoderGeneratorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCoderGeneratorDlg::OnPaint() 
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
HCURSOR CCoderGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCoderGeneratorDlg::OnOverViewFile( CString& aoStrFileName, const char* apsSuffix )
{
	CFileDialog loFileDlg( TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, apsSuffix );
	if ( loFileDlg.DoModal() == IDOK )
	{
		aoStrFileName = loFileDlg.GetPathName();
	}
}

void CCoderGeneratorDlg::OnBnClickedButtonOpenXsd()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	OnOverViewFile( coStrXSDFileName, "XSD�ļ� (*.xsd)|*.xsd||" );
	UpdateData(false);}

void CCoderGeneratorDlg::OnBnClickedButtonOpenWsdl()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	OnOverViewFile( coStrWSDLFileName, "WSDL�ļ� (*.wsdl)|*.wsdl||" );
	UpdateData(false);}

void CCoderGeneratorDlg::OnBnClickedButtonGencoder()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData( TRUE );

	if ( coStrWSDLFileName.GetLength() < 3 )
	{
		MessageBox("��ѡ��WSDL�ļ���");
		return;
	}

	clsFileName loWSDLFileName(coStrWSDLFileName);
	
	try
	{
		// H�ļ���CPP�ļ����ɽ���������
		clsContext loContext;
		loContext.SetVariable( "FILENAME_BASE", loWSDLFileName.GetBaseName() );
		if ( ((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck() )
		{
			loContext.SetVariable( "SOAP_STYLE", "rpc" );
			loContext.SetVariable( "SOAP_USE",   "encoded" );
		}
		else if ( ((CButton *)GetDlgItem(IDC_RADIO2))->GetCheck() )
		{
			loContext.SetVariable( "SOAP_STYLE", "rpc" );
			loContext.SetVariable( "SOAP_USE",   "literal" );
		}
		else if ( ((CButton *)GetDlgItem(IDC_RADIO3))->GetCheck() )
		{
			loContext.SetVariable( "SOAP_STYLE", "document" );
			loContext.SetVariable( "SOAP_USE",   "encoded" );
		}
		else if ( ((CButton *)GetDlgItem(IDC_RADIO4))->GetCheck() )
		{
			loContext.SetVariable( "SOAP_STYLE", "document" );
			loContext.SetVariable( "SOAP_USE",   "literal" );
		}

		loContext.LoadFormFile( coStrConfigFileName );

		// ��ȡĿ���ļ�����������Ŀ���ļ�
		// ��ȡĿ���ļ�����������Ŀ���ļ�
		std::string lsDir, loStrHFileName, loStrCppFileName;
		lsDir = csCurrentDir;
		lsDir += "\\Code\\";

		::CreateDirectory( lsDir.c_str(), NULL);

		loStrHFileName = lsDir + loContext.GetOutputFileName() + ".h";
		loStrCppFileName = lsDir + loContext.GetOutputFileName() + ".cpp";

		clsFile loHOutPut(loStrHFileName.c_str(), "w");
		clsFile loCppOutPut(loStrCppFileName.c_str(), "w");
		if ( loHOutPut.Fail() || loCppOutPut.Fail() )
		{
			MessageBox( ::Format("��Ŀ���ļ�[%s]/[%s]ʧ�ܣ�", loStrHFileName.c_str(), loStrCppFileName.c_str()) );
			return;
		}

		InitializeNatureType( loContext );

		//////////////////////////////////////////
		coProgress.SetPos(0) ;
		/////////////////////////////////////////
		std::string loStrBuffer;
		std::string liLine;
		const char* lpsLine = NULL;
		if ( coStrXSDFileName.GetLength() > 3 )
		{
			/////////////////////////////////////////
			coProgress.SetPos(5) ;
			/////////////////////////////////////////
			clsXmlSchemaParser loXSDParser;
			loXSDParser.ParserFromFile( coStrXSDFileName );
		}

		/////////////////////////////////////////
		coProgress.SetPos(10) ;
		//////////////////////for wsdl file////////////////////////
		loStrBuffer = "";
		clsFile loWSDLInPut( coStrWSDLFileName, "r" );
		while ( ( lpsLine = loWSDLInPut.GetLine( ) ) != NULL )
		{
			loStrBuffer += lpsLine;
		}
		/////////////////////////////////////////
		coProgress.SetPos(15);
		/////////////////////////////////////////
		clsXmlWSDLParser   loWSDLParser(loContext);
		clsXmlSchemaParser loXSDParser;

		loXSDParser.ParserFromBuffer( loStrBuffer.c_str() );
		loWSDLParser.ParserFromBuffer( loStrBuffer.c_str() );

		/////////////////////////////////////////
		coProgress.SetPos(65) ;
		//Note
		/////////////////////////////////////////

		clsCodeBuilder loHBuilder( loContext );

		/////////////////////////////////////////
		loHOutPut << loHBuilder.BuildHCoder() << EOL;
		coProgress.SetPos(75);
		loCppOutPut << loHBuilder.BuildCppCoder() << EOL; 
		coProgress.SetPos(85);

		/////////////////////////////////////////
		coProgress.SetPos(100) ;
		/////////////////////////////////////////

	}
	catch ( LuaException& e )
	{
		coProgress.SetPos(0) ;
		MessageBox ( e.GetMessage() );
	}
	catch (clsException& e)
	{
		coProgress.SetPos(0) ;
		MessageBox ( e.What() );
	}

	clsTypes::Release();
	clsMessage::Release();
	clsPortType::Release();
	clsBinding::Release();
	clsService::Release();
}

void CCoderGeneratorDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	OnOverViewFile( coStrConfigFileName, "lua�ļ� (*.lua)|*.lua||" );
	UpdateData(false);
}
