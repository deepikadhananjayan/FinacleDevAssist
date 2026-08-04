#include "ValidationPanel.h"
#include "Docking.h"
#include "../Notepad_plus_msgs.h"
#include "../Utils/Logger.h"
#include "../Features/ScintillaHelper.h"

#include <shellapi.h>
#pragma comment(lib,"shell32.lib")
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

// -----------------------------------------------------------------------------
// Static Member Initialization
// -----------------------------------------------------------------------------

HINSTANCE   ValidationPanel::_hInstance = NULL;
HWND        ValidationPanel::_nppHandle = NULL;
HWND        ValidationPanel::_hPanel = NULL;
HWND        ValidationPanel::_hHeader = NULL;
HWND        ValidationPanel::_hListView = NULL;
HWND        ValidationPanel::_hSummary = NULL;
HWND        ValidationPanel::_hDeveloper = NULL;
HFONT       ValidationPanel::_hFont = NULL;
HFONT       ValidationPanel::_hBoldFont = NULL;
HFONT       ValidationPanel::_hLinkFont = NULL;

constexpr auto FDA_GITHUB_URL = TEXT("https://github.com/santhoshswamyv");

bool ValidationPanel::_isRegistered = false;
std::vector<Issue> ValidationPanel::_issues;

// -----------------------------------------------------------------------------
// Initialize Validation Panel
// Creates UI controls
// -----------------------------------------------------------------------------

void ValidationPanel::init(HINSTANCE hInstance, HWND nppHandle)
{
    
    if (_hPanel != NULL)
    {
        Logger::error("[VALIDATION PANEL] Panel Already initialized");
        return;
    }

    _hInstance = hInstance;
    _nppHandle = nppHandle;

    // Register custom window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = panelProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("FinacleValidationPanel");

    RegisterClassEx(&wc);

    // Main panel window
    _hPanel = CreateWindowEx(
        0,
        TEXT("FinacleValidationPanel"),
        TEXT("Validation Results"),
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 500, 300,
        nppHandle,
        NULL,
        hInstance,
        NULL
    );

    // Header
    _hHeader = CreateWindowEx(
        0,
        TEXT("STATIC"),
        TEXT("[!] Validation Results"),
        WS_CHILD | WS_VISIBLE,
        5, 5, 480, 30,
        _hPanel,
        NULL,
        hInstance,
        NULL
    );

    // List View
    _hListView = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
        5, 40, 480, 200,
        _hPanel,
        NULL,
        hInstance,
        NULL
    );

    ListView_SetExtendedListViewStyle(
        _hListView,
        LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER
    );

    // -------------------------------------------------------------------------
    // Columns
    // -------------------------------------------------------------------------

    LVCOLUMN column = { 0 };
    column.mask = LVCF_TEXT | LVCF_WIDTH;

    // Line
    column.pszText = TEXT("Line");
    column.cx = 70;
    ListView_InsertColumn(_hListView, 0, &column);

    // Severity
    column.pszText = TEXT("Severity");
    column.cx = 100;
    ListView_InsertColumn(_hListView, 1, &column);

    // Message (Width will be adjusted dynamically)
    column.pszText = TEXT("Message");
    column.cx = 200;
    ListView_InsertColumn(_hListView, 2, &column);

    // -------------------------------------------------------------------------
    // Footer
    // -------------------------------------------------------------------------

    _hSummary = CreateWindowEx(
        0,
        TEXT("STATIC"),
        TEXT("Errors : 0       Warnings : 0"),
        WS_CHILD | WS_VISIBLE,
        5, 250, 250, 30,
        _hPanel,
        NULL,
        hInstance,
        NULL
    );

    _hDeveloper = CreateWindowEx(
        0,
        L"STATIC",
        L"Developed by Sandy \xD83D\xDC97",
        WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_NOTIFY,
        300,
        250,
        180,
        30,
        _hPanel,
        (HMENU)1001,
        hInstance,
        NULL
    );

    // -------------------------------------------------------------------------
    // Apply Notepad++ editor font
    // -------------------------------------------------------------------------

    std::wstring fontName = ScintillaHelper::getCurrentEditorFont();
    int          fontSize = ScintillaHelper::getCurrentEditorFontSize();

    _hFont = CreateFont(
        -(fontSize + 2),
        0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        fontName.c_str()
    );

    _hBoldFont = CreateFont(
        -(fontSize + 2),
        0, 0, 0,
        FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        fontName.c_str()
    );

    _hLinkFont = CreateFont(
        -(fontSize + 2),
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        fontName.c_str()
    );

    SendMessage(_hHeader, WM_SETFONT, (WPARAM)_hFont, TRUE);
    SendMessage(_hListView, WM_SETFONT, (WPARAM)_hFont, TRUE);
    SendMessage(_hSummary, WM_SETFONT, (WPARAM)_hBoldFont, TRUE);
    SendMessage(_hDeveloper, WM_SETFONT, (WPARAM)_hLinkFont, TRUE);
    InvalidateRect(
        _hDeveloper,
        NULL,
        TRUE
    );

    Logger::info("[VALIDATION PANEL] Initialized");
}

