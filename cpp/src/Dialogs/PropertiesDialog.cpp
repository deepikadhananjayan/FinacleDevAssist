#include "PropertiesDialog.h"
#include "../PluginDefinition.h"
#include "../DockingFeature/resource.h"
#include "../Configuration/FDAConfig.h"
#include "../Core/FDAApplication.h"
#include "../Utils/Logger.h"
#include <string>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

// -------------------------------------------------------
// Helpers
// -------------------------------------------------------
static std::wstring NarrowToWide(const std::string& s)
{
    if (s.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], size);
    return result;
}

static std::string WideToNarrow(const TCHAR* wide)
{
    if (!wide || wide[0] == L'\0') return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &result[0], size, nullptr, nullptr);
    return result;
}

static void handleError(int errCode, HWND hDlg)
{
    std::wstring errMsg;

    switch (errCode) {
    case 13:
        errMsg = L"Configuration file is not writable.\n\n"
            L"Please restart Notepad++ as Administrator and try again.";
        break;

    case -1:
        errMsg = L"Protected key cannot be added, updated, or deleted.";
        break;

    case 1:
        errMsg = L"Only FI environments can be added. "
            L"The key must start with \"fi.\".";
        break;

    case 2:
        errMsg = L"Key cannot contain only \"fi.\". "
            L"Please provide an environment name.";
        break;

    case 3:
        errMsg = L"Key must not contain \"=\".";
        break;

    case 4:
        errMsg = L"Value cannot be empty.";
        break;

    case 5:
        errMsg = L"Key already exists.";
        break;

    case 6:
        errMsg = L"Key is not editable.";
        break;

    case 7:
        errMsg = L"Key not found.";
        break;

    case 8:
        errMsg = L"Key is not deletable.";
        break;

    default:
        errMsg = L"Unexpected error occurred while updating the properties.";
        break;
    }

    MessageBox(
        hDlg,
        errMsg.c_str(),
        L"Finacle Dev Assist",
        MB_OK | MB_ICONWARNING
    );
}

// -------------------------------------------------------
// Populate ListView
// -------------------------------------------------------
static void PopulateList(HWND hList)
{
    ListView_DeleteAllItems(hList);

    const std::vector<FDAProperty>& props = FDAConfig::getProperties();

    for (int i = 0; i < static_cast<int>(props.size()); ++i)
    {
        const FDAProperty& p = props[i];

        LVITEM lvi     = {};
        lvi.mask       = LVIF_TEXT;
        lvi.iItem      = i;
        lvi.iSubItem   = 0;
        std::wstring wKey = NarrowToWide(p.key);
        lvi.pszText    = const_cast<LPWSTR>(wKey.c_str());
        ListView_InsertItem(hList, &lvi);

        std::wstring wVal = NarrowToWide(p.value);
        ListView_SetItemText(hList, i, 1, const_cast<LPWSTR>(wVal.c_str()));

        std::wstring wPerm = (!p.editable && !p.deletable)
            ? L"Read Only"
            : L"Edit / Delete";
        ListView_SetItemText(hList, i, 2, const_cast<LPWSTR>(wPerm.c_str()));
    }
}

// -------------------------------------------------------
// Update button states based on selected item
// -------------------------------------------------------
static void UpdateButtonStates(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);
    int  sel   = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

    bool editEnabled   = false;
    bool deleteEnabled = false;

    if (sel >= 0)
    {
        const std::vector<FDAProperty>& props = FDAConfig::getProperties();
        if (sel < static_cast<int>(props.size()))
        {
            editEnabled   = props[sel].editable;
            deleteEnabled = props[sel].deletable;
        }
    }

    EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_EDIT),   editEnabled   ? TRUE : FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_DELETE), deleteEnabled ? TRUE : FALSE);
}

// -------------------------------------------------------
// Add / Edit inner dialog
// -------------------------------------------------------
struct PropertyEditParams
{
    std::string key = "";
    std::string value = "";
    bool        keyReadOnly = false;
};

