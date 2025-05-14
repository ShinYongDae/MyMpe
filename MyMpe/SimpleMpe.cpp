// SimpleMpe.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MyMpe.h"
#include "SimpleMpe.h"


// CSimpleMpe

IMPLEMENT_DYNAMIC(CSimpleMpe, CWnd)

CSimpleMpe::CSimpleMpe(CWnd* pParent, CString sPathParam)
{
	int i = 0;
	unsigned short nCpuNumber = 1, nPortNumber = 1;
	m_pParent = pParent;
	m_hController = 0;
	m_pIO = NULL;
	m_pIO_F = NULL;

	if(!LoadParam(sPathParam))
	{
		AfxMessageBox(_T("CSimpleMpe::LoadParam() Failed!!!"));
		return;
	}

	if (Mpe.nMaxSeg > 0)
	{
		m_pIO = new unsigned short[Mpe.nMaxSeg];
		m_pIO_F = new unsigned short[Mpe.nMaxSeg];
		for (i = 0; i < Mpe.nMaxSeg; i++)
		{
			m_pIO[i] = 0;
			m_pIO_F[i] = 0;
		}
	}

	if (!Create(NULL, _T("MPE"), WS_CHILD, CRect(0, 0, 0, 0), m_pParent, (UINT)this))
	{
		AfxMessageBox(_T("CSimpleMpe::Create() Failed!!!"));
		return;
	}

	memset(&m_ComDevice, 0x00, sizeof(COM_DEVICE));
	m_ComDevice.ComDeviceType = COMDEVICETYPE_PCI_MODE;		// PCI 버스 통신
	m_ComDevice.CpuNumber = (WORD)nCpuNumber;				// CPU 번호
	m_ComDevice.PortNumber = (WORD)nPortNumber;				// 포토 번호
	m_ComDevice.Timeout = 10000;							// 통신 프로세스의 타임 아웃 시간

	DWORD dwRtnVal = ymcOpenController(&m_ComDevice, &m_hController);
	if (dwRtnVal != MP_SUCCESS)
	{
		AfxMessageBox(_T("콘트롤러 오픈 실패."));
		return;
	}

	Sleep(10);
	StartThread();
}

CSimpleMpe::~CSimpleMpe()
{
	StopThread();
	Sleep(30);
	t1.join();

	if (m_pIO)
	{
		delete[] m_pIO;
		m_pIO = NULL;
	}

	if (m_pIO_F)
	{
		delete[] m_pIO_F;
		m_pIO_F = NULL;
	}
}


BEGIN_MESSAGE_MAP(CSimpleMpe, CWnd)
END_MESSAGE_MAP()



// CSimpleMpe 메시지 처리기입니다.

void CSimpleMpe::StartThread()
{
	m_bEndThreadState = FALSE;
	m_bAliveThread = TRUE;
	t1 = std::thread(funcGetGroupRegisterData, this);
}

void CSimpleMpe::funcGetGroupRegisterData(const LPVOID lpContext)
{
	CSimpleMpe* pSimpleMpe = reinterpret_cast<CSimpleMpe*>(lpContext);

	while (pSimpleMpe->IsAliveThread())
	{
		if (!pSimpleMpe->GetGroupRegisterData())
			break;
		Sleep(100);
	}

	pSimpleMpe->EndThread();
}

BOOL CSimpleMpe::IsAliveThread()
{
	return m_bAliveThread;
}

void CSimpleMpe::StopThread()
{
	m_bAliveThread = FALSE;
	MSG message;
	const DWORD dwTimeOut = 1000 * 60 * 3; // 3 Minute
	DWORD dwStartTick = GetTickCount();
	Sleep(30);
	while (!m_bEndThreadState)
	{	
		if (GetTickCount() >= (dwStartTick + dwTimeOut))
		{
			AfxMessageBox(_T("WaitUntilThreadEnd() Time Out!!!", NULL, MB_OK | MB_ICONSTOP));
			return;
		}
		if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
		Sleep(30);
	}
}

void CSimpleMpe::EndThread()
{
	m_bEndThreadState = TRUE;
}


