#include "stdafx.h"
#include "CallMonitor3.h"
#include "CallData.h"
#include "CallMonitorDoc.h"

#include <sys/stat.h>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LOG_FILENAME   _T("testlog")

IMPLEMENT_DYNCREATE(CCallMonitorDoc, CDocument)

BEGIN_MESSAGE_MAP(CCallMonitorDoc, CDocument)
	//{{AFX_MSG_MAP(CCallMonitorDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CCallMonitorDoc::CCallMonitorDoc()
{
	m_calldata = NULL;
	m_init_error = FALSE;

}

CCallMonitorDoc::~CCallMonitorDoc()
{
}

BOOL CCallMonitorDoc::OnNewDocument()
{

	if (!CDocument::OnNewDocument())
		return FALSE;

	m_calldata = new CCallData;
	m_calldata->SetLogFileName(LOG_FILENAME);
	if (!m_calldata->Init()) {
		m_init_error = TRUE;
	}
	
	// Save report file
	SYSTEMTIME stime;
	::GetLocalTime(&stime);
	CString reportName;
	reportName.Format("%02d-%02d-%04d.csv", stime.wDay, stime.wMonth, stime.wYear);

	struct stat buffer;
	bool fileExist = (stat("DailyReports/" + reportName, &buffer) == 0);

	if (!fileExist){
		ofstream reportFile;
		reportFile.open("DailyReports/" + reportName, std::ios_base::app);

		reportFile << "Type" << "," << "Date" << "," << "Time" << "," << "Extension" << ","
			<< "Name" << "," << "Trunk" << "," << "Call Type" << "," << "Phone Number" << "," << "Group" << endl;

		reportFile.close();
	}
	

	return TRUE;
}



void CCallMonitorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

#ifdef _DEBUG
void CCallMonitorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCallMonitorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void CCallMonitorDoc::DeleteContents() 
{
	if (m_calldata) {
		delete m_calldata;
		m_calldata = NULL;
	}
	
	CDocument::DeleteContents();
}

void CCallMonitorDoc::OnCloseDocument() 
{
	
	CDocument::OnCloseDocument();
}
