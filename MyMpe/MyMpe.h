
// MyMpe.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CMyMpeApp:
// �� Ŭ������ ������ ���ؼ��� MyMpe.cpp�� �����Ͻʽÿ�.
//

class CMyMpeApp : public CWinApp
{
public:
	CMyMpeApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CMyMpeApp theApp;