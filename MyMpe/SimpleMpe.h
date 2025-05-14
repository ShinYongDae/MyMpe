#pragma once
#include <thread>

#include "ymcPCAPI.h"
#pragma comment (lib, "ymcPCAPI_x64.lib")

#define PATH_MPE			_T("C:\\R2RSet\\Mpe.ini")
#define MAX_GROUPS			16
#define MAX_BYTE_SIZE		1800
#define MAX_WORD_SIZE		900
#define MAX_LONG_SIZE		450

struct stMpe
{
	unsigned short nCpu, nPort;
	int nMaxSeg, nInSeg, nOutSeg;
	CString **pAddrIn, **pAddrOut;
	CString **pSymIn, **pSymOut;
	CString **pMRegIn, **pMRegOut;
	CString **pCmtIn, **pCmtOut;
	CString **pBothIn, **pBothOut;

	int nGrpStep;
	int nGrpIn, nGrpOut;
	int nGrpInSt, nGrpOutSt;

	stMpe()
	{
		nCpu = 0; 
		nPort = 0;
		nMaxSeg = 0;
		nInSeg = 0; nOutSeg = 0;		
		pAddrIn = NULL; pAddrOut = NULL;		
		pSymIn = NULL; pSymOut = NULL;
		pMRegIn = NULL; pMRegOut = NULL;		
		pCmtIn = NULL; pCmtOut = NULL;
		pBothIn = NULL; pBothOut = NULL;
	}

	~stMpe()
	{
		if (pAddrIn)
		{
			for (int k = 0; k < nInSeg; k++)
				delete[] pAddrIn[k];
			delete[] pAddrIn;
			pAddrIn = NULL;
		}

		if (pAddrOut)
		{
			for (int k = 0; k < nOutSeg; k++)
				delete[] pAddrOut[k];
			delete[] pAddrOut;
			pAddrOut = NULL;
		}

		if (pSymIn)
		{
			for (int k = 0; k < nInSeg; k++)
				delete[] pSymIn[k];
			delete[] pSymIn;
			pSymIn = NULL;
		}

		if (pSymOut)
		{
			for (int k = 0; k < nOutSeg; k++)
				delete[] pSymOut[k];
			delete[] pSymOut;
			pSymOut = NULL;
		}

		if (pMRegIn)
		{
			for (int k = 0; k < nInSeg; k++)
				delete[] pMRegIn[k];
			delete[] pMRegIn;
			pMRegIn = NULL;
		}

		if (pMRegOut)
		{
			for (int k = 0; k < nOutSeg; k++)
				delete[] pMRegOut[k];
			delete[] pMRegOut;
			pMRegOut = NULL;
		}

		if (pCmtIn)
		{
			for (int k = 0; k < nInSeg; k++)
				delete[] pCmtIn[k];
			delete[] pCmtIn;
			pCmtIn = NULL;
		}

		if (pCmtOut)
		{
			for (int k = 0; k < nOutSeg; k++)
				delete[] pCmtOut[k];
			delete[] pCmtOut;
			pCmtOut = NULL;
		}

		if (pBothIn)
		{
			for (int k = 0; k < nInSeg; k++)
				delete[] pBothIn[k];
			delete[] pBothIn;
			pBothIn = NULL;
		}

		if (pBothOut)
		{
			for (int k = 0; k < nOutSeg; k++)
				delete[] pBothOut[k];
			delete[] pBothOut;
			pBothOut = NULL;
		}
	}
};

// CSimpleMpe

class CSimpleMpe : public CWnd
{
	DECLARE_DYNAMIC(CSimpleMpe)

	COM_DEVICE		m_ComDevice;
	HREGISTERDATA	m_hInRegData, m_hOutRegData;
	HCONTROLLER		m_hController;
	stMpe			Mpe;
	unsigned short	*m_pIO, *m_pIO_F;

	CWnd*			m_pParent;
	BOOL			m_bAliveThread, m_bEndThreadState;
	std::thread		t1;

	void StringToChar(CString str, char* szStr);
	void StringToTChar(CString str, TCHAR* tszStr);
	CString CharToString(char *szStr);
	int atoh(CString &sVal);

	void StartThread();
	void StopThread();

	BOOL LoadParam(CString sPath);
	BOOL GetIoData(int nRegAddr, unsigned short &InputData);
	BOOL SetIoData(CString sRegAddr, long OutputData);

public:
	CSimpleMpe(CWnd* pParent = NULL, CString sPathParam = PATH_MPE);
	virtual ~CSimpleMpe();

	static void funcGetGroupRegisterData(const LPVOID lpContext);
	BOOL GetGroupRegisterData();
	BOOL IsAliveThread();
	void EndThread();

	long Read(CString sRegAddr);
	BOOL Write(CString sRegAddr, long lData);

protected:
	DECLARE_MESSAGE_MAP()
};


