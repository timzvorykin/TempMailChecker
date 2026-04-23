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
    // База данных доменов в памяти (хэш-таблица для быстрого поиска)
    std::unordered_set<std::string> m_disposableDomains;

    // =============== ПОЛЬЗОВАТЕЛЬСКИЕ МЕТОДЫ ===============
    // Загрузка списка доменов из текстового файла
    void LoadDomainDatabase(const CString& filePath);

    // Проверка email на принадлежность к временным сервисам
    bool IsDisposable(const CString& email) const;

    // Валидация email (проверка корректности формата)
    bool IsValidEmail(const CString& email) const;

    // Вспомогательная функция для записи строки в файл с UTF-8 кодировкой
    void WriteLineToFile(CFile& file, const CString& str);

    // =============== ЭЛЕМЕНТЫ УПРАВЛЕНИЯ ===============
    CEdit m_editEmail;          // Поле ввода email
    CListCtrl m_listResults;    // Таблица результатов
    CEdit m_editLog;            // Лог операций

    // =============== ОБРАБОТЧИКИ КНОПОК ===============
    afx_msg void OnBnClickedBtnCheckSingle();
    afx_msg void OnBnClickedBtnLoadFile();
    afx_msg void OnBnClickedBtnSaveFile();
    afx_msg void OnBnClickedBtnClearAll();
};