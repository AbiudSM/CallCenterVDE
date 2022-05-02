#include "stdafx.h"
#include "D_LineDevCaps.h"
#include "D_LineAddressCaps.h"
#include "CallMonitor3.h"
#include "CallMonitorDoc.h"
#include "CallData.h"
#include "InformDlg.h"
#include "ProgDlg.h"

#include <iostream>
#include <fstream> 
#include <string>
#include<windows.h>
#include <sys/stat.h>

using namespace std;



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CCallData::CCallData()
{
	m_init = FALSE;
	m_hLineApp=NULL;
	m_APIVersionHigh = 0x00020000;
	m_APIVersionLow  = 0x00020000;
	m_EXTVersion = 0x00010002;

	m_pExtTree = NULL;
	m_pCOTree = NULL;
	m_pEtcTree = NULL;
	m_hExtTreeRoot = NULL;
	m_hCOTreeRoot = NULL;
	m_hEtcTreeRoot = NULL;
	m_hMCCall = NULL;
	m_hMCLine = NULL;
	m_RequestID_MakeCall = 0;
	m_RequestID_Answer = 0;
	m_RequestID_Drop = 0;

	m_LogFileName = _T("");

	m_ExtLineDevCapsList.clear();
	m_COLineDevCapsList.clear();
	m_EtcLineDevCapsList.clear();
	m_LineAddressCapsList.clear();
	
	m_COMapLine2Data.clear();
	m_ExtMapLine2Data.clear();
	m_EtcMapLine2Data.clear();
	m_COMapTree2Data.clear();
	m_ExtMapTree2Data.clear();
	m_EtcMapTree2Data.clear();

	m_EtcMapCall2Tree.clear();
	m_ExtMapCall2Tree.clear();
	m_COMapCall2Tree.clear();

}

CCallData::~CCallData()
{
	Close();
}

void CCallData::Close()
{
// Ext
	int dsize = m_ExtLineDevCapsList.size();
	if (dsize) {
		for (int i=0;i<dsize;i++) {
			CD_LineDevCaps* pDevCaps = m_ExtLineDevCapsList[i];
			if (pDevCaps != NULL) {
				HLINE hLine = pDevCaps->GethLine();
				if (hLine!=NULL) {
					lineClose(hLine);
				}
				delete pDevCaps;
			}
		}
		m_ExtLineDevCapsList.clear();
	}
// CO
	dsize = m_COLineDevCapsList.size();
	if (dsize) {
		for (int i=0;i<dsize;i++) {
			CD_LineDevCaps* pDevCaps = m_COLineDevCapsList[i];
			if (pDevCaps != NULL) {
				HLINE hLine = pDevCaps->GethLine();
				if (hLine!=NULL) {
					lineClose(hLine);
				}
				delete pDevCaps;
			}
		}
		m_COLineDevCapsList.clear();
	}
// Etc
	dsize = m_EtcLineDevCapsList.size();
	if (dsize) {
		for (int i=0;i<dsize;i++) {
			CD_LineDevCaps* pDevCaps = m_EtcLineDevCapsList[i];
			if (pDevCaps != NULL) {
				HLINE hLine = pDevCaps->GethLine();
				if (hLine!=NULL) {
					lineClose(hLine);
				}
				delete pDevCaps;
			}
		}
		m_EtcLineDevCapsList.clear();
	}

	dsize = m_LineAddressCapsList.size();
	if (dsize) {
		for (int i=0;i<dsize;i++) {
			delete m_LineAddressCapsList[i];
		}
		m_LineAddressCapsList.clear();
	}

// Shutdown
	if (m_init) {
		lineShutdown(m_hLineApp);
		m_init = FALSE;
	}
}

void CCallData::ExtDestroy()
{
	m_ExtMapCall2Tree.clear();
}

void CCallData::CODestroy()
{
	m_COMapCall2Tree.clear();
}

void CCallData::EtcDestroy()
{
	m_EtcMapCall2Tree.clear();
}

void CALLBACK CCallData::Callback(DWORD hDevice,DWORD dwMsg,DWORD dwCBInst,DWORD dwParam1,  DWORD dwParam2,DWORD dwParam3)
{
	CCallMonitorApp* pApp = (CCallMonitorApp*)AfxGetApp();
	if (pApp == NULL) return;
	if (pApp->m_progress) return;

	CFrameWnd* pMWnd = (CFrameWnd*)AfxGetMainWnd();
	if (pMWnd == NULL) return;
	CCallMonitorDoc* pDoc= (CCallMonitorDoc*)pMWnd->GetActiveDocument();
	if (pDoc == NULL) return;
	CCallData* pThis = pDoc->m_calldata;
	if (pThis == NULL) return;

	switch (dwMsg) {
	case LINE_ADDRESSSTATE:
		break;
	case LINE_CALLINFO:
		pThis->OnLineCallinfo(hDevice,dwCBInst,dwParam1,dwParam2,dwParam3);
		break;
	case LINE_CALLSTATE:
		pThis->OnLineCallstate(hDevice,dwCBInst,dwParam1,dwParam2,dwParam3);
		break;
	case LINE_CLOSE:
		break;
	case LINE_DEVSPECIFIC:
		break;
	case LINE_DEVSPECIFICFEATURE:
		break;
	case LINE_GATHERDIGITS:
		break;
	case LINE_GENERATE:
		break;
	case LINE_LINEDEVSTATE:
		pThis->OnLineLinedevstate(hDevice,dwCBInst,dwParam1,dwParam2,dwParam3);
		break;
	case LINE_MONITORDIGITS:
		break;
	case LINE_MONITORMEDIA:
		break;
	case LINE_MONITORTONE:
		break;
	case LINE_REPLY:
		pThis->OnLineReply(hDevice,dwCBInst,dwParam1,dwParam2,dwParam3);
		break;
	case LINE_REQUEST:
		break;
	case PHONE_BUTTON:
		break;
	case PHONE_CLOSE:
		break;
	case PHONE_DEVSPECIFIC:
		break;
	case PHONE_REPLY:
		break;
	case PHONE_STATE:
		break;
	case LINE_CREATE:
		break;
	case PHONE_CREATE:
		break;
	case LINE_AGENTSPECIFIC:
		break;
	case LINE_AGENTSTATUS:
		break;
	case LINE_APPNEWCALL:
		pThis->OnLineAppnewcall(hDevice,dwCBInst,dwParam1,dwParam2,dwParam3);
		break;
	case LINE_PROXYREQUEST:
		break;
	case LINE_REMOVE:
		break;
	case PHONE_REMOVE:
		break;
	default:
		break;
	}
	pDoc->UpdateAllViews( NULL );
}

void CCallData::OnLineReply(DWORD hDevice,DWORD dwCBInst,DWORD dwParam1,  DWORD dwParam2,DWORD dwParam3)
{
	DWORD idRequest=dwParam1;
	DWORD Status=dwParam2;
	HCALL hCall=(HCALL)dwParam3;
	if (idRequest==(DWORD)m_RequestID_MakeCall) {
		if (!Status) {
			OnLineAppnewcall((DWORD)m_hMCLine,0,0,(DWORD)m_hMCCall,0);
		}
		m_hMCCall=NULL;
		m_hMCLine=NULL;
		m_RequestID_MakeCall=0;
	} else if (idRequest==(DWORD)m_RequestID_Answer) {
		m_RequestID_Answer=0;
	} else if (idRequest==(DWORD)m_RequestID_Drop) {
		m_RequestID_Drop=0;
	}

}

