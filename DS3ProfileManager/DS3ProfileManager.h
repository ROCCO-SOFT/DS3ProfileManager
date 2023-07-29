#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"

class CDS3ProfileManagerApp : public CWinApp
{
public:
	CDS3ProfileManagerApp();
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CDS3ProfileManagerApp theApp;
