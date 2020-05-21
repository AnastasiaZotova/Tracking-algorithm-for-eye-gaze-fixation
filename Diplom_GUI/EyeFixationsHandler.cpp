
// Diplom_GUI.cpp: 
//

#include "pch.h"
#include "framework.h"
#include "EyeFixationsHandler.h"
#include "EyeFixationsHandlerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDiplomGUIApp

BEGIN_MESSAGE_MAP(CDiplomGUIApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDiplomGUIApp

CDiplomGUIApp::CDiplomGUIApp()
{

	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;


}



CDiplomGUIApp theApp;


BOOL CDiplomGUIApp::InitInstance()
{

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();


	CShellManager *pShellManager = new CShellManager;

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));


	SetRegistryKey(_T("_"));

	CDiplomGUIDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) 	{}
	else if (nResponse == IDCANCEL)	{}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning. The dialog box could not be created, so the application terminated unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning. When using MFC controls for a dialog box, it is not possible #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif


	return FALSE;
}