BOOL CSimpleMpe::LoadParam(CString sPath)
{
	int nSeg, k;

	TCHAR sep[] = { _T(",;\r\n\t") };
	TCHAR szData[200];
	TCHAR *token1, *token2, *token3, *token4, *token5;

	CString sIdx, sInSeg, sOutSeg;
	CString sIReg, sAddr, sMReg, sSymbol, sComment, sMBoth;

	// MpeIO

	if (0 < ::GetPrivateProfileString(_T("INFO"), _T("Cpu_Number"), NULL, szData, sizeof(szData), sPath))
		Mpe.nCpu = (int)_tstoi(szData);
	else
		Mpe.nCpu = (int)0L;

	if (0 < ::GetPrivateProfileString(_T("INFO"), _T("Port_Number"), NULL, szData, sizeof(szData), sPath))
		Mpe.nPort = (int)_tstoi(szData);
	else
		Mpe.nPort = (int)0L;

	if (0 < ::GetPrivateProfileString(_T("INFO"), _T("MAX_SEGMENT"), NULL, szData, sizeof(szData), sPath))
		Mpe.nMaxSeg = (int)_tstoi(szData);
	else
		Mpe.nMaxSeg = (int)0L;

	if (0 < ::GetPrivateProfileString(_T("INFO"), _T("IN_SEGMENT"), NULL, szData, sizeof(szData), sPath))
		Mpe.nInSeg = (int)_tstoi(szData);
	else
		Mpe.nInSeg = (int)0L;

	if (0 < ::GetPrivateProfileString(_T("INFO"), _T("OUT_SEGMENT"), NULL, szData, sizeof(szData), sPath))
		Mpe.nOutSeg = (int)_tstoi(szData);
	else
		Mpe.nOutSeg = (int)0L;

	if (Mpe.nInSeg > Mpe.nOutSeg)
		nSeg = Mpe.nInSeg;
	else
		nSeg = Mpe.nOutSeg;

	Mpe.pAddrIn = new CString*[Mpe.nInSeg];
	for (k = 0; k < Mpe.nInSeg; k++)
		Mpe.pAddrIn[k] = new CString[16];

	Mpe.pAddrOut = new CString*[Mpe.nOutSeg];
	for (k = 0; k < Mpe.nOutSeg; k++)
		Mpe.pAddrOut[k] = new CString[16];

	Mpe.pSymIn = new CString*[Mpe.nInSeg];
	for (k = 0; k < Mpe.nInSeg; k++)
		Mpe.pSymIn[k] = new CString[16];

	Mpe.pSymOut = new CString*[Mpe.nOutSeg];
	for (k = 0; k < Mpe.nOutSeg; k++)
		Mpe.pSymOut[k] = new CString[16];

	Mpe.pMRegIn = new CString*[Mpe.nInSeg];
	for (k = 0; k < Mpe.nInSeg; k++)
		Mpe.pMRegIn[k] = new CString[16];

	Mpe.pMRegOut = new CString*[Mpe.nOutSeg];
	for (k = 0; k < Mpe.nOutSeg; k++)
		Mpe.pMRegOut[k] = new CString[16];

	Mpe.pCmtIn = new CString*[Mpe.nInSeg];
	for (k = 0; k < Mpe.nInSeg; k++)
		Mpe.pCmtIn[k] = new CString[16];

	Mpe.pCmtOut = new CString*[Mpe.nOutSeg];
	for (k = 0; k < Mpe.nOutSeg; k++)
		Mpe.pCmtOut[k] = new CString[16];

	Mpe.pBothIn = new CString*[Mpe.nInSeg];
	for (k = 0; k < Mpe.nInSeg; k++)
		Mpe.pBothIn[k] = new CString[16];

	Mpe.pBothOut = new CString*[Mpe.nOutSeg];
	for (k = 0; k < Mpe.nOutSeg; k++)
		Mpe.pBothOut[k] = new CString[16];

	if (0 < ::GetPrivateProfileString(_T("GROUP START SEG"), _T("GROUP_STEP"), NULL, szData, sizeof(szData), sPath))
		Mpe.nGrpStep = (int)_tstoi(szData);
	else
		Mpe.nGrpStep = 0;

	if (0 < ::GetPrivateProfileString(_T("GROUP START SEG"), _T("IN_GROUP"), NULL, szData, sizeof(szData), sPath))
		Mpe.nGrpIn = (int)_tstoi(szData);
	else
		Mpe.nGrpIn = 0;
	if (0 < ::GetPrivateProfileString(_T("GROUP START SEG"), _T("IN_START"), NULL, szData, sizeof(szData), sPath))
		Mpe.nGrpInSt = (int)_tstoi(szData);
	else
		Mpe.nGrpInSt = 0;

	if (0 < ::GetPrivateProfileString(_T("GROUP START SEG"), _T("OUT_GROUP"), NULL, szData, sizeof(szData), sPath))
		Mpe.nGrpOut = (int)_tstoi(szData);
	else
		Mpe.nGrpOut = 0;

	if (0 < ::GetPrivateProfileString(_T("GROUP START SEG"), _T("OUT_START"), NULL, szData, sizeof(szData), sPath))
		Mpe.nGrpOutSt = (int)_tstoi(szData);
	else
		Mpe.nGrpOutSt = 0;


	for (k = 0; k < nSeg; k++)
	{
		sInSeg.Format(_T("%d"), k);
		sOutSeg.Format(_T("%d"), Mpe.nInSeg + k);

		for (int nR = 0; nR < 16; nR++)
		{
			if (k < Mpe.nInSeg)
			{
				sIdx.Format(_T("%d"), nR);
				if (0 < ::GetPrivateProfileString(sInSeg, sIdx, NULL, szData, sizeof(szData), sPath))
				{
					token1 = _tcstok(szData, sep);
					token2 = _tcstok(NULL, sep);
					token3 = _tcstok(NULL, sep);
					token4 = _tcstok(NULL, sep);
					token5 = _tcstok(NULL, sep);

					sAddr = CString(token1);
					sSymbol = CString(token2);
					sMReg = CString(token3);
					sComment = CString(token4);
					sMBoth = CString(token5);
				}
				else
				{
					sAddr = _T("");
					sSymbol = _T("");
					sMReg = _T("");
					sComment = _T("");
					sMBoth = _T("");
				}
				Mpe.pAddrIn[k][nR] = sAddr;
				Mpe.pSymIn[k][nR] = sSymbol;
				Mpe.pMRegIn[k][nR] = sMReg;
				Mpe.pCmtIn[k][nR] = sMReg;
				Mpe.pBothIn[k][nR] = sMReg;
			}

			if (k < Mpe.nOutSeg)
			{
				sIdx.Format(_T("%d"), nR);
				if (0 < ::GetPrivateProfileString(sOutSeg, sIdx, NULL, szData, sizeof(szData), sPath))
				{
					token1 = _tcstok(szData, sep);
					token2 = _tcstok(NULL, sep);
					token3 = _tcstok(NULL, sep);
					token4 = _tcstok(NULL, sep);
					token5 = _tcstok(NULL,sep);

					sAddr = CString(token1);
					sSymbol = CString(token2);
					sMReg = CString(token3);
					sComment = CString(token4);
					sMBoth = CString(token5);
				}
				else
				{
					sAddr = _T("");
					sSymbol = _T("");
					sMReg = _T("");
					sComment = _T("");
					sMBoth = _T("");
				}
				Mpe.pAddrOut[k][nR] = sAddr;
				Mpe.pSymOut[k][nR] = sSymbol;
				Mpe.pMRegOut[k][nR] = sMReg;
				Mpe.pCmtOut[k][nR] = sMReg;
				Mpe.pBothOut[k][nR] = sMReg;
			}
		}
	}

	return TRUE;
}

