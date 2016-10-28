// MidCoderGeneratorDlg.cpp : 实现文件
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


// CMidCoderGeneratorDlg 对话框



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


// CMidCoderGeneratorDlg 消息处理程序

BOOL CMidCoderGeneratorDlg::OnInitDialog()
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
	
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMidCoderGeneratorDlg::OnPaint() 
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
HCURSOR CMidCoderGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMidCoderGeneratorDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	CFileDialog loFileDlg( TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "MIB xml文件 (*.xml)|*.xml||" );
	if ( loFileDlg.DoModal() == IDOK )
	{
		coStrFileName = loFileDlg.GetPathName();
	}
	UpdateData(false);
}

void CMidCoderGeneratorDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData( TRUE );

	coProgress.SetPos( 0 );

	if ( coStrFileName.GetLength() < 3 )
	{
		MessageBox("请选择xml文件！");
		return;
	}
	clsFileName loFileName(coStrFileName);
	TObjectList loObjectList;
	try
	{
		// 配置文件
		CString loStrHConfFileName;
		CString loStrCppConfFileName;
		loStrHConfFileName.Format( "%s/../config/h.conf", csCurrentDir );
		loStrCppConfFileName.Format( "%s/../config/cpp.conf", csCurrentDir );
		// H文件和CPP文件生成解释上下文
		clsContext loHContext;
		clsCppContext loCppContext;
		loHContext.SetVariable( "FILENAME_BASE", loFileName.GetBaseName() );
		loHContext.LoadFormFile( loStrHConfFileName );
		loCppContext.SetVariable( "FILENAME_BASE", loFileName.GetBaseName() );
		loCppContext.LoadFormFile( loStrCppConfFileName );

		// 读取目标文件名，并创建目标文件
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
			MessageBox( ::Format("打开目标文件[%s]/[%s]失败！", loStrHFileName.c_str(), loStrCppFileName.c_str()) );
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
