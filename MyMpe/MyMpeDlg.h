
// MyMpeDlg.h : ��� ����
//

#pragma once

#include "SimpleMpe.h"

// CMyMpeDlg ��ȭ ����
class CMyMpeDlg : public CDialogEx
{
	CSimpleMpe *m_pMpe;

// �����Դϴ�.
public:
	CMyMpeDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	~CMyMpeDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYMPE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
