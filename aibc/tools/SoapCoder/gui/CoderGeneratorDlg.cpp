// CoderGeneratorDlg.cpp : 实现文件
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


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CCoderGeneratorDlg 对话框



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


// CCoderGeneratorDlg 消息处理程序

BOOL CCoderGeneratorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetCurrentDirectory( MAX_BUFF_SIZE, csCurrentDir);
	coProgress.SetRange ( 0, 100 );
	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
	
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCoderGeneratorDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
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
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	OnOverViewFile( coStrXSDFileName, "XSD文件 (*.xsd)|*.xsd||" );
	UpdateData(false);}

void CCoderGeneratorDlg::OnBnClickedButtonOpenWsdl()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	OnOverViewFile( coStrWSDLFileName, "WSDL文件 (*.wsdl)|*.wsdl||" );
	UpdateData(false);}

void CCoderGeneratorDlg::OnBnClickedButtonGencoder()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData( TRUE );

	if ( coStrWSDLFileName.GetLength() < 3 )
	{
		MessageBox("请选择WSDL文件！");
		return;
	}

	clsFileName loWSDLFileName(coStrWSDLFileName);
	
	try
	{
		// H文件和CPP文件生成解释上下文
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

		// 读取目标文件名，并创建目标文件
		// 读取目标文件名，并创建目标文件
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
			MessageBox( ::Format("打开目标文件[%s]/[%s]失败！", loStrHFileName.c_str(), loStrCppFileName.c_str()) );
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
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	OnOverViewFile( coStrConfigFileName, "lua文件 (*.lua)|*.lua||" );
	UpdateData(false);
}
