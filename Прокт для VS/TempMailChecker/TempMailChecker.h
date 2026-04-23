
// TempMailChecker.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CTempMailCheckerApp:
// Сведения о реализации этого класса: TempMailChecker.cpp
//

class CTempMailCheckerApp : public CWinApp
{
public:
	CTempMailCheckerApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CTempMailCheckerApp theApp;