INT_PTR CALLBACK PropertyEditDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH             hDarkBrush  = nullptr;
    static PropertyEditParams* pParams    = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pParams = reinterpret_cast<PropertyEditParams*>(lParam);

        // Icon
        HICON hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        HICON hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(hIconBig));

        // Center
        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right  - pr.left  - (dr.right  - dr.left))  / 2;
        int y = pr.top  + (pr.bottom - pr.top   - (dr.bottom - dr.top))   / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Pre-fill
        if (pParams)
        {
            SetWindowText(GetDlgItem(hDlg, IDC_PROP_KEY),
                NarrowToWide(pParams->key).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_PROP_VALUE),
                NarrowToWide(pParams->value).c_str());

            // When editing, lock the key field
            if (pParams->keyReadOnly)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_PROP_KEY), FALSE);
                SetWindowText(hDlg, TEXT("Edit Property"));
                SetWindowText(GetDlgItem(hDlg, IDC_PROP_KEY),
                    NarrowToWide(pParams->key).c_str());
            }
            else
            {
                SetWindowText(hDlg, TEXT("Add Property"));
                SetWindowText(GetDlgItem(hDlg, IDC_PROP_KEY), TEXT("fi."));
                // Move cursor to end so user types after fi.
                HWND hKey = GetDlgItem(hDlg, IDC_PROP_KEY);
                SendMessage(hKey, EM_SETSEL, 3, 3);
                SetFocus(hKey);
            }
        }

        // Dark mode
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));

        ::SendMessage(nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfInit),
            reinterpret_cast<LPARAM>(hDlg));

        return TRUE;
    }

    case WM_SETTINGCHANGE:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));
        ::SendMessage(nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfHandleChange),
            reinterpret_cast<LPARAM>(hDlg));
        InvalidateRect(hDlg, nullptr, TRUE);
        break;
    }

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    {
        if (hDarkBrush)
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(220, 220, 220));
            SetBkColor(hdc, RGB(37, 37, 38));
            return reinterpret_cast<INT_PTR>(hDarkBrush);
        }
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            TCHAR keyBuf[256] = {};
            TCHAR valueBuf[512] = {};

            GetWindowText(GetDlgItem(hDlg, IDC_PROP_KEY), keyBuf, 256);
            GetWindowText(GetDlgItem(hDlg, IDC_PROP_VALUE), valueBuf, 512);

            std::string key = WideToNarrow(keyBuf);
            std::string value = WideToNarrow(valueBuf);

            auto trim = [](std::string& s) {
                size_t f = s.find_first_not_of(" \t");
                size_t l = s.find_last_not_of(" \t");
                s = (f == std::string::npos) ? "" : s.substr(f, l - f + 1);
                };
            trim(key);
            trim(value);

            if (key.empty() || key == "fi.")
            {
                MessageBox(hDlg,
                    TEXT("Key must start with 'fi.' followed by a name.\nExample: fi.fin1025"),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_PROP_KEY));
                return TRUE;
            }

            if (value.empty())
            {
                MessageBox(hDlg, TEXT("Value cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_PROP_VALUE));
                return TRUE;
            }

            if (pParams)
            {
                pParams->key = key;
                pParams->value = value;
            }

            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_DESTROY:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        break;
    }
    }

    return FALSE;
}

