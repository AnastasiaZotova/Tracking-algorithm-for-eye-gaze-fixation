
// Diplom_GUI.h: 
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		


// CDiplomGUIApp:


class CDiplomGUIApp : public CWinApp
{
public:
	CDiplomGUIApp();

public:
	virtual BOOL InitInstance();


	DECLARE_MESSAGE_MAP()
};

extern CDiplomGUIApp theApp;
