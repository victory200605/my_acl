// MidCoderGenerator.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CMidCoderGeneratorApp:
// �йش����ʵ�֣������ MidCoderGenerator.cpp
//

class CMidCoderGeneratorApp : public CWinApp
{
public:
	CMidCoderGeneratorApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMidCoderGeneratorApp theApp;
