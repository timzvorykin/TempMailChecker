#pragma once

#include <unordered_set>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <afxcmn.h>

// Класс главного диалогового окна
class CTempMailCheckerDlg : public CDialogEx
{
public:
    CTempMailCheckerDlg(CWnd* pParent = nullptr);

    // Данные диалога
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TEMPMAILCHECKER_DIALOG };
#endif

protected:
    // Стандартные члены MFC
    HICON m_hIcon;

    // Сгенерированные функции карты сообщений
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    // =============== ПОЛЬЗОВАТЕЛЬСКИЕ ДАННЫЕ ===============
    std::unordered_set<std::string> m_disposableDomains;

    // =============== ПОЛЬЗОВАТЕЛЬСКИЕ МЕТОДЫ ===============
    void LoadDomainDatabase(const CString& filePath);
    bool IsDisposable(const CString& email) const;
    bool IsValidEmail(const CString& email) const;
    void WriteLineToFile(CFile& file, const CString& str);

    // =============== ЭЛЕМЕНТЫ УПРАВЛЕНИЯ ===============
    CEdit m_editEmail;
    CListCtrl m_listResults;
    CEdit m_editLog;

    // =============== ОБРАБОТЧИКИ КНОПОК ===============
    afx_msg void OnBnClickedBtnCheckSingle();
    afx_msg void OnBnClickedBtnLoadFile();
    afx_msg void OnBnClickedBtnSaveFile();
    afx_msg void OnBnClickedBtnClearAll();
};