// -------------------------------------------------------
// Main Properties Dialog
// -------------------------------------------------------
INT_PTR CALLBACK PropertiesDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH hDarkBrush = nullptr;
    static HICON  hIconSmall = nullptr;
    static HICON  hIconBig   = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(hIconBig));

        // Center
        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right  - pr.left  - (dr.right  - dr.left))  / 2;
        int y = pr.top  + (pr.bottom - pr.top   - (dr.bottom - dr.top))   / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Setup ListView columns
        HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);

        RECT listRect;
        GetClientRect(hList, &listRect);
        int totalWidth = listRect.right - listRect.left - 4; // -4 for border

        int colValue = totalWidth / 2;        // 50%
        int colProperty = totalWidth / 4;        // 25%
        int colAccess = totalWidth - colValue - colProperty; // 25%

        LVCOLUMN col = {};
        col.mask     = LVCF_TEXT | LVCF_WIDTH;

        col.pszText  = const_cast<LPWSTR>(L"Property");
        col.cx       = colProperty;
        ListView_InsertColumn(hList, 0, &col);

        col.pszText  = const_cast<LPWSTR>(L"Value");
        col.cx       = colValue;
        ListView_InsertColumn(hList, 1, &col);

        col.pszText  = const_cast<LPWSTR>(L"Access");
        col.cx       = colAccess;
        ListView_InsertColumn(hList, 2, &col);

        // Full row select
        ListView_SetExtendedListViewStyle(hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        PopulateList(hList);

        // Start with Edit/Delete disabled
        EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_EDIT),   FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_DELETE), FALSE);

        // Dark mode
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));

        ::SendMessage(nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfInit),
            reinterpret_cast<LPARAM>(hDlg));

        return TRUE;
    }

    case WM_SETTINGCHANGE:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));
        ::SendMessage(nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfHandleChange),
            reinterpret_cast<LPARAM>(hDlg));
        InvalidateRect(hDlg, nullptr, TRUE);
        break;
    }

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    {
        if (hDarkBrush)
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(220, 220, 220));
            SetBkColor(hdc, RGB(37, 37, 38));
            return reinterpret_cast<INT_PTR>(hDarkBrush);
        }
        break;
    }

    case WM_NOTIFY:
    {
        NMHDR* pNmHdr = reinterpret_cast<NMHDR*>(lParam);

        if (pNmHdr->idFrom == IDC_PROPERTIES_LIST)
        {
            switch (pNmHdr->code)
            {
            case LVN_ITEMCHANGED:
                UpdateButtonStates(hDlg);
                break;

            case LVN_BEGINLABELEDIT:
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                return TRUE;

            case NM_DBLCLK:
                return TRUE;
            }
        }
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        // --------------------------------------------------
        // Add
        // --------------------------------------------------
        case IDC_PROPERTIES_ADD:
        {
            PropertyEditParams params;
            params.key         = "";
            params.value       = "";
            params.keyReadOnly = false;

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_PROPERTY_EDIT_DIALOG),
                hDlg,
                PropertyEditDialogProc,
                reinterpret_cast<LPARAM>(&params)
            );

            if (result == IDOK)
            {
                int res = FDAConfig::addProperty(params.key, params.value);
                
                if (res != 0)
                {
                    handleError(res,hDlg);
                }
                else
                {
                    PopulateList(GetDlgItem(hDlg, IDC_PROPERTIES_LIST));
                    UpdateButtonStates(hDlg);
                }
            }

            return TRUE;
        }

        // --------------------------------------------------
        // Edit
        // --------------------------------------------------
        case IDC_PROPERTIES_EDIT:
        {
            HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);
            int  sel   = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

            if (sel < 0) return TRUE;

            const std::vector<FDAProperty>& props = FDAConfig::getProperties();
            if (sel >= static_cast<int>(props.size())) return TRUE;

            const FDAProperty& selected = props[sel];

            if (!selected.editable)
                return TRUE;

            PropertyEditParams params;
            params.key         = selected.key;
            params.value       = selected.value;
            params.keyReadOnly = true; // key cannot change on edit

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_PROPERTY_EDIT_DIALOG),
                hDlg,
                PropertyEditDialogProc,
                reinterpret_cast<LPARAM>(&params)
            );

            if (result == IDOK)
            {
                int res = FDAConfig::updateProperty(params.key, params.value);
                if (res != 0)
                {
                    handleError(res, hDlg);
                }
                else
                {
                    PopulateList(hList);
                    UpdateButtonStates(hDlg);
                }
            }

            return TRUE;
        }

        // --------------------------------------------------
        // Delete
        // --------------------------------------------------
        case IDC_PROPERTIES_DELETE:
        {
            HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);
            int  sel   = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

            if (sel < 0) return TRUE;

            const std::vector<FDAProperty>& props = FDAConfig::getProperties();
            if (sel >= static_cast<int>(props.size())) return TRUE;

            std::string key = props[sel].key;

            std::wstring msg = L"Delete property '" + NarrowToWide(key) + L"'?";

            if (MessageBox(hDlg, msg.c_str(),
                TEXT("Finacle Dev Assist"),
                MB_YESNO | MB_ICONWARNING) == IDYES)
            {
                int res = FDAConfig::deleteProperty(key);
                if (res != 0)
                {
                    handleError(res, hDlg);
                }
                else
                {
                    PopulateList(hList);
                    UpdateButtonStates(hDlg);
                }
            }

            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_DESTROY:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        if (hIconSmall) { DestroyIcon(hIconSmall);  hIconSmall = nullptr; }
        if (hIconBig)   { DestroyIcon(hIconBig);    hIconBig   = nullptr; }
        break;
    }
    }

    return FALSE;
}