void CCallData::OnLineAppnewcall(DWORD hDevice,DWORD dwCBInst,DWORD dwParam1,  DWORD dwParam2,DWORD dwParam3)
{
	HLINE	hLine=(HLINE)hDevice;
	DWORD	dwAddressID = dwParam1;
	HCALL	hCall=(HCALL)dwParam2;
	DWORD	dwPrivilege = dwParam3;

	map <HLINE,CD_LineDevCaps*>::iterator it1;
	it1 = m_ExtMapLine2Data.find(hLine);
	if (it1!=m_ExtMapLine2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		HTREEITEM hTree = CreateCall(hCall,pDevCaps,m_pExtTree);
		m_ExtMapCall2Tree[hCall] = hTree;
	}

	it1 = m_COMapLine2Data.find(hLine);
	if (it1!=m_COMapLine2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		HTREEITEM hTree = CreateCall(hCall,pDevCaps,m_pCOTree);
		m_COMapCall2Tree[hCall] = hTree;
	}

	it1 = m_EtcMapLine2Data.find(hLine);
	if (it1!=m_EtcMapLine2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		HTREEITEM hTree = CreateCall(hCall,pDevCaps,m_pEtcTree);
		m_EtcMapCall2Tree[hCall] = hTree;
	}
}

HTREEITEM CCallData::CreateCall(HCALL hCall,CD_LineDevCaps* pDevCaps,CTreeCtrl* pTree)
{
	HTREEITEM hPTree = pDevCaps->GethTree();
	HTREEITEM hTree = pTree->InsertItem(_T("NEW"),hPTree);
	pTree->SetItemData(hTree,(DWORD)hCall);
	pTree->Expand(hPTree,TVE_EXPAND);
	pTree->EnsureVisible(hTree);
	SetInfoData(hCall,m_pExtTree,hTree);
	return hTree;
}

void CCallData::OnLineLinedevstate(DWORD hDevice,DWORD dwCBInst,DWORD dwParam1,  DWORD dwParam2,DWORD dwParam3)
{
	HLINE	hLine=(HLINE)hDevice;
	DWORD	DeviceState = dwParam1;
	DWORD	DeviceStateDetail1 = dwParam2;
	DWORD	DeviceStateDetail2 = dwParam3;
	CString mes;

	if (DeviceState==LINEDEVSTATE_OUTOFSERVICE) {
		mes = _T("OUTOFSERVICE");
		WriteLogString(mes);

		map <HLINE,CD_LineDevCaps*>::iterator it1;
		it1 = m_ExtMapLine2Data.find(hLine);
		if (it1!=m_ExtMapLine2Data.end()) {
			CD_LineDevCaps* pDevCaps= (*it1).second;
			HTREEITEM hPTree = pDevCaps->GethTree();
			HTREEITEM hCTree = m_pExtTree->GetChildItem(hPTree);
			while (hCTree) {
				HCALL hCall = (HCALL)m_pExtTree->GetItemData(hCTree);
				if (hCall!=NULL) {
					lineDeallocateCall(hCall);
					map <HCALL,HTREEITEM>::iterator it2;
					it2 = m_ExtMapCall2Tree.find(hCall);
					if (it2!=m_ExtMapCall2Tree.end()) {
						m_ExtMapCall2Tree.erase(it2);
					}
				}
				HTREEITEM hCTreeBak = hCTree;
				hCTree = m_pExtTree->GetNextSiblingItem(hCTree);
				m_pExtTree->DeleteItem(hCTreeBak);
			}
		}
		it1 = m_COMapLine2Data.find(hLine);
		if (it1!=m_COMapLine2Data.end()) {
			CD_LineDevCaps* pDevCaps= (*it1).second;
			HTREEITEM hPTree = pDevCaps->GethTree();
			HTREEITEM hCTree = m_pCOTree->GetChildItem(hPTree);
			while (hCTree) {
				HCALL hCall = (HCALL)m_pCOTree->GetItemData(hCTree);
				if (hCall!=NULL) {
					lineDeallocateCall(hCall);
					map <HCALL,HTREEITEM>::iterator it2;
					it2 = m_COMapCall2Tree.find(hCall);
					if (it2!=m_COMapCall2Tree.end()) {
						m_COMapCall2Tree.erase(it2);
					}
				}
				HTREEITEM hCTreeBak = hCTree;
				hCTree = m_pCOTree->GetNextSiblingItem(hCTree);
				m_pCOTree->DeleteItem(hCTreeBak);
			}
		}

		it1 = m_EtcMapLine2Data.find(hLine);
		if (it1!=m_EtcMapLine2Data.end()) {
			CD_LineDevCaps* pDevCaps= (*it1).second;
			HTREEITEM hPTree = pDevCaps->GethTree();
			HTREEITEM hCTree = m_pEtcTree->GetChildItem(hPTree);
			while (hCTree) {
				HCALL hCall = (HCALL)m_pEtcTree->GetItemData(hCTree);
				if (hCall!=NULL) {
					lineDeallocateCall(hCall);
					map <HCALL,HTREEITEM>::iterator it2;
					it2 = m_EtcMapCall2Tree.find(hCall);
					if (it2!=m_EtcMapCall2Tree.end()) {
						m_EtcMapCall2Tree.erase(it2);
					}
				}
				HTREEITEM hCTreeBak = hCTree;
				hCTree = m_pEtcTree->GetNextSiblingItem(hCTree);
				m_pEtcTree->DeleteItem(hCTreeBak);
			}
		}
	} else if (DeviceState==LINEDEVSTATE_REINIT) {
		mes = _T("REINIT");
		WriteLogString(mes);
		Close();
		//if (AfxMessageBox(_T("TAPI Reinit.\nRetry?"),MB_YESNO)==IDYES) {
			AfxGetMainWnd()->PostMessage(WM_COMMAND,ID_FILE_NEW);
		//} else {
			//AfxGetMainWnd()->PostMessage(WM_COMMAND,ID_FILE_CLOSE);
		//}
	}

}

