#include "pch.h"
#include "framework.h"
#include "TempMailChecker.h"
#include "TempMailCheckerDlg.h"
#include "resource.h"
#include <afxdialogex.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// =============== КАРТА СООБЩЕНИЙ ===============
BEGIN_MESSAGE_MAP(CTempMailCheckerDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_CHECK_SINGLE, &CTempMailCheckerDlg::OnBnClickedBtnCheckSingle)
    ON_BN_CLICKED(IDC_BTN_LOAD_FILE, &CTempMailCheckerDlg::OnBnClickedBtnLoadFile)
    ON_BN_CLICKED(IDC_BTN_SAVE_FILE, &CTempMailCheckerDlg::OnBnClickedBtnSaveFile)
    ON_BN_CLICKED(IDC_BTN_CLEAR_ALL, &CTempMailCheckerDlg::OnBnClickedBtnClearAll)
END_MESSAGE_MAP()

// =============== КОНСТРУКТОР ===============
CTempMailCheckerDlg::CTempMailCheckerDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_TEMPMAILCHECKER_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// =============== ОБМЕН ДАННЫМИ ===============
void CTempMailCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_EMAIL, m_editEmail);
    DDX_Control(pDX, IDC_LIST_RESULTS, m_listResults);
    DDX_Control(pDX, IDC_EDIT_LOG, m_editLog);
}

// =============== ИНИЦИАЛИЗАЦИЯ ДИАЛОГА ===============
BOOL CTempMailCheckerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Установка иконок
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    // Настройка колонок таблицы результатов
    m_listResults.InsertColumn(0, _T("Email"), LVCFMT_LEFT, 300);
    m_listResults.InsertColumn(1, _T("Статус"), LVCFMT_LEFT, 240);
    m_listResults.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Загрузка базы временных доменов из файла рядом с EXE
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    CString strPath(szPath);
    int pos = strPath.ReverseFind('\\');
    CString strDir = strPath.Left(pos);

    LoadDomainDatabase(strDir + _T("\\temp_domains.txt"));

    return TRUE;
}

// =============== СИСТЕМНЫЕ ОБРАБОТЧИКИ ===============
void CTempMailCheckerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    CDialogEx::OnSysCommand(nID, lParam);
}

void CTempMailCheckerDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR CTempMailCheckerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// =============== ЗАГРУЗКА БАЗЫ ДОМЕНОВ ИЗ ФАЙЛА ===============
void CTempMailCheckerDlg::LoadDomainDatabase(const CString& filePath)
{
    m_disposableDomains.clear();

    CT2CA pszConvertedAnsiString(filePath);
    std::string pathStr(pszConvertedAnsiString);

    std::ifstream file(pathStr);
    if (!file.is_open())
    {
        CString logMsg;
        logMsg.Format(_T("[ОШИБКА] Файл базы не найден: %s\r\n"), (LPCTSTR)filePath);
        m_editLog.SetWindowText(logMsg);
        return;
    }

    std::string line;
    int loadedCount = 0;

    while (std::getline(file, line))
    {
        // Удаляем пробелы и спецсимволы вручную
        std::string cleanLine;
        for (char ch : line)
        {
            if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
                cleanLine += ch;
        }

        if (!cleanLine.empty() && cleanLine[0] != '#')
        {
            // Приводим к нижнему регистру (безопасно для ASCII)
            for (char& ch : cleanLine)
            {
                if (ch >= 'A' && ch <= 'Z')
                    ch = ch + ('a' - 'A');
            }
            m_disposableDomains.insert(cleanLine);
            loadedCount++;
        }
    }
    file.close();

    CString logMsg;
    logMsg.Format(_T("[OK] Загружено %d временных доменов.\r\n"), loadedCount);
    m_editLog.SetWindowText(logMsg);
}