// -----------------------------------------------------------------------------
// Register panel with Notepad++ docking manager
// -----------------------------------------------------------------------------

void ValidationPanel::registerPanel()
{
    if (_hPanel == NULL)
    {
        Logger::error("[VALIDATION PANEL] Panel not initialized");
        return;
    }

    if (_isRegistered)
    {
        Logger::info("[VALIDATION PANEL] Already registered");
        return;
    }

    DockedWidgetData dwd = { 0 };
    dwd.hClient = _hPanel;
    dwd.pszName = TEXT("Finacle Dev Assist");
    dwd.dlgID = 0;
    dwd.uMask = DWS_DF_CONT_BOTTOM;
    dwd.pszModuleName = TEXT("FinaclePlugin");

    LRESULT result = ::SendMessage(
        _nppHandle,
        NPPM_DMMREGASDCKDLG,
        0,
        (LPARAM)&dwd
    );

    if (result)
    {
        _isRegistered = true;
        Logger::info("[VALIDATION PANEL] Registered");

        ::SendMessage(_nppHandle, NPPM_DMMHIDE, 0, (LPARAM)_hPanel);
    }
    else
    {
        Logger::error("[VALIDATION PANEL] Registration failed");
    }
}

// -----------------------------------------------------------------------------
// Resize panel controls
// Called whenever docking panel size changes
// -----------------------------------------------------------------------------

void ValidationPanel::resize(
    int width,
    int height
)
{
    if (!_hPanel)
        return;

    MoveWindow(_hHeader, 5, 5, width - 10, 30, TRUE);

    MoveWindow(_hListView, 5, 40, width - 10, height - 90, TRUE);

    // Column sizes
    ListView_SetColumnWidth(_hListView, 0, 70);
    ListView_SetColumnWidth(_hListView, 1, 100);

    // Remaining width goes to Message
    ListView_SetColumnWidth(_hListView, 2, width - 200);

    // Footer left
    MoveWindow(_hSummary, 5, height - 40, 250, 30, TRUE);

    // Footer right
    MoveWindow(_hDeveloper, width - 210, height - 40, 200, 30, TRUE);
}

// -----------------------------------------------------------------------------
// Window Procedure
// Handles resize and cleanup
// -----------------------------------------------------------------------------

LRESULT CALLBACK ValidationPanel::panelProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        resize(width, height);
        return 0;
    }

    case WM_ERASEBKGND:
    {
        // Prevent flickering while resizing
        return TRUE;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);

        return 0;
    }

    case WM_DESTROY:
    {
        if (_hFont)
        {
            DeleteObject(_hFont);
            _hFont = NULL;
        }

        if (_hBoldFont)
        {
            DeleteObject(_hBoldFont);
            _hBoldFont = NULL;
        }

        if (_hLinkFont)
        {
            DeleteObject(_hLinkFont);
            _hLinkFont = NULL;
        }
        break;
    }

    case WM_SETCURSOR:
    {
        HWND hwndControl = (HWND)wParam;

        if (hwndControl == _hDeveloper)
        {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
        }

        break;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == 1001 &&
            HIWORD(wParam) == STN_CLICKED)
        {
            ShellExecute(
                NULL,
                TEXT("open"),
                FDA_GITHUB_URL,
                NULL,
                NULL,
                SW_SHOWNORMAL
            );

            return 0;
        }

        break;
    }

    case WM_NOTIFY:
    {
        LPNMHDR hdr = (LPNMHDR)lParam;

        if (hdr->hwndFrom == _hListView &&
            hdr->code == NM_DBLCLK)
        {
            int selectedRow =
                ListView_GetNextItem(
                    _hListView,
                    -1,
                    LVNI_SELECTED
                );


            if (selectedRow >= 0 &&
                selectedRow < (int)_issues.size())
            {
                Issue issue = _issues[selectedRow];

                int lineNumber = issue.line - 1; // Scintilla uses 0 based index

                if (lineNumber < 0)
                {
                    return 0;
                }

                HWND hSci = ScintillaHelper::getCurrentEditor();

                if (hSci)
                {
                    // Scroll to line
                    ::SendMessage(
                        hSci,
                        SCI_ENSUREVISIBLE,
                        lineNumber,
                        0
                    );

                    // Get line start position
                    int start =
                        (int)::SendMessage(
                            hSci,
                            SCI_POSITIONFROMLINE,
                            lineNumber,
                            0
                        );

                    // Get line end position
                    int end =
                        (int)::SendMessage(
                            hSci,
                            SCI_GETLINEENDPOSITION,
                            lineNumber,
                            0
                        );

                    // Highlight complete line
                    ::SendMessage(
                        hSci,
                        SCI_SETSEL,
                        start,
                        end
                    );

                    // Move focus to editor
                    ::SendMessage(
                        hSci,
                        SCI_GRABFOCUS,
                        0,
                        0
                    );
                }
                else
                {
                    Logger::error("[VALIDATION PANEL] Scintilla handle not found");
                }
            }
            return 0;
        }
        break;
    }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