void CCallData::OnLineCallstate(DWORD hDevice,DWORD dwCBInst,DWORD dwParam1,  DWORD dwParam2,DWORD dwParam3)
{
	HCALL	hCall=(HCALL)hDevice;

	CString mes=CD_Base::GetCallState(dwParam1);

	CString mes2=CD_Base::GetCallStateMode(dwParam1,dwParam2);
	if (mes2.GetLength()) {
		mes += _T(" - ") + mes2;
	}

	switch (dwParam3) {
		case LINECALLPRIVILEGE_MONITOR:
			mes += _T(" - (NONITOR)");
			break;
		case LINECALLPRIVILEGE_OWNER:
			mes += _T(" - (OWNER)");
			break;
	}
	
	
	map <HCALL,HTREEITEM>::iterator it1;
	it1 = m_ExtMapCall2Tree.find(hCall);
	if (it1!=m_ExtMapCall2Tree.end()) {
		HTREEITEM hTree= (*it1).second;
		WriteLogStateData(m_pExtTree,hTree,mes, hCall);
		if (dwParam1==LINECALLSTATE_IDLE) {
			lineDeallocateCall(hCall);
			m_pExtTree->DeleteItem(hTree);
			m_ExtMapCall2Tree.erase(it1);
		} else {
			m_pExtTree->SetItemText(hTree,mes);
			HTREEITEM hPTree = m_pExtTree->GetParentItem(hTree);
			m_pExtTree->EnsureVisible(hPTree);
			SetInfoData(hCall,m_pExtTree,hTree);

		}
	}
	it1 = m_COMapCall2Tree.find(hCall);
	if (it1!=m_COMapCall2Tree.end()) {
		HTREEITEM hTree= (*it1).second;
		WriteLogStateData(m_pCOTree,hTree,mes, hCall);
		if (dwParam1==LINECALLSTATE_IDLE) {
			lineDeallocateCall(hCall);
			m_pCOTree->DeleteItem(hTree);
			m_COMapCall2Tree.erase(it1);
		} else {
			m_pCOTree->SetItemText(hTree,mes);
			HTREEITEM hPTree = m_pCOTree->GetParentItem(hTree);
			m_pCOTree->EnsureVisible(hPTree);
			SetInfoData(hCall,m_pCOTree,hTree);
		}
	}

	it1 = m_EtcMapCall2Tree.find(hCall);
	if (it1!=m_EtcMapCall2Tree.end()) {
		HTREEITEM hTree= (*it1).second;
		WriteLogStateData(m_pEtcTree,hTree,mes, hCall);
		if (dwParam1==LINECALLSTATE_IDLE) {
			lineDeallocateCall(hCall);
			m_pEtcTree->DeleteItem(hTree);
			m_EtcMapCall2Tree.erase(it1);
		} else {
			m_pEtcTree->SetItemText(hTree,mes);
			HTREEITEM hPTree = m_pEtcTree->GetParentItem(hTree);
			m_pEtcTree->EnsureVisible(hPTree);
			SetInfoData(hCall,m_pEtcTree,hTree);
		}
	}

	TRACE1("%s\n",mes);

}

void CCallData::WriteLogStateData(CTreeCtrl* pTree,HTREEITEM hTree,LPCTSTR strMes, HCALL hCall)
{
	CString strLog;
	CString strTemp;
	HTREEITEM hPTree = pTree->GetParentItem(hTree);
	strLog = pTree->GetItemText(hPTree)+SEPARATE_ST;
	strLog += _T("LINE_CALLSTATE");
	strLog += SEPARATE_ST;
	strTemp.Format( "hCall=%X(%d)", (DWORD)hCall, (DWORD)hCall ); 
	strTemp += SEPARATE_ST;
	strLog += strTemp;
	strLog += strMes;
	WriteLogString(strLog);
}

#define SERVICEID_INTEG_TAPI 1
int lineNumber_ = 1;
char *providerType_ = "Panasonic";

void CCallData::OnLineCallinfo(DWORD hDevice,DWORD dwCBInst,DWORD dwParam1,  DWORD dwParam2,DWORD dwParam3)
{
	HCALL	hCall=(HCALL)hDevice;
	CString mes = CD_Base::GetCallInfoState(dwParam1);

	if ( ((dwParam1 & LINECALLINFOSTATE_CALLERID) == LINECALLINFOSTATE_CALLERID) ||
		 ((dwParam1 & LINECALLINFOSTATE_CALLEDID) == LINECALLINFOSTATE_CALLEDID) ||
		 ((dwParam1 & LINECALLINFOSTATE_CONNECTEDID) == LINECALLINFOSTATE_CONNECTEDID) ||
		 ((dwParam1 & LINECALLINFOSTATE_TRUNK) == LINECALLINFOSTATE_TRUNK) ||
		 ((dwParam1 & LINECALLINFOSTATE_ORIGIN) == LINECALLINFOSTATE_ORIGIN) ||
		 ((dwParam1 & LINECALLINFOSTATE_REASON) == LINECALLINFOSTATE_REASON) ||
		 ((dwParam1 & LINECALLINFOSTATE_REDIRECTIONID) == LINECALLINFOSTATE_REDIRECTIONID) ||
		 ((dwParam1 & LINECALLINFOSTATE_REDIRECTINGID) == LINECALLINFOSTATE_REDIRECTINGID) ) {
		map <HCALL,HTREEITEM>::iterator it1;
		it1 = m_ExtMapCall2Tree.find(hCall);
		if (it1!=m_ExtMapCall2Tree.end()) {
			HTREEITEM hTree= (*it1).second;
			SetInfoData(hCall,m_pExtTree,hTree);
			WriteLogInfoData(hCall,m_pExtTree,hTree,mes);
		}
		it1 = m_COMapCall2Tree.find(hCall);
		if (it1!=m_COMapCall2Tree.end()) {
			HTREEITEM hTree= (*it1).second;
			SetInfoData(hCall,m_pCOTree,hTree);
			WriteLogInfoData(hCall,m_pCOTree,hTree,mes);
		}

		it1 = m_EtcMapCall2Tree.find(hCall);
		if (it1!=m_EtcMapCall2Tree.end()) {
			HTREEITEM hTree= (*it1).second;
			SetInfoData(hCall,m_pEtcTree,hTree);
			WriteLogInfoData(hCall,m_pEtcTree,hTree,mes);
		}
	}
	
	TRACE1("%s\n",mes);
}