BOOL CSimpleMpe::GetGroupRegisterData() // GroupNuber is up to 16 groups. ( One group can read data from the registers that total size is 1,800 bytes. : LONG(450), WORD(900) )
{
	DWORD				rc;											// Motion API return value
	REGISTER_INFO		RegInfo[MAX_GROUPS];						// Stores the discontinuous register information
	HREGISTERDATA		hRegisterData[MAX_GROUPS];					// Stores the register data handle
	WORD				pRegisterWData[MAX_GROUPS][MAX_WORD_SIZE];	// Buffer of 2*900 bytes
	DWORD				pRegisterLData[MAX_GROUPS][MAX_LONG_SIZE];	// Buffer of  4*450 bytes
	DWORD				GroupNumber;								// Setting Register Information Number 

	// MpeIO
	int nSize, nIdx, nLoop, nSt, nBit;
	int nInSeg = Mpe.nInSeg;
	int nOutSeg = Mpe.nOutSeg;
	int nGrpStep = Mpe.nGrpStep;						// Numbers of WORD	(Max is 900)
	int nGrpIn = Mpe.nGrpIn;
	int nGrpOut = Mpe.nGrpOut;

	if (nGrpIn + nGrpOut > MAX_GROUPS)
	{
		AfxMessageBox(_T("Error - ( IN_GROUP + OUT_GROUP ) > 16"));
		return FALSE;
	}

	CString sAddr;
	int nLen = 0;
	char* cAddr = NULL;
	char cType;

	// Input IO
	nSt = 0;
	for (nLoop = 0; nLoop < nGrpIn; nLoop++)
	{
		sAddr = Mpe.pAddrIn[nSt][0];
		nLen = sAddr.GetLength() + 1; // for '\0'
		cAddr = new char[nLen];
		cType = sAddr.GetAt(0);
		sAddr.SetAt(1, 'W');

		if (cType == 'M' || cType == 'm')
			StringToChar(sAddr.Left(7), cAddr);
		else
			StringToChar(sAddr.Left(6), cAddr);

		GroupNumber = nLoop;
		// Gets the regiter data handle of group (1 ~ nGrpIn)
		rc = ymcGetRegisterDataHandle((LPBYTE)cAddr, &hRegisterData[GroupNumber]);
		delete cAddr;

		RegInfo[GroupNumber].hRegisterData = hRegisterData[GroupNumber];					// Register handle
		RegInfo[GroupNumber].RegisterDataNumber = nGrpStep;							// The number of register data
		RegInfo[GroupNumber].pRegisterData = pRegisterWData[GroupNumber];					// The number of register data
		nSt += nGrpStep;
	}

	// Output IO
	nSt = 0;
	for (nLoop = 0; nLoop < nGrpOut; nLoop++)
	{
		sAddr = Mpe.pAddrOut[nSt][0];
		nLen = sAddr.GetLength() + 1; // for '\0'
		cAddr = new char[nLen];
		cType = sAddr.GetAt(0);
		sAddr.SetAt(1, 'W');

		if (cType == 'M' || cType == 'm')
			StringToChar(sAddr.Left(7), cAddr);
		else
			StringToChar(sAddr.Left(6), cAddr);

		GroupNumber = nGrpIn + nLoop;
		// Gets the regiter data handle of group (nGrpIn ~ nGrpOut)
		rc = ymcGetRegisterDataHandle((LPBYTE)cAddr, &hRegisterData[GroupNumber]);
		delete cAddr;

		RegInfo[GroupNumber].hRegisterData = hRegisterData[GroupNumber];	// Register handle
		RegInfo[GroupNumber].RegisterDataNumber = nGrpStep;					// The number of register data
		RegInfo[GroupNumber].pRegisterData = pRegisterWData[GroupNumber];	// The number of register data
		nSt += nGrpStep;
	}

	// Sets the register data for (nGrpIn + nGrpOut) groups of the discontinuous register information
	GroupNumber = nGrpIn + nGrpOut;											// Setting Register Information Number

	rc = ymcGetGroupRegisterData(GroupNumber, &RegInfo[0]);
	// Error check processing
	if (rc != MP_SUCCESS)
	{
		AfxMessageBox(_T("ymcGetGroupRegisterData ERROR"));
		return FALSE;
	}

	WORD RegWData;
	nSt = 0;
	for (nLoop = 0; nLoop < GroupNumber; nLoop++)
	{
		for (nIdx = 0; nIdx < nGrpStep; nIdx++)
		{
			RegWData = *((WORD*)RegInfo[nLoop].pRegisterData + nIdx);
			m_pIO[nIdx + nSt] = (unsigned short)RegWData;
		}
		nSt += nGrpStep;
	}

	return TRUE;
}

long CSimpleMpe::Read(CString sRegAddr)	// M-Register
{
	long lData = 0L;

	unsigned short wReadBuffer = 0, wReadBuffer2 = 0, nAddrLastNum = 0;
	long lReadBuffer = 0;
	CString strData;
	int nthBit;
	BOOL bOdd;
	char chDataType = sRegAddr.GetAt(0);
	if (!(chDataType == 'M' || chDataType == 'm'))
		return 0;
	chDataType = sRegAddr.GetAt(1);
	switch (chDataType)
	{
	case 'B':
	case 'b':		
		strData = sRegAddr.Right(1); // Make Bit mask
		nthBit = atoh(strData);
		wReadBuffer2 = (0x01 << nthBit);		
		sRegAddr.Delete(sRegAddr.GetLength() - 1); // Erase Bit data
		sRegAddr.Right(sRegAddr.GetLength() - 2); // Erase First & Second character
		if (!GetIoData(_tstoi(sRegAddr), wReadBuffer))
		{
			AfxMessageBox(_T("Mp2100m - Reading Error!!!"));
			return 0;
		}
		if (wReadBuffer & wReadBuffer2)
			lData = 1;
		break;
	case 'W':
	case 'w':
		sRegAddr.Right(sRegAddr.GetLength() - 2); // Erase First & Second character
		if (!GetIoData(_tstoi(sRegAddr), wReadBuffer))
		{
			AfxMessageBox(_T("Mp2100m - Reading Error!!!"));
			return 0;
		}
		lData = (long)wReadBuffer;
		break;
	case 'L':
	case 'l':
		sRegAddr.Right(sRegAddr.GetLength() - 2); // Erase First & Second character
		if (!GetIoData(_tstoi(sRegAddr), wReadBuffer))
		{
			AfxMessageBox(_T("Mp2100m - Reading Error!!!"));
			return 0;
		}
		if (!GetIoData(_tstoi(sRegAddr)+1, wReadBuffer2))
		{
			AfxMessageBox(_T("Mp2100m - Reading Error!!!"));
			return 0;
		}
		lReadBuffer = wReadBuffer2;
		lReadBuffer = (lReadBuffer << 16) & 0xFFFF0000;
		lReadBuffer = lReadBuffer | wReadBuffer;
		lData = lReadBuffer;
		break;
	case 'F':
	case 'f':
		lData = 0;
		break;
	}

	return lData;
}