// -----------------------------------------------------------------------------
// Clear previous validation results
// -----------------------------------------------------------------------------

void ValidationPanel::clear()
{
    if (_hListView)
    {
        ListView_DeleteAllItems(_hListView);
    }

    SetWindowText(
        _hSummary,
        TEXT("Errors : 0       Warnings : 0")
    );
}

// -----------------------------------------------------------------------------
// Display Validation Result
// -----------------------------------------------------------------------------

void ValidationPanel::showValidationResults(const ValidationResult& result)
{
    if (_hListView == NULL)
        return;

    _issues.clear();
    ListView_DeleteAllItems(_hListView);

    if (result.excpOccr)
    {
        MessageBox(
            _nppHandle,
            TEXT("Unexpected Error Occured."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    int index = 0;

    // ---------------------------------------------------------
    // Errors
    // ---------------------------------------------------------

    for (const auto& issue : result.errors)
    {
        _issues.push_back(issue);

        LVITEM item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = index;

        std::wstring line = std::to_wstring(issue.line);
        item.pszText = const_cast<wchar_t*>(line.c_str());

        ListView_InsertItem(_hListView, &item);

        std::wstring severity = L"ERROR";
        ListView_SetItemText(
            _hListView,
            index,
            1,
            const_cast<wchar_t*>(severity.c_str())
        );

        std::wstring message(issue.message.begin(), issue.message.end());
        ListView_SetItemText(
            _hListView,
            index,
            2,
            const_cast<wchar_t*>(message.c_str())
        );

        index++;
    }

    // ---------------------------------------------------------
    // Warnings
    // ---------------------------------------------------------

    for (const auto& issue : result.warnings)
    {
        _issues.push_back(issue);

        LVITEM item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = index;

        std::wstring line = std::to_wstring(issue.line);
        item.pszText = const_cast<wchar_t*>(line.c_str());

        ListView_InsertItem(_hListView, &item);

        std::wstring severity = L"WARNING";
        ListView_SetItemText(
            _hListView,
            index,
            1,
            const_cast<wchar_t*>(severity.c_str())
        );

        std::wstring message(issue.message.begin(), issue.message.end());
        ListView_SetItemText(
            _hListView,
            index,
            2,
            const_cast<wchar_t*>(message.c_str())
        );

        index++;
    }

    // Update summary
    int errorCount = static_cast<int>(result.errors.size());
    int warningCount = static_cast<int>(result.warnings.size());

    std::wstring summary =
        L"Errors : " + std::to_wstring(errorCount) +
        L"       Warnings : " + std::to_wstring(warningCount);


    SetWindowText(
        _hSummary,
        summary.c_str()
    );


    // ---------------------------------------------------------
    // Show panel
    // ---------------------------------------------------------

    ::SendMessage(
        _nppHandle,
        NPPM_DMMSHOW,
        0,
        (LPARAM)_hPanel
    );
}
void ValidationPanel::hidePanel()
{
    Logger::info("[VALIDATION PANEL] Hiding panel");

    // Hide docking panel
    if (_nppHandle && _hPanel)
    {
        ::SendMessage(_nppHandle, NPPM_DMMHIDE, 0, (LPARAM)_hPanel);
    }

    // Clear current validation result
    clear();

    Logger::info("[VALIDATION PANEL] Disabled successfully");
}


void ValidationPanel::destroyPanel()
{
    Logger::info("[VALIDATION PANEL] Destroying resources");

    // Release font resource
    if (_hFont)
    {
        DeleteObject(_hFont);
        _hFont = NULL;
    }

    if (_hBoldFont)
    {
        DeleteObject(_hBoldFont);
        _hBoldFont = NULL;
    }

    if (_hLinkFont)
    {
        DeleteObject(_hLinkFont);
        _hLinkFont = NULL;
    }

    _hPanel = NULL;
    _hHeader = NULL;
    _hListView = NULL;
    _hSummary = NULL;
    _hDeveloper = NULL;
    _nppHandle = NULL;

    Logger::info("[VALIDATION PANEL] Resources released");
}

// -----------------------------------------------------------------------------
// Get panel handle
// -----------------------------------------------------------------------------

HWND ValidationPanel::getPanel()
{
    return _hPanel;
}