bool is_number(const std::string& s){
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

void CCallData::SetInfoData(HCALL hCall,CTreeCtrl* pTree,HTREEITEM hTree)
{
	CD_LineCallInfo* pCInfo = GetCallInfo(hCall);
	if (pCInfo==NULL) return;
	CString strID;
	CString strTime = GetTimeString();
	
	if (pTree->ItemHasChildren(hTree)) {
		HTREEITEM hCTree = pTree->GetChildItem(hTree);

		strID = pTree->GetItemText(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("Caller ID    = ");
		strID += pCInfo->GetCallerID();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("Called ID    = ");
		strID += pCInfo->GetCalledID();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("Connected ID = ");
		strID += pCInfo->GetConnectedID();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("Trunk = ");
		strID += pCInfo->GetTrunk();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("Origin = ");
		strID += pCInfo->GetOrigin();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("Reason = ");
		strID += pCInfo->GetReason();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("RedirectionID = ");
		strID += pCInfo->GetRedirectionID();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("RedirectingID = ");
		strID += pCInfo->GetRedirectingID();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);

		strID = _T("DID/DDI = ");
		strID += pCInfo->GetDIDDDI();
		hCTree = pTree->GetNextSiblingItem(hCTree);
		pTree->SetItemText(hCTree,strID);
	} else {
		strID = _T("Create Time        = ");
		strID += strTime;
		pTree->InsertItem(strID,hTree);

		strID = _T("Caller ID    = ");
		strID += pCInfo->GetCallerID();
		pTree->InsertItem(strID,hTree);

		strID = _T("Called ID    = ");
		strID += pCInfo->GetCalledID();
		pTree->InsertItem(strID,hTree);

		strID = _T("Connected ID = ");
		strID += pCInfo->GetConnectedID();
		pTree->InsertItem(strID,hTree);

		strID = _T("Trunk = ");
		strID += pCInfo->GetTrunk();
		pTree->InsertItem(strID,hTree);

		strID = _T("Origin = ");
		strID += pCInfo->GetOrigin();
		pTree->InsertItem(strID,hTree);

		strID = _T("Reason = ");
		strID += pCInfo->GetReason();
		pTree->InsertItem(strID,hTree);

		strID = _T("RedirectionID = ");
		strID += pCInfo->GetRedirectionID();
		pTree->InsertItem(strID,hTree);

		strID = _T("RedirectingID = ");
		strID += pCInfo->GetRedirectingID();
		pTree->InsertItem(strID,hTree);

		strID = _T("DID/DDI = ");
		strID += pCInfo->GetDIDDDI();
		pTree->InsertItem(strID,hTree);

		pTree->Expand(hTree,TVE_EXPAND);
	}

	
	string status = pTree->GetItemText(hTree);
	HTREEITEM prevCTree = pTree->GetParentItem(hTree);
	string topItem = pTree->GetItemText(prevCTree);
	string extension = topItem.substr(0, 4);

	string lineName;
	if (topItem.length() > 5) lineName = topItem.substr(5, topItem.length());

	if (is_number(extension) && status.substr(0, 6) != "ONHOLD" && status.substr(0, 10) != "DISCONNECT" && stoi(extension) >= 1000){
		string callerID = pCInfo->GetCallerID();
		string calledID = pCInfo->GetCalledID();
		string redirectionID = pCInfo->GetRedirectionID();
		string textFile, callType, phoneNumber = "null";

		if (callerID.length() > 8 && is_number(callerID)){
			callType = "entrante";
			phoneNumber = callerID;
		}
		if (calledID.length() > 8 && is_number(calledID)){
			callType = "saliente";
			phoneNumber = calledID;
		}
		if (redirectionID.length() > 8 && is_number(redirectionID)){
			callType = "entrante";
			phoneNumber = redirectionID;
		}

		if (phoneNumber != "null"){

			textFile = callType + ":" + phoneNumber;

			ofstream myfile;

			string file_name = "ExtText/" + extension + ".txt";

			myfile.open(file_name.c_str(), ios::out);

			myfile << textFile;

			myfile.close();
			
			// Save daily report
			SYSTEMTIME stime;
			::GetLocalTime(&stime);

			CString date, time, reportName;
			
			date.Format("%02d/%02d/%04d", stime.wDay, stime.wMonth, stime.wYear);

			time.Format("%02d:%02d:%02d", stime.wHour, stime.wMinute, stime.wSecond);

			reportName.Format("%02d-%02d-%04d.csv", stime.wDay, stime.wMonth, stime.wYear);

			ofstream reportFile;
			reportFile.open("DailyReports/" + reportName, std::ios_base::app);


			reportFile << status << "," << date << "," << time << "," << extension << "," << lineName << "," 
				       << pCInfo->GetTrunk() << "," << callType << "," << phoneNumber << "," << pCInfo->GetRedirectingID() << endl;

			reportFile.close();
			
			
		}
	}



	delete pCInfo;
}

void CCallData::WriteLogInfoData(HCALL hCall,CTreeCtrl* pTree,HTREEITEM hTree,LPCTSTR strMes)
{
	CD_LineCallInfo* pCInfo = GetCallInfo(hCall);
	if (pCInfo==NULL) return;
	CString strLog;
	CString strTemp;
	HTREEITEM hPTree = pTree->GetParentItem(hTree);
	strLog = pTree->GetItemText(hPTree)+SEPARATE_ST;
	strLog += _T("LINE_CALLINFO");
	strLog += SEPARATE_ST;
	strTemp.Format( "hCall=%X(%d)", (DWORD)hCall, (DWORD)hCall );
	strTemp += SEPARATE_ST;
	strLog += strTemp;
	strLog += strMes;
	strLog += SEPARATE_ST;
	strLog += pCInfo->GetLogString();
	WriteLogString(strLog);
	delete pCInfo;
}

BOOL CCallData::Init()
{

	HINSTANCE g_hInst = AfxGetInstanceHandle();

	CCallMonitorApp* pApp = (CCallMonitorApp*)AfxGetApp();
	if (pApp == NULL) return FALSE;

	pApp->m_progress = TRUE;
	CProgDlg* pdlg = new CProgDlg;
	pdlg->Create();
	pdlg->SetRange(0,100);
	pdlg->SetStatus("Initializing");

	LINEINITIALIZEEXPARAMS exparams = {0};
	exparams.dwTotalSize = sizeof( LINEINITIALIZEEXPARAMS );
	exparams.dwOptions = LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

	DWORD dwLineDevNum=0;
	DWORD dwAPIVersion = m_APIVersionLow;
	LONG Init_ret = lineInitializeEx( &m_hLineApp, g_hInst, Callback, 
		"CallMonitor3", &dwLineDevNum, &dwAPIVersion, &exparams );
	if (Init_ret) return FALSE;
	m_init = TRUE;
	if (!dwLineDevNum) return FALSE;

	pdlg->SetPos(20);

	BOOL open_flag=FALSE;
	for (DWORD i = 0;i < dwLineDevNum; i ++ ) {
		pdlg->SetPos(20+20*i/dwLineDevNum);
		{
			char data[100];
			sprintf( data, "GetDevCaps/GetAddressCaps:%d/%d", i+1, dwLineDevNum );
			pdlg->SetStatus(data);
		}
		if (pdlg->CheckCancelButton()) {
			pdlg->DestroyWindow();
			delete pdlg;
			return FALSE;
		}
		CD_LineDevCaps* pDevCaps = GetDevCaps(i);
		if (pDevCaps!=NULL) {
			if (pDevCaps->IsExtLine()) {
				m_ExtLineDevCapsList.push_back(pDevCaps);
			} else if (pDevCaps->IsCOLine())  {
				m_COLineDevCapsList.push_back(pDevCaps);
			} else {
				m_EtcLineDevCapsList.push_back(pDevCaps);
			}
			CD_LineAddressCaps* pAddCaps = GetAddressCaps(i,0);
			if (pAddCaps!=NULL) {
				m_LineAddressCapsList.push_back(pAddCaps);
				pDevCaps->SetAddressCaps(pAddCaps);
			}
			open_flag=TRUE;
		}
	}
	if (!open_flag) {
		return FALSE;
	}
	
	long listcount = m_ExtLineDevCapsList.size();
	for ( long i=0;i<listcount;i++) {
		pdlg->SetPos(40+20*i/listcount);
		{
			char data[100];
			sprintf( data, "ExtLineOpen:%d/%d", i+1, listcount );
			pdlg->SetStatus(data);
		}
		if (pdlg->CheckCancelButton()) {
			pdlg->DestroyWindow();
			delete pdlg;
			return FALSE;
		}
		CD_LineDevCaps* pDevCaps = m_ExtLineDevCapsList[i];
		DWORD devid = pDevCaps->GetDeviceID();

		DWORD EXTVersion = m_EXTVersion;
		lineNegotiateExtVersion( m_hLineApp,devid, m_APIVersionLow, m_EXTVersion, 0xffffffff, &EXTVersion );
		HLINE hLine = 0;

		LONG ret = lineOpen(m_hLineApp,devid,&hLine, 0x20000,EXTVersion,
			0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
		if (ret != 0) {
			ret = lineOpen(m_hLineApp,devid,&hLine, 0x10004,EXTVersion,
				0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
			if (ret != 0) {
				ret = lineOpen(m_hLineApp,devid,&hLine, 0x10003,EXTVersion,
					0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
				if (ret != 0) {
					continue;
				}
			}
		}

		::lineSetStatusMessages(hLine,
			LINEDEVSTATE_OTHER|LINEDEVSTATE_RINGING|LINEDEVSTATE_CONNECTED|LINEDEVSTATE_DISCONNECTED|
			LINEDEVSTATE_MSGWAITON|LINEDEVSTATE_MSGWAITOFF|LINEDEVSTATE_INSERVICE|LINEDEVSTATE_OUTOFSERVICE|
			LINEDEVSTATE_MAINTENANCE|LINEDEVSTATE_OPEN|LINEDEVSTATE_CLOSE|LINEDEVSTATE_NUMCALLS|LINEDEVSTATE_NUMCOMPLETIONS|
			LINEDEVSTATE_TERMINALS|LINEDEVSTATE_ROAMMODE|LINEDEVSTATE_BATTERY|LINEDEVSTATE_SIGNAL|LINEDEVSTATE_DEVSPECIFIC|
			LINEDEVSTATE_REINIT|LINEDEVSTATE_LOCK|LINEDEVSTATE_CAPSCHANGE|LINEDEVSTATE_CONFIGCHANGE|LINEDEVSTATE_TRANSLATECHANGE|
			LINEDEVSTATE_COMPLCANCEL|LINEDEVSTATE_REMOVED,
			LINEADDRESSSTATE_OTHER|LINEADDRESSSTATE_DEVSPECIFIC|LINEADDRESSSTATE_INUSEZERO|LINEADDRESSSTATE_INUSEONE|
			LINEADDRESSSTATE_INUSEMANY|LINEADDRESSSTATE_NUMCALLS|LINEADDRESSSTATE_FORWARD|LINEADDRESSSTATE_TERMINALS|LINEADDRESSSTATE_CAPSCHANGE
			);

		pDevCaps->SethLine(hLine);
		m_ExtMapLine2Data[hLine] = pDevCaps;
		CD_LineAddressStatus* pAddStatus = GetAddressStatus(hLine,0);
		if (pAddStatus != NULL) {
			pDevCaps->SetDevName(pAddStatus->GetExtName());
			delete pAddStatus;
		}
	}

	listcount = m_COLineDevCapsList.size();
	for ( long i=0;i<listcount;i++) {
		pdlg->SetPos(60+20*i/listcount);
		{
			char data[100];
			sprintf( data, "COLineOpen:%d/%d", i+1, listcount );
			pdlg->SetStatus(data);
		}
		if (pdlg->CheckCancelButton()) {
			pdlg->DestroyWindow();
			delete pdlg;
			return FALSE;
		}
		CD_LineDevCaps* pDevCaps = m_COLineDevCapsList[i];
		DWORD devid = pDevCaps->GetDeviceID();

		DWORD EXTVersion = m_EXTVersion;
		lineNegotiateExtVersion( m_hLineApp,devid, m_APIVersionLow, m_EXTVersion, 0xffffffff, &EXTVersion );

		HLINE hLine = 0;

		LONG ret = lineOpen(m_hLineApp,devid,&hLine, 0x20000,EXTVersion,
			0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
		if (ret != 0) {
			ret = lineOpen(m_hLineApp,devid,&hLine, 0x10004,EXTVersion,
				0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
			if (ret != 0) {
				ret = lineOpen(m_hLineApp,devid,&hLine, 0x10003,EXTVersion,
					0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
				if (ret != 0) {
					continue;
				}
			}
		}
		::lineSetStatusMessages(hLine,
			LINEDEVSTATE_OTHER|LINEDEVSTATE_RINGING|LINEDEVSTATE_CONNECTED|LINEDEVSTATE_DISCONNECTED|
			LINEDEVSTATE_MSGWAITON|LINEDEVSTATE_MSGWAITOFF|LINEDEVSTATE_INSERVICE|LINEDEVSTATE_OUTOFSERVICE|
			LINEDEVSTATE_MAINTENANCE|LINEDEVSTATE_OPEN|LINEDEVSTATE_CLOSE|LINEDEVSTATE_NUMCALLS|LINEDEVSTATE_NUMCOMPLETIONS|
			LINEDEVSTATE_TERMINALS|LINEDEVSTATE_ROAMMODE|LINEDEVSTATE_BATTERY|LINEDEVSTATE_SIGNAL|LINEDEVSTATE_DEVSPECIFIC|
			LINEDEVSTATE_REINIT|LINEDEVSTATE_LOCK|LINEDEVSTATE_CAPSCHANGE|LINEDEVSTATE_CONFIGCHANGE|LINEDEVSTATE_TRANSLATECHANGE|
			LINEDEVSTATE_COMPLCANCEL|LINEDEVSTATE_REMOVED,
			LINEADDRESSSTATE_OTHER|LINEADDRESSSTATE_DEVSPECIFIC|LINEADDRESSSTATE_INUSEZERO|LINEADDRESSSTATE_INUSEONE|
			LINEADDRESSSTATE_INUSEMANY|LINEADDRESSSTATE_NUMCALLS|LINEADDRESSSTATE_FORWARD|LINEADDRESSSTATE_TERMINALS|LINEADDRESSSTATE_CAPSCHANGE
			);
		pDevCaps->SethLine(hLine);
		m_COMapLine2Data[hLine] = pDevCaps;
		CD_LineAddressStatus* pAddStatus = GetAddressStatus(hLine,0);
		if (pAddStatus != NULL) {
			pDevCaps->SetDevName(pAddStatus->GetCOName());
			delete pAddStatus;
		}
	}

	listcount = m_EtcLineDevCapsList.size();
	for ( long i=0;i<listcount;i++) {
		pdlg->SetPos(80+20*i/listcount);
		{
			char data[100];
			sprintf( data, "EtcLineOpen:%d/%d", i+1, listcount );
			pdlg->SetStatus(data);
		}
		if (pdlg->CheckCancelButton()) {
			pdlg->DestroyWindow();
			delete pdlg;
			return FALSE;
		}
		CD_LineDevCaps* pDevCaps = m_EtcLineDevCapsList[i];
		DWORD devid = pDevCaps->GetDeviceID();

		DWORD EXTVersion = m_EXTVersion;
		lineNegotiateExtVersion( m_hLineApp,devid, m_APIVersionLow, m_EXTVersion, 0xffffffff, &EXTVersion );

		HLINE hLine = 0;

		LONG ret = lineOpen(m_hLineApp,devid,&hLine, 0x20000,EXTVersion,
			0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
		if (ret != 0) {
			ret = lineOpen(m_hLineApp,devid,&hLine, 0x10004,EXTVersion,
				0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
			if (ret != 0) {
				ret = lineOpen(m_hLineApp,devid,&hLine, 0x10003,EXTVersion,
					0,LINECALLPRIVILEGE_MONITOR,LINEMEDIAMODE_INTERACTIVEVOICE, NULL);
				if (ret != 0) {
					continue;
				}
			}
		}
		::lineSetStatusMessages(hLine,
			LINEDEVSTATE_OTHER|LINEDEVSTATE_RINGING|LINEDEVSTATE_CONNECTED|LINEDEVSTATE_DISCONNECTED|
			LINEDEVSTATE_MSGWAITON|LINEDEVSTATE_MSGWAITOFF|LINEDEVSTATE_INSERVICE|LINEDEVSTATE_OUTOFSERVICE|
			LINEDEVSTATE_MAINTENANCE|LINEDEVSTATE_OPEN|LINEDEVSTATE_CLOSE|LINEDEVSTATE_NUMCALLS|LINEDEVSTATE_NUMCOMPLETIONS|
			LINEDEVSTATE_TERMINALS|LINEDEVSTATE_ROAMMODE|LINEDEVSTATE_BATTERY|LINEDEVSTATE_SIGNAL|LINEDEVSTATE_DEVSPECIFIC|
			LINEDEVSTATE_REINIT|LINEDEVSTATE_LOCK|LINEDEVSTATE_CAPSCHANGE|LINEDEVSTATE_CONFIGCHANGE|LINEDEVSTATE_TRANSLATECHANGE|
			LINEDEVSTATE_COMPLCANCEL|LINEDEVSTATE_REMOVED,
			LINEADDRESSSTATE_OTHER|LINEADDRESSSTATE_DEVSPECIFIC|LINEADDRESSSTATE_INUSEZERO|LINEADDRESSSTATE_INUSEONE|
			LINEADDRESSSTATE_INUSEMANY|LINEADDRESSSTATE_NUMCALLS|LINEADDRESSSTATE_FORWARD|LINEADDRESSSTATE_TERMINALS|LINEADDRESSSTATE_CAPSCHANGE
			);
		pDevCaps->SethLine(hLine);
		m_EtcMapLine2Data[hLine] = pDevCaps;
		if (pDevCaps->IsGroupLine()) {
			CD_LineAddressStatus* pAddStatus = GetAddressStatus(hLine,0);
			if (pAddStatus != NULL) {
				pDevCaps->SetDevName(pAddStatus->GetGroupName());
				delete pAddStatus;
			}
		}
	}
	pdlg->DestroyWindow();
	delete pdlg;
	pApp->m_progress = FALSE;

	return TRUE;
}

CD_LineDevCaps* CCallData::GetDevCaps(DWORD dwDeviceID)
{
	DWORD EXTVersion = m_EXTVersion;
	DWORD dwTotalSize = sizeof( LINEDEVCAPS);
	LPLINEDEVCAPS pLineDevCaps = (LPLINEDEVCAPS)malloc(dwTotalSize);
	if (pLineDevCaps==NULL) return NULL;
	pLineDevCaps->dwTotalSize = dwTotalSize;
	while (TRUE) {
		lineNegotiateExtVersion( m_hLineApp,dwDeviceID, m_APIVersionLow, m_EXTVersion, 0xffffffff, &EXTVersion );

		LONG ret = lineGetDevCaps(m_hLineApp, dwDeviceID, 0x20000, 0x0, pLineDevCaps);
		if (ret != 0) {
			ret = lineGetDevCaps(m_hLineApp, dwDeviceID, 0x10004, 0x0, pLineDevCaps);
			if (ret != 0) {
				ret = lineGetDevCaps(m_hLineApp, dwDeviceID, 0x10003, 0x0, pLineDevCaps);
				if (ret != 0) {
					free(pLineDevCaps);
					return NULL;
				}
			}
		}
		if (pLineDevCaps->dwTotalSize < pLineDevCaps->dwNeededSize) {
			dwTotalSize = pLineDevCaps->dwNeededSize;
			pLineDevCaps = (LPLINEDEVCAPS)realloc(pLineDevCaps,dwTotalSize);
			pLineDevCaps->dwTotalSize = dwTotalSize;
		} else {
			break;
		}
	}

	CD_LineDevCaps* pData = new CD_LineDevCaps(pLineDevCaps);
	pData->CheckProvider();
	pData->SetDeviceID(dwDeviceID);
	free(pLineDevCaps);

	return pData;
}

CD_LineDevStatus* CCallData::GetLineDevStatus(HLINE hLine)
{
	DWORD dwTotalSize = sizeof( LINEDEVSTATUS);
	LPLINEDEVSTATUS pLineDevStatus = (LPLINEDEVSTATUS)malloc(dwTotalSize);
	if (pLineDevStatus==NULL) return NULL;
	pLineDevStatus->dwTotalSize = dwTotalSize;
	while (TRUE) {
		LONG ret = lineGetLineDevStatus(hLine,pLineDevStatus);
		if (ret != 0) {
			free(pLineDevStatus);
			return NULL;
		}
		if (pLineDevStatus->dwTotalSize < pLineDevStatus->dwNeededSize) {
			dwTotalSize = pLineDevStatus->dwNeededSize;
			pLineDevStatus = (LPLINEDEVSTATUS)realloc(pLineDevStatus,dwTotalSize);
			pLineDevStatus->dwTotalSize = dwTotalSize;
		} else {
			break;
		}
	}
	CD_LineDevStatus* pData = new CD_LineDevStatus(pLineDevStatus);

	free(pLineDevStatus);

	return pData;
}


CD_LineAddressCaps* CCallData::GetAddressCaps(DWORD dwDeviceID,DWORD dwAddressID)
{
	DWORD EXTVersion = m_EXTVersion;
	DWORD dwTotalSize = sizeof( LINEADDRESSCAPS);
	LPLINEADDRESSCAPS pLineAddressCaps = (LPLINEADDRESSCAPS)malloc(dwTotalSize);
	if (pLineAddressCaps==NULL) return NULL;
	pLineAddressCaps->dwTotalSize = dwTotalSize;
	while (TRUE) {
		lineNegotiateExtVersion( m_hLineApp,dwDeviceID, m_APIVersionLow, m_EXTVersion, 0xffffffff, &EXTVersion );
		LONG ret = lineGetAddressCaps(m_hLineApp, dwDeviceID, dwAddressID, 0x20000, 0, pLineAddressCaps);
		if (ret != 0) {
			LONG ret = lineGetAddressCaps(m_hLineApp, dwDeviceID, dwAddressID, 0x10004, 0, pLineAddressCaps);
			if (ret != 0) {
				LONG ret = lineGetAddressCaps(m_hLineApp, dwDeviceID, dwAddressID, 0x10003, 0, pLineAddressCaps);
				if (ret != 0) {
					free(pLineAddressCaps);
					return NULL;
				}
			}
		}
		if (pLineAddressCaps->dwTotalSize < pLineAddressCaps->dwNeededSize) {
			dwTotalSize = pLineAddressCaps->dwNeededSize;
			pLineAddressCaps = (LPLINEADDRESSCAPS)realloc(pLineAddressCaps,dwTotalSize);
			pLineAddressCaps->dwTotalSize = dwTotalSize;
		} else {
			break;
		}
	}
	CD_LineAddressCaps* pData = new CD_LineAddressCaps(pLineAddressCaps);

	free(pLineAddressCaps);

	return pData;
}

CD_LineAddressStatus* CCallData::GetAddressStatus(HLINE hLine,DWORD dwAddressID)
{
	DWORD dwTotalSize = sizeof(LINEADDRESSSTATUS);
	LPLINEADDRESSSTATUS pLineAddressStatus = (LPLINEADDRESSSTATUS)malloc(dwTotalSize);
	if (pLineAddressStatus==NULL) return NULL;
	pLineAddressStatus->dwTotalSize = dwTotalSize;
	while (TRUE) {
		LONG ret = lineGetAddressStatus(hLine, dwAddressID, pLineAddressStatus);
		if (ret != 0) {
			free(pLineAddressStatus);
			return NULL;
		}
		if (pLineAddressStatus->dwTotalSize < pLineAddressStatus->dwNeededSize) {
			dwTotalSize = pLineAddressStatus->dwNeededSize;
			pLineAddressStatus = (LPLINEADDRESSSTATUS)realloc(pLineAddressStatus,dwTotalSize);
			pLineAddressStatus->dwTotalSize = dwTotalSize;
		} else {
			break;
		}
	}
	CD_LineAddressStatus* pData = new CD_LineAddressStatus(pLineAddressStatus);

	free(pLineAddressStatus);

	return pData;
}

CD_LineCallInfo* CCallData::GetCallInfo(HCALL hCall)
{
	DWORD dwTotalSize = sizeof(LINECALLINFO);
	LPLINECALLINFO pLineCallInfo = (LPLINECALLINFO)malloc(dwTotalSize);
	if (pLineCallInfo==NULL) return NULL;
	pLineCallInfo->dwTotalSize = dwTotalSize;
	while (TRUE) {
		LONG ret = lineGetCallInfo(hCall, pLineCallInfo);
		if (ret != 0) {
			free(pLineCallInfo);
			return NULL;
		}
		if (pLineCallInfo->dwTotalSize < pLineCallInfo->dwNeededSize) {
			dwTotalSize = pLineCallInfo->dwNeededSize;
			pLineCallInfo = (LPLINECALLINFO)realloc(pLineCallInfo,dwTotalSize);
			pLineCallInfo->dwTotalSize = dwTotalSize;
		} else {
			break;
		}
	}
	CD_LineCallInfo* pData = new CD_LineCallInfo(pLineCallInfo);
	free(pLineCallInfo);

	return pData;
}

CD_LineCallStatus* CCallData::GetCallStatus(HCALL hCall)
{
	DWORD dwTotalSize = sizeof(LINECALLSTATUS);
	LPLINECALLSTATUS pLineCallStatus = (LPLINECALLSTATUS)malloc(dwTotalSize);
	if (pLineCallStatus==NULL) return NULL;
	pLineCallStatus->dwTotalSize = dwTotalSize;
	while (TRUE) {
		LONG ret = lineGetCallStatus(hCall, pLineCallStatus);
		if (ret != 0) {
			free(pLineCallStatus);
			return NULL;
		}

		if (pLineCallStatus->dwTotalSize < pLineCallStatus->dwNeededSize) {
			dwTotalSize = pLineCallStatus->dwNeededSize;
			pLineCallStatus = (LPLINECALLSTATUS)realloc(pLineCallStatus,dwTotalSize);
			pLineCallStatus->dwTotalSize = dwTotalSize;
		} else {
			break;
		}
	}
	CD_LineCallStatus* pData = new CD_LineCallStatus(pLineCallStatus);
	free(pLineCallStatus);

	return pData;
}

int CCallData::GetCODataSize()
{
	return m_COLineDevCapsList.size();
}

CD_LineDevCaps* CCallData::GetCOData(int no)
{
	if (no<0 || no>(int)m_COLineDevCapsList.size()) {
		return NULL;
	}
	return m_COLineDevCapsList[no];
}

int CCallData::GetExtDataSize()
{
	return m_ExtLineDevCapsList.size();
}

CD_LineDevCaps* CCallData::GetExtData(int no)
{
	if (no<0 || no>(int)m_ExtLineDevCapsList.size()) {
		return NULL;
	}
	return m_ExtLineDevCapsList[no];
}

int CCallData::GetEtcDataSize()
{
	return m_EtcLineDevCapsList.size();
}

CD_LineDevCaps* CCallData::GetEtcData(int no)
{
	if (no<0 || no>(int)m_EtcLineDevCapsList.size()) {
		return NULL;
	}
	return m_EtcLineDevCapsList[no];
}

void CCallData::TreeExtSet(CTreeCtrl* pTree)
{
	m_pExtTree = pTree;
}

void CCallData::TreeCOSet(CTreeCtrl* pTree)
{
	m_pCOTree = pTree;
}

void CCallData::TreeEtcSet(CTreeCtrl* pTree)
{
	m_pEtcTree = pTree;
}

void CCallData::TreeExpExtData()
{
	if (m_pExtTree==NULL) return;

	HTREEITEM hRoot = m_pExtTree->InsertItem(EXT_ROOT);
	m_hExtTreeRoot = hRoot;
	int lsize = GetExtDataSize();
	for (int i=0;i<lsize;i++) {
		CD_LineDevCaps* pData = GetExtData(i);
		CString lineName = pData->GetLineName();
		CString str = pData->GetAddressCaps()->GetAddress();
		CString extName = pData->GetDevName();
		if (extName.GetLength()) str += _T(" ")+extName;
		hRoot = m_pExtTree->InsertItem(str,m_hExtTreeRoot);
		pData->SethTree(hRoot);
		m_ExtMapTree2Data[hRoot] = pData;
	}
	m_pExtTree->Expand(m_hExtTreeRoot,TVE_EXPAND);
	m_pExtTree->SortChildren(m_hExtTreeRoot);

}

void CCallData::TreeExpCOData()
{
	if (m_pCOTree==NULL) return;

	HTREEITEM hRoot = m_pCOTree->InsertItem(CO_ROOT);
	m_hCOTreeRoot = hRoot;
	int lsize = GetCODataSize();
	for (int i=0;i<lsize;i++) {
		CD_LineDevCaps* pData = GetCOData(i);
		CString str = pData->GetAddressCaps()->GetAddressP2();
		CString devName = pData->GetDevName();
		if (devName.GetLength()) str += _T(" ")+devName;
		hRoot = m_pCOTree->InsertItem(str,m_hCOTreeRoot);
		pData->SethTree(hRoot);
		m_COMapTree2Data[hRoot] = pData;
	}
	m_pCOTree->Expand(m_hCOTreeRoot,TVE_EXPAND);
	m_pCOTree->SortChildren(m_hCOTreeRoot);

}

void CCallData::TreeExpEtcData()
{
	if (m_pEtcTree==NULL) return;

	HTREEITEM hRoot = m_pEtcTree->InsertItem(ETC_ROOT);
	m_hEtcTreeRoot = hRoot;
	int lsize = GetEtcDataSize();
	for (int i=0;i<lsize;i++) {
		CD_LineDevCaps* pData = GetEtcData(i);
		CString str = pData->GetLineName();
		CString str2 = pData->GetAddressCaps()->GetAddress();
		if (str2.GetLength() && str != str2) str += _T(" ")+str2;
		CString devName = pData->GetDevName();
		if (devName.GetLength()) str += _T(" ")+devName;
		hRoot = m_pEtcTree->InsertItem(str,m_hEtcTreeRoot);
		pData->SethTree(hRoot);
		m_EtcMapTree2Data[hRoot] = pData;
	}
	m_pEtcTree->Expand(m_hEtcTreeRoot,TVE_EXPAND);
	m_pEtcTree->SortChildren(m_hEtcTreeRoot);

}

int CCallData::MakeCall(HTREEITEM hTree,LPCSTR pDialNo)
{
	CString dialno(pDialNo);

	map <HTREEITEM,CD_LineDevCaps*>::iterator it1;
	it1 = m_ExtMapTree2Data.find(hTree);
	if (it1!=m_ExtMapTree2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		HLINE hLine = pDevCaps->GethLine();
		LONG ret = lineMakeCall(hLine,&m_hMCCall,dialno,0,NULL);
		if (ret>0) {
			m_RequestID_MakeCall = ret;
			m_hMCLine = hLine;
		}
	}

	return 0;
}

BOOL CCallData::CanAnswer(HTREEITEM hTree)
{
	BOOL retcd=TRUE;
	HCALL hCall = (HCALL)m_pExtTree->GetItemData(hTree);
	if (hCall==NULL) {
		return FALSE;
	}
	CD_LineCallStatus* pCStatus = GetCallStatus(hCall);
	if (pCStatus==NULL) return FALSE;
	if (!pCStatus->CanAnswer()) {
		retcd=FALSE;
	}
	delete pCStatus;
	return retcd;
}

int CCallData::Answer(HTREEITEM hTree)
{
	if (!CanAnswer(hTree)) {
		return -1;
	}
	HCALL hCall = (HCALL)m_pExtTree->GetItemData(hTree);
	if (hCall==NULL) {
		return -1;
	}
	LONG ret = lineAnswer(hCall,NULL,0);
	if (ret>0) {
		m_RequestID_Answer = ret;
	}
	return 0;
}

BOOL CCallData::CanDrop(HTREEITEM hTree)
{
	BOOL retcd=TRUE;
	HCALL hCall = (HCALL)m_pExtTree->GetItemData(hTree);
	if (hCall==NULL) {
		return FALSE;
	}
	CD_LineCallStatus* pCStatus = GetCallStatus(hCall);
	if (pCStatus==NULL) return FALSE;
	if (!pCStatus->CanDrop()) {
		retcd=FALSE;
	}
	delete pCStatus;
	return retcd;
}

int CCallData::Drop(HTREEITEM hTree)
{
	if (!CanDrop(hTree)) {
		return -1;
	}
	HCALL hCall = (HCALL)m_pExtTree->GetItemData(hTree);
	if (hCall==NULL) {
		return -1;
	}
	LONG ret = lineDrop(hCall,NULL,0);
	if (ret>0) {
		m_RequestID_Drop = ret;
	}
	return 0;
}

void CCallData::LineInformationCO(HTREEITEM hTree)
{
	CD_LineDevCaps* pDevCaps=NULL;

	map <HTREEITEM,CD_LineDevCaps*>::iterator it1;
	it1 = m_COMapTree2Data.find(hTree);
	if (it1!=m_COMapTree2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		ViewLineInformation(pDevCaps);
	}
}

void CCallData::LineInformationExt(HTREEITEM hTree)
{
	CD_LineDevCaps* pDevCaps=NULL;

	map <HTREEITEM,CD_LineDevCaps*>::iterator it1;
	it1 = m_ExtMapTree2Data.find(hTree);
	if (it1!=m_ExtMapTree2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		ViewLineInformation(pDevCaps);
	}
}

void CCallData::LineInformationEtc(HTREEITEM hTree)
{
	CD_LineDevCaps* pDevCaps=NULL;

	map <HTREEITEM,CD_LineDevCaps*>::iterator it1;
	it1 = m_EtcMapTree2Data.find(hTree);
	if (it1!=m_EtcMapTree2Data.end()) {
		CD_LineDevCaps* pDevCaps= (*it1).second;
		ViewLineInformation(pDevCaps);
	}
}

void CCallData::ViewLineInformation(CD_LineDevCaps* pDevCaps)
{
	CInformDlg dlg;

	DWORD deviceID = pDevCaps->GetDeviceID();
	HLINE hLine = pDevCaps->GethLine();
	CD_LineDevCaps* pDevCapsNew = GetDevCaps(deviceID);
	dlg.m_text = pDevCapsNew->GetInformation();
	delete pDevCapsNew;

	if (hLine != 0) {
		CD_LineDevStatus* pLineDevStatus = GetLineDevStatus(hLine);
		dlg.m_text += pLineDevStatus->GetInformation();
		delete pLineDevStatus;
	}

	CD_LineAddressCaps* pAddressCaps = GetAddressCaps(deviceID,0);
	dlg.m_text += pAddressCaps->GetInformation();
	delete pAddressCaps;

	if (hLine != 0) {
		CD_LineAddressStatus* pAddressStatus = GetAddressStatus(hLine,0);
		dlg.m_text += pAddressStatus->GetInformation();
		delete pAddressStatus;
	}
	dlg.DoModal();
}

void CCallData::CallInformationCO(HTREEITEM hTree)
{
	HCALL hCall = (HCALL)m_pCOTree->GetItemData(hTree);
	if (hCall!=NULL) {
		ViewCallInformation(hCall);
	}
}

void CCallData::CallInformationExt(HTREEITEM hTree)
{
	HCALL hCall = (HCALL)m_pExtTree->GetItemData(hTree);
	if (hCall!=NULL) {
		ViewCallInformation(hCall);
	}
}

void CCallData::CallInformationEtc(HTREEITEM hTree)
{
	HCALL hCall = (HCALL)m_pEtcTree->GetItemData(hTree);
	if (hCall!=NULL) {
		ViewCallInformation(hCall);
	}
}

void CCallData::ViewCallInformation(HCALL hCall)
{
	CInformDlg dlg;

	CD_LineCallInfo* pLineCallInfo = GetCallInfo(hCall);
	dlg.m_text += pLineCallInfo->GetInformation();
	delete pLineCallInfo;

	CD_LineCallStatus* pLineCallStatus = GetCallStatus(hCall);
	dlg.m_text += pLineCallStatus->GetInformation();
	delete pLineCallStatus;

	dlg.DoModal();
}

CString CCallData::GetTimeString()
{
	static CString strTime;
	SYSTEMTIME stime;
	::GetLocalTime(&stime);
	strTime.Format("%04d/%02d/%02d %02d:%02d:%02d:%03d",
		stime.wYear,stime.wMonth,stime.wDay,
		stime.wHour,stime.wMinute,stime.wSecond,stime.wMilliseconds);
	return strTime;
}

void CCallData::WriteLogString( LPCTSTR  pText)
{
	CString strFileTime;
	SYSTEMTIME stime;
	::GetLocalTime(&stime);
	strFileTime.Format("%04d%02d%02d.csv",
		stime.wYear,stime.wMonth,stime.wDay);

	CString strText(pText);

	CString strLog;
	strLog = GetTimeString();

	strLog += SEPARATE_ST+strText+"\n";

	CStdioFile* LogFile = NULL;

	try {
		LogFile = new CStdioFile(m_LogFileName+strFileTime,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::typeText|CFile::shareDenyWrite);
	} catch (CFileException e) {

	}

	if (LogFile) {
		try {
			LogFile->SeekToEnd();
			LogFile->WriteString(strLog);
		} catch (CFileException e) {

		}
	}

	if (LogFile) {
		LogFile->Close();
		delete LogFile;
	}

}