// =============== БЕЗОПАСНЫЕ ФУНКЦИИ ПРОВЕРКИ СИМВОЛОВ ===============
static bool IsAsciiLetter(wchar_t ch)
{
    return (ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z');
}

static bool IsAsciiDigit(wchar_t ch)
{
    return (ch >= L'0' && ch <= L'9');
}

static bool IsAsciiLetterOrDigit(wchar_t ch)
{
    return IsAsciiLetter(ch) || IsAsciiDigit(ch);
}

// =============== ВАЛИДАЦИЯ EMAIL ===============
bool CTempMailCheckerDlg::IsValidEmail(const CString& email) const
{
    CString str = email;
    str.Trim();

    if (str.IsEmpty())
        return false;

    // Проверка на ASCII (не Unicode)
    for (int i = 0; i < str.GetLength(); i++)
    {
        if (str[i] > 127)
            return false;
    }

    // Поиск символа @
    int atPos = str.Find(_T('@'));
    if (atPos <= 0 || atPos == str.GetLength() - 1)
        return false;

    // Проверка, что @ только один
    if (str.Find(_T('@'), atPos + 1) != -1)
        return false;

    // Локальная часть (до @)
    CString localPart = str.Left(atPos);
    if (localPart.IsEmpty() || localPart.GetLength() > 64)
        return false;

    for (int i = 0; i < localPart.GetLength(); i++)
    {
        wchar_t ch = localPart[i];
        if (!IsAsciiLetterOrDigit(ch) && ch != L'.' && ch != L'-' && ch != L'_' && ch != L'+')
            return false;
    }

    if (localPart[0] == L'.' || localPart[localPart.GetLength() - 1] == L'.')
        return false;

    // Доменная часть (после @)
    CString domain = str.Mid(atPos + 1);
    if (domain.IsEmpty() || domain.GetLength() > 255)
        return false;

    int dotPos = domain.Find(_T('.'));
    if (dotPos <= 0 || dotPos == domain.GetLength() - 1)
        return false;

    for (int i = 0; i < domain.GetLength(); i++)
    {
        wchar_t ch = domain[i];
        if (!IsAsciiLetterOrDigit(ch) && ch != L'.' && ch != L'-')
            return false;
    }

    if (domain[0] == L'-' || domain[0] == L'.' ||
        domain[domain.GetLength() - 1] == L'-' || domain[domain.GetLength() - 1] == L'.')
        return false;

    // TLD (последняя часть после точки)
    int lastDotPos = domain.ReverseFind(_T('.'));
    if (lastDotPos != -1)
    {
        CString tld = domain.Mid(lastDotPos + 1);
        if (tld.GetLength() < 2)
            return false;

        for (int i = 0; i < tld.GetLength(); i++)
        {
            if (!IsAsciiLetter(tld[i]))
                return false;
        }
    }

    // Нет двух точек подряд
    if (str.Find(_T("..")) != -1)
        return false;

    return true;
}

// =============== ПРОВЕРКА EMAIL НА ВРЕМЕННОСТЬ ===============
bool CTempMailCheckerDlg::IsDisposable(const CString& email) const
{
    int atPos = email.Find(_T('@'));
    if (atPos == -1)
        return false;

    CString domain = email.Mid(atPos + 1);
    domain.MakeLower();

    CT2CA pszConverted(domain);
    std::string strDomain(pszConverted);

    return m_disposableDomains.find(strDomain) != m_disposableDomains.end();
}

// =============== ЗАПИСЬ СТРОКИ В UTF-8 ===============
void CTempMailCheckerDlg::WriteLineToFile(CFile& file, const CString& str)
{
    int nLength = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (nLength > 0)
    {
        char* pBuffer = new char[nLength];
        WideCharToMultiByte(CP_UTF8, 0, str, -1, pBuffer, nLength, NULL, NULL);
        file.Write(pBuffer, nLength - 1);
        file.Write("\r\n", 2);
        delete[] pBuffer;
    }
}

// =============== КНОПКА "ПРОВЕРИТЬ" ===============
void CTempMailCheckerDlg::OnBnClickedBtnCheckSingle()
{
    CString email;
    m_editEmail.GetWindowText(email);
    email.Trim();

    if (email.IsEmpty())
    {
        MessageBox(_T("Введите email для проверки!"), _T("Внимание"), MB_OK | MB_ICONINFORMATION);
        return;
    }

    if (!IsValidEmail(email))
    {
        CString errorMsg;
        errorMsg.Format(_T("Строка \"%s\" не является корректным email-адресом!\n\n")
            _T("Email должен иметь формат: name@domain.com"),
            (LPCTSTR)email);
        MessageBox(errorMsg, _T("Некорректный email"), MB_OK | MB_ICONWARNING);

        CString logMsg;
        m_editLog.GetWindowText(logMsg);
        logMsg.AppendFormat(_T("❌ Некорректный формат: %s\r\n"), (LPCTSTR)email);
        m_editLog.SetWindowText(logMsg);
        return;
    }

    bool isTemp = IsDisposable(email);
    CString status = isTemp ? _T("❌ НЕНАДЕЖНЫЙ (временный)") : _T("✅ ОК (постоянный)");

    int index = m_listResults.InsertItem(0, email);
    m_listResults.SetItemText(index, 1, status);

    CString logMsg;
    m_editLog.GetWindowText(logMsg);
    logMsg.AppendFormat(_T("✅ Проверен: %s → %s\r\n"), (LPCTSTR)email, (LPCTSTR)status);
    m_editLog.SetWindowText(logMsg);

    m_editEmail.SetWindowText(_T(""));
    m_editEmail.SetFocus();
}

// =============== КНОПКА "ЗАГРУЗИТЬ СПИСОК" ===============
void CTempMailCheckerDlg::OnBnClickedBtnLoadFile()
{
    CFileDialog dlg(TRUE,
        _T("*.txt"),
        NULL,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST,
        _T("Text Files (*.txt)|*.txt|CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||"),
        this);

    if (dlg.DoModal() == IDOK)
    {
        CString filePath = dlg.GetPathName();
        m_listResults.DeleteAllItems();

        CStdioFile file;
        CFileException fileException;

        if (!file.Open(filePath, CFile::modeRead | CFile::shareDenyWrite, &fileException))
        {
            TCHAR szError[1024];
            fileException.GetErrorMessage(szError, 1024);
            CString errorMsg;
            errorMsg.Format(_T("Не удалось открыть файл!\n%s"), (LPCTSTR)szError);
            MessageBox(errorMsg, _T("Ошибка"), MB_OK | MB_ICONERROR);
            return;
        }

        CString line;
        int processedCount = 0;
        int tempCount = 0;
        int invalidCount = 0;

        while (file.ReadString(line))
        {
            line.Trim();
            if (!line.IsEmpty())
            {
                if (!IsValidEmail(line))
                {
                    int index = m_listResults.InsertItem(m_listResults.GetItemCount(), line);
                    m_listResults.SetItemText(index, 1, _T("⚠️ НЕКОРРЕКТНЫЙ ФОРМАТ"));
                    invalidCount++;
                    continue;
                }

                bool isTemp = IsDisposable(line);
                if (isTemp) tempCount++;

                CString status = isTemp ? _T("❌ НЕНАДЕЖНЫЙ") : _T("✅ ОК");
                int index = m_listResults.InsertItem(m_listResults.GetItemCount(), line);
                m_listResults.SetItemText(index, 1, status);
                processedCount++;
            }
        }
        file.Close();

        CString logMsg;
        logMsg.Format(_T("[OK] Файл обработан: %s\r\n"), (LPCTSTR)filePath);

        CString temp;
        temp.Format(_T("     Корректных адресов: %d\r\n"), processedCount);
        logMsg += temp;
        temp.Format(_T("     Из них временных: %d (%.1f%%)\r\n"),
            tempCount, processedCount > 0 ? (tempCount * 100.0 / processedCount) : 0.0);
        logMsg += temp;
        if (invalidCount > 0)
        {
            temp.Format(_T("     Некорректных строк: %d\r\n"), invalidCount);
            logMsg += temp;
        }
        m_editLog.SetWindowText(logMsg);

        CString resultMsg;
        resultMsg.Format(_T("Обработка завершена!\n\n")
            _T("Корректных адресов: %d\nНекорректных строк: %d\nВременных адресов: %d"),
            processedCount, invalidCount, tempCount);
        MessageBox(resultMsg, _T("Результат загрузки"), MB_OK | MB_ICONINFORMATION);
    }
}

// =============== КНОПКА "СОХРАНИТЬ РЕЗУЛЬТАТ" ===============
void CTempMailCheckerDlg::OnBnClickedBtnSaveFile()
{
    if (m_listResults.GetItemCount() == 0)
    {
        MessageBox(_T("Нет данных для сохранения!"), _T("Внимание"), MB_OK | MB_ICONINFORMATION);
        return;
    }

    CFileDialog dlg(FALSE,
        _T("txt"),
        _T("email_check_report"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
        this);

    if (dlg.DoModal() == IDOK)
    {
        CString filePath = dlg.GetPathName();
        if (dlg.GetFileExt().IsEmpty())
            filePath += _T(".txt");

        CFile file;
        CFileException fileException;

        if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &fileException))
        {
            TCHAR szError[1024];
            fileException.GetErrorMessage(szError, 1024);
            CString errorMsg;
            errorMsg.Format(_T("Не удалось создать файл!\nПуть: %s\nОшибка: %s"),
                (LPCTSTR)filePath, (LPCTSTR)szError);
            MessageBox(errorMsg, _T("Ошибка сохранения"), MB_OK | MB_ICONERROR);
            return;
        }

        try
        {
            unsigned char utf8bom[3] = { 0xEF, 0xBB, 0xBF };
            file.Write(utf8bom, 3);

            WriteLineToFile(file, _T("================================================================================"));
            WriteLineToFile(file, _T("                    ОТЧЁТ ПРОВЕРКИ EMAIL АДРЕСОВ"));
            WriteLineToFile(file, _T("================================================================================"));
            WriteLineToFile(file, _T(""));

            CTime currentTime = CTime::GetCurrentTime();
            CString dateStr;
            dateStr.Format(_T("Дата создания: %02d.%02d.%d %02d:%02d:%02d"),
                currentTime.GetDay(), currentTime.GetMonth(), currentTime.GetYear(),
                currentTime.GetHour(), currentTime.GetMinute(), currentTime.GetSecond());
            WriteLineToFile(file, dateStr);
            WriteLineToFile(file, _T("--------------------------------------------------------------------------------"));
            WriteLineToFile(file, _T(""));

            int count = m_listResults.GetItemCount();
            int tempCount = 0, invalidCount = 0, validCount = 0;

            WriteLineToFile(file, _T("№    Email                                         Статус"));
            WriteLineToFile(file, _T("---  --------------------------------------------  ------------------------------"));

            for (int i = 0; i < count; i++)
            {
                CString email = m_listResults.GetItemText(i, 0);
                CString statusRaw = m_listResults.GetItemText(i, 1);
                CString statusText;

                if (statusRaw.Find(_T("⚠️")) != -1 || statusRaw.Find(_T("НЕКОРРЕКТНЫЙ")) != -1)
                {
                    statusText = _T("⚠️ НЕКОРРЕКТНЫЙ ФОРМАТ");
                    invalidCount++;
                }
                else if (statusRaw.Find(_T("❌")) != -1 || statusRaw.Find(_T("НЕНАДЕЖНЫЙ")) != -1)
                {
                    statusText = _T("❌ НЕНАДЕЖНЫЙ (временный сервис)");
                    tempCount++;
                    validCount++;
                }
                else
                {
                    statusText = _T("✅ ОК (постоянный адрес)");
                    validCount++;
                }

                CString line;
                line.Format(_T("%-4d  %-45s %s"), i + 1, (LPCTSTR)email, (LPCTSTR)statusText);
                WriteLineToFile(file, line);
            }

            WriteLineToFile(file, _T(""));
            WriteLineToFile(file, _T("--------------------------------------------------------------------------------"));
            WriteLineToFile(file, _T("                               СТАТИСТИКА"));
            WriteLineToFile(file, _T("--------------------------------------------------------------------------------"));
            WriteLineToFile(file, _T(""));

            CString stats;
            stats.Format(_T("Всего записей:                   %d"), count);
            WriteLineToFile(file, stats);
            stats.Format(_T("Корректных адресов:              %d"), validCount);
            WriteLineToFile(file, stats);
            stats.Format(_T("Некорректных записей:            %d"), invalidCount);
            WriteLineToFile(file, stats);
            WriteLineToFile(file, _T(""));
            WriteLineToFile(file, _T("Из корректных адресов:"));
            stats.Format(_T("  Временных (ненадёжных):        %d  (%.1f%%)"),
                tempCount, validCount > 0 ? (tempCount * 100.0 / validCount) : 0.0);
            WriteLineToFile(file, stats);
            stats.Format(_T("  Постоянных (надёжных):         %d  (%.1f%%)"),
                validCount - tempCount,
                validCount > 0 ? ((validCount - tempCount) * 100.0 / validCount) : 0.0);
            WriteLineToFile(file, stats);
            WriteLineToFile(file, _T(""));
            WriteLineToFile(file, _T("================================================================================"));
            WriteLineToFile(file, _T("                         КОНЕЦ ОТЧЁТА"));
            WriteLineToFile(file, _T("================================================================================"));

            file.Close();

            CString successMsg;
            successMsg.Format(_T("Отчёт успешно сохранен!\n\nФайл: %s\n\n")
                _T("Всего записей: %d\nКорректных: %d\nНекорректных: %d\nВременных: %d\nПостоянных: %d"),
                (LPCTSTR)filePath, count, validCount, invalidCount, tempCount, validCount - tempCount);
            MessageBox(successMsg, _T("Сохранение завершено"), MB_OK | MB_ICONINFORMATION);

            CString logMsg;
            m_editLog.GetWindowText(logMsg);
            logMsg.AppendFormat(_T("[OK] Отчёт сохранен: %s (Записей: %d)\r\n"), (LPCTSTR)filePath, count);
            m_editLog.SetWindowText(logMsg);
        }
        catch (...)
        {
            MessageBox(_T("Ошибка при сохранении файла!"), _T("Ошибка"), MB_OK | MB_ICONERROR);
            if (file.m_hFile != CFile::hFileNull)
                file.Close();
        }
    }
}

// =============== КНОПКА "ОЧИСТИТЬ ВСЁ" ===============
void CTempMailCheckerDlg::OnBnClickedBtnClearAll()
{
    if (m_listResults.GetItemCount() > 0)
    {
        if (MessageBox(_T("Очистить все результаты и поле ввода?"),
            _T("Подтверждение"), MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
    }

    m_editEmail.SetWindowText(_T(""));
    m_listResults.DeleteAllItems();

    CString logMsg;
    m_editLog.GetWindowText(logMsg);
    CTime currentTime = CTime::GetCurrentTime();
    CString timeStr = currentTime.Format(_T("%H:%M:%S"));
    logMsg.AppendFormat(_T("[%s] Очистка выполнена.\r\n"), (LPCTSTR)timeStr);
    m_editLog.SetWindowText(logMsg);
    m_editEmail.SetFocus();
}