BOOL CSimpleMpe::GetIoData(int nRegAddr, unsigned short &InputData)
{
	int nSeg, k;

	if (Mpe.nInSeg > Mpe.nOutSeg)
		nSeg = Mpe.nInSeg;
	else
		nSeg = Mpe.nOutSeg;

	CString sIdx, sInSeg, sOutSeg;
	CString sMRegIn, sMRegOut;

	for (k = 0; k < nSeg; k++)
	{
		sInSeg.Format(_T("%d"), k);
		sOutSeg.Format(_T("%d"), Mpe.nInSeg + k);
		if (k < Mpe.nInSeg)
		{
			sMRegIn = Mpe.pMRegIn[k][0];
			sMRegIn.Delete(sMRegIn.GetLength() - 1); // Erase Bit data
			sMRegIn.Right(sMRegIn.GetLength() - 2); // Erase First & Second character
			if (_tstoi(sMRegIn) == nRegAddr)
			{
				InputData = m_pIO[k];
				return TRUE;
			}
		}

		if (k < Mpe.nOutSeg)
		{
			sMRegOut = Mpe.pMRegOut[k][0];
			sMRegOut.Delete(sMRegOut.GetLength() - 1); // Erase Bit data
			sMRegOut.Right(sMRegOut.GetLength() - 2); // Erase First & Second character
			if (_tstoi(sMRegOut) == nRegAddr)
			{
				InputData = m_pIO[Mpe.nInSeg + k];
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CSimpleMpe::Write(CString sRegAddr, long lData)	// M-Register
{
	if (sRegAddr.IsEmpty())
	{
		return FALSE;
	}

	CString sAddr;
	long lWriteBuffer, lReadBuffer;
	CString sData;
	int nthBit, nAddrLastNum;
	BOOL bOdd;
	sAddr = sRegAddr;
	char chDataType = sAddr.GetAt(1);

	switch (chDataType)
	{
	case 'B':
	case 'b':
		sData = sAddr.Right(1);					// Make Bit mask
		nthBit = atoh(sData);
		sAddr.Delete(sAddr.GetLength() - 1);	// Erase Bit data
		sData = sAddr.Right(1);					// Adjust Address
		nAddrLastNum = _tstoi(sData);
		bOdd = nAddrLastNum % 2;
		if (bOdd)
		{
			int nDecrese = nAddrLastNum - 1;
			sAddr.SetAt(sAddr.GetLength() - 1, '0' + nDecrese);
			sAddr.SetAt(1, 'L');
			lReadBuffer = Read(sAddr);
			if (lData)
				lWriteBuffer = lReadBuffer | (0x01 << (16 + nthBit));
			else
				lWriteBuffer = lReadBuffer & (~(0x01 << (16 + nthBit)));
		}
		else
		{
			sAddr.SetAt(1, 'L');
			lReadBuffer = Read(sAddr);
			if (lData)
				lWriteBuffer = lReadBuffer | (0x01 << (nthBit));
			else
				lWriteBuffer = lReadBuffer & (~(0x01 << (nthBit)));
		}

		if (!SetIoData(sAddr, lWriteBuffer))
		{
			AfxMessageBox(_T("Mp2100m - Writing Error!!!"));
			return FALSE;
		}
		break;
	case 'W':
	case 'w':
		sData = sAddr.Right(1);
		nAddrLastNum = _tstoi(sData);
		bOdd = nAddrLastNum % 2;
		if (bOdd)
		{
			sAddr.SetAt(sAddr.GetLength() - 1, ('0' + nAddrLastNum - 1));
			sAddr.SetAt(1, 'L');
			lReadBuffer = Read(sAddr);
			lData = (lData << 16);
			lWriteBuffer = (lReadBuffer & 0x0000FFFF) | lData;
		}
		else
		{
			sAddr.SetAt(1, 'L');
			lReadBuffer = Read(sAddr);
			lWriteBuffer = (lReadBuffer & 0xFFFF0000) | (lData & 0x0000FFFF);
		}

		if (!SetIoData(sAddr, lWriteBuffer))
		{
			AfxMessageBox(_T("Mp2100m - Writing Error!!!"));
			return FALSE;
		}

		break;
	case 'L':
	case 'l':
		sData = sAddr.Right(1);
		nAddrLastNum = _tstoi(sData);
		bOdd = nAddrLastNum % 2;
		if (bOdd)
		{
			sAddr.SetAt(sAddr.GetLength() - 1, ('0' + nAddrLastNum - 1));
			lReadBuffer = Read(sAddr);
			lWriteBuffer = (lReadBuffer & 0x0000FFFF) | (lData << 16);
			if (!SetIoData(sAddr, lWriteBuffer))							// 하위 DWORD를 출력
			{
				AfxMessageBox(_T("Mp2100m - Writing Error!!!"));
				return FALSE;
			}

			sAddr.SetAt(sAddr.GetLength() - 1, ('0' + nAddrLastNum + 1));
			lReadBuffer = Read(sAddr);
			lWriteBuffer = (lReadBuffer & 0xFFFF0000) | (lData >> 16);
			if (!SetIoData(sAddr, lWriteBuffer))							// 상위 DWORD를 출력
			{
				AfxMessageBox(_T("Mp2100m - Writing Error!!!"));
				return FALSE;
			}
		}
		else
		{
			lWriteBuffer = lData;
			if (!SetIoData(sAddr, lWriteBuffer))
			{
				AfxMessageBox(_T("Mp2100m - Writing Error!!!"));
				return FALSE;
			}
		}
		break;
	case 'F':
	case 'f':
		lWriteBuffer = lData;
		if (!SetIoData(sAddr, lWriteBuffer))
		{
			AfxMessageBox(_T("Mp2100m - Writing Error!!!"));
			return FALSE;
		}

		break;
	}

	return TRUE;
}

BOOL CSimpleMpe::SetIoData(CString sRegAddr, long OutputData)
{
	char* cRegAddr;
	int nLen = sRegAddr.GetLength() + 1; // for '\0'
	cRegAddr = new char[nLen];
	StringToChar(sRegAddr, cRegAddr);

	DWORD dwReturnVal = ymcSetController(m_hController);
	if (dwReturnVal != MP_SUCCESS)
	{
		AfxMessageBox(_T("Controller 핸들을 취득할 수 없습니다."));
		return FALSE;
	}

	dwReturnVal = ymcGetRegisterDataHandle((LPBYTE)cRegAddr, &m_hOutRegData);
	delete cRegAddr;

	if (dwReturnVal != MP_SUCCESS)
	{
		AfxMessageBox(_T("출력 레지스터 핸들을 취득할 수 없습니다."));
		return FALSE;
	}

	dwReturnVal = ymcSetRegisterData(m_hOutRegData, 1, &OutputData);
	if (dwReturnVal != MP_SUCCESS)
	{
		AfxMessageBox(_T("출력 데이터 설정 실패."));
		return FALSE;
	}

	return TRUE;
}

void CSimpleMpe::StringToChar(CString str, char* szStr)  // char* returned must be deleted... 
{
	int nLen = str.GetLength();
	strcpy(szStr, CT2A(str));
	szStr[nLen] = _T('\0');
}

void CSimpleMpe::StringToTChar(CString str, TCHAR* tszStr) // TCHAR* returned must be deleted... 
{
	int nLen = str.GetLength() + 1;
	memset(tszStr, 0x00, nLen * sizeof(TCHAR));
	_tcscpy(tszStr, str);
}

CString CSimpleMpe::CharToString(char *szStr)
{
	CString strRet;

	int nLen = strlen(szStr) + sizeof(char);
	wchar_t *tszTemp = NULL;
	tszTemp = new WCHAR[nLen];

	MultiByteToWideChar(CP_ACP, 0, szStr, -1, tszTemp, nLen * sizeof(WCHAR));

	strRet.Format(_T("%s"), (CString)tszTemp);
	if (tszTemp)
	{
		delete[] tszTemp;
		tszTemp = NULL;
	}
	return strRet;
}

int CSimpleMpe::atoh(CString &sVal)
{
	int nHexVal = 0;
	int nLen = sVal.GetLength();
	CString strChar, strPas;
	if (nLen > 0)
	{
		for (int i = 1; i <= nLen; i++)
		{
			strPas = sVal.Left(i);
			int nLenPars = strPas.GetLength();
			if (nLenPars > 1)
				strPas.Delete(0, nLenPars - 1);
			strChar = strPas;

			strChar.MakeUpper();
			if (strChar == _T("A"))	nHexVal += int(10.0 * pow(16, (nLen - i)));
			else if (strChar == _T("B"))	nHexVal += int(11.0 * pow(16, (nLen - i)));
			else if (strChar == _T("C"))	nHexVal += int(12.0 * pow(16, (nLen - i)));
			else if (strChar == _T("D"))	nHexVal += int(13.0 * pow(16, (nLen - i)));
			else if (strChar == _T("E"))	nHexVal += int(14.0 * pow(16, (nLen - i)));
			else if (strChar == _T("F"))	nHexVal += int(15.0 * pow(16, (nLen - i)));
			else
				nHexVal += int(_tstof(strChar) * pow(16, (nLen - i)));
		}
	}

	return nHexVal;
}
