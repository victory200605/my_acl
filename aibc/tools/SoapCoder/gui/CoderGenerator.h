// CoderGenerator.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CCoderGeneratorApp:
// �йش����ʵ�֣������ CoderGenerator.cpp
//

class CCoderGeneratorApp : public CWinApp
{
public:
	CCoderGeneratorApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCoderGeneratorApp theApp;
