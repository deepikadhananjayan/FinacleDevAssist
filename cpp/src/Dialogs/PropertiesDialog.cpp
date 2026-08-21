#include "PropertiesDialog.h"
#include "../PluginDefinition.h"
#include "../DockingFeature/resource.h"
#include "../Configuration/FDAConfig.h"
#include "../Core/FDAApplication.h"
#include "../Utils/Logger.h"
#include "../Utils/StringConvert.h"
#include <string>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

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

    case 9:
        errMsg = L"Environment name cannot be empty.";
        break;

    case 10:
        errMsg = L"Environment name cannot contain '=' or '.'.";
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
struct DisplayRow
{
    bool isC24Group = false;
    std::string identifier;
};

static std::vector<DisplayRow> gDisplayRows;

static void PopulateList(HWND hList)
{
    ListView_DeleteAllItems(hList);
    gDisplayRows.clear();

    const std::vector<FDAProperty>& props = FDAConfig::getProperties();
    std::vector<std::string> c24Names = FDAConfig::getC24EnvironmentNames();

    int row = 0;

    for (const FDAProperty& p : props)
    {
        if (FDAConfig::isC24EnvironmentKey(p.key))
            continue;

        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = row;
        lvi.iSubItem = 0;
        std::wstring wKey = NarrowToWide(p.key);
        lvi.pszText = const_cast<LPWSTR>(wKey.c_str());
        ListView_InsertItem(hList, &lvi);

        std::wstring wVal = NarrowToWide(p.value);
        ListView_SetItemText(hList, row, 1, const_cast<LPWSTR>(wVal.c_str()));

        std::wstring wPerm = (!p.editable && !p.deletable) ? L"Read Only" : (!p.deletable) ? L"Edit" : L"Edit / Delete";
        ListView_SetItemText(hList, row, 2, const_cast<LPWSTR>(wPerm.c_str()));

        gDisplayRows.push_back({ false, p.key });
        ++row;
    }

    for (const std::string& name : c24Names)
    {
        FDAC24Environment env;
        bool decryptFailed = false; // unused here — summary row never shows username/password
        FDAConfig::getC24Environment(name, env, decryptFailed);

        std::wstring label = L"c24." + NarrowToWide(name);

        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = row;
        lvi.iSubItem = 0;
        lvi.pszText = const_cast<LPWSTR>(label.c_str());
        ListView_InsertItem(hList, &lvi);

        std::wstring summary = NarrowToWide(env.host) + L":" + NarrowToWide(env.port);
        ListView_SetItemText(hList, row, 1, const_cast<LPWSTR>(summary.c_str()));
        ListView_SetItemText(hList, row, 2, const_cast<LPWSTR>(L"Edit / Delete"));

        gDisplayRows.push_back({ true, name });
        ++row;
    }
}

// -------------------------------------------------------
// Update button states based on selected item
// -------------------------------------------------------
static void UpdateButtonStates(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);
    int  sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

    bool editEnabled = false;
    bool deleteEnabled = false;

    if (sel >= 0 && sel < static_cast<int>(gDisplayRows.size()))
    {
        const DisplayRow& row = gDisplayRows[sel];

        if (row.isC24Group)
        {
            editEnabled = true;
            deleteEnabled = true;
        }
        else
        {
            const std::vector<FDAProperty>& props = FDAConfig::getProperties();
            for (const FDAProperty& p : props)
            {
                if (p.key == row.identifier)
                {
                    editEnabled = p.editable;
                    deleteEnabled = p.deletable;
                    break;
                }
            }
        }
    }

    EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_EDIT), editEnabled ? TRUE : FALSE);
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
    static HBRUSH             hDarkBrush = nullptr;
    static PropertyEditParams* pParams = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pParams = reinterpret_cast<PropertyEditParams*>(lParam);

        HICON hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        HICON hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));

        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right - pr.left - (dr.right - dr.left)) / 2;
        int y = pr.top + (pr.bottom - pr.top - (dr.bottom - dr.top)) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        if (pParams)
        {
            SetWindowText(GetDlgItem(hDlg, IDC_PROP_VALUE),
                NarrowToWide(pParams->value).c_str());

            HWND hKey = GetDlgItem(hDlg, IDC_PROP_KEY);
            HWND hPrefix = GetDlgItem(hDlg, IDC_PROP_KEY_PREFIX);

            RECT editFull = { 55, 12, 260, 26 };
            RECT editSuffix = { 78, 12, 260, 26 };
            MapDialogRect(hDlg, &editFull);
            MapDialogRect(hDlg, &editSuffix);

            if (pParams->keyReadOnly)
            {
                SetWindowText(hPrefix, TEXT(""));
                SetWindowPos(hKey, nullptr, editFull.left, editFull.top,
                    editFull.right - editFull.left, editFull.bottom - editFull.top,
                    SWP_NOZORDER);
                SetWindowText(hKey, NarrowToWide(pParams->key).c_str());
                EnableWindow(hKey, FALSE);
                SetWindowText(hDlg, TEXT("Edit Property"));
            }
            else
            {
                SetWindowText(hPrefix, TEXT("fi."));
                SetWindowPos(hKey, nullptr, editSuffix.left, editSuffix.top,
                    editSuffix.right - editSuffix.left, editSuffix.bottom - editSuffix.top,
                    SWP_NOZORDER);
                SetWindowText(hKey, TEXT(""));
                SetWindowText(hDlg, TEXT("Add FI Environment"));
                SetFocus(hKey);
            }
        }

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

            std::string typedKey = WideToNarrow(keyBuf);
            std::string value = WideToNarrow(valueBuf);

            auto trim = [](std::string& s) {
                size_t f = s.find_first_not_of(" \t");
                size_t l = s.find_last_not_of(" \t");
                s = (f == std::string::npos) ? "" : s.substr(f, l - f + 1);
                };
            trim(typedKey);
            trim(value);

            std::string key;

            if (pParams && !pParams->keyReadOnly)
            {
                if (typedKey.empty())
                {
                    MessageBox(hDlg,
                        TEXT("Please provide an environment name.\nExample: fin1025"),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    SetFocus(GetDlgItem(hDlg, IDC_PROP_KEY));
                    return TRUE;
                }

                if (typedKey.find('=') != std::string::npos)
                {
                    MessageBox(hDlg, TEXT("Name must not contain '='."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    SetFocus(GetDlgItem(hDlg, IDC_PROP_KEY));
                    return TRUE;
                }

                key = "fi." + typedKey;
            }
            else
            {
                key = typedKey;
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

struct C24EnvEditParams
{
    FDAC24Environment env;
    bool nameReadOnly = false;
};

INT_PTR CALLBACK C24EnvEditDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH             hDarkBrush = nullptr;
    static C24EnvEditParams* pParams = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pParams = reinterpret_cast<C24EnvEditParams*>(lParam);

        HICON hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        HICON hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));

        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right - pr.left - (dr.right - dr.left)) / 2;
        int y = pr.top + (pr.bottom - pr.top - (dr.bottom - dr.top)) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        if (pParams)
        {
            SetWindowText(GetDlgItem(hDlg, IDC_C24_NAME), NarrowToWide(pParams->env.name).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_HOST), NarrowToWide(pParams->env.host).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_PORT), NarrowToWide(pParams->env.port).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_USERNAME), NarrowToWide(pParams->env.username).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_PASSWORD), NarrowToWide(pParams->env.password).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_BANKID), NarrowToWide(pParams->env.bankId).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_BEPATH), NarrowToWide(pParams->env.bePath).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_C24_FEPATH), NarrowToWide(pParams->env.fePath).c_str());

            if (pParams->nameReadOnly)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_C24_NAME), FALSE);
                SetWindowText(hDlg, TEXT("Edit C24 Environment"));
            }
            else
            {
                SetWindowText(hDlg, TEXT("Add C24 Environment"));
                SetFocus(GetDlgItem(hDlg, IDC_C24_NAME));
            }
        }

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
            TCHAR nameBuf[128] = {};
            TCHAR hostBuf[256] = {};
            TCHAR portBuf[32] = {};
            TCHAR userBuf[128] = {};
            TCHAR passBuf[128] = {};
            TCHAR bankBuf[64] = {};
            TCHAR beBuf[512] = {};
            TCHAR feBuf[512] = {};

            GetWindowText(GetDlgItem(hDlg, IDC_C24_NAME), nameBuf, 128);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_HOST), hostBuf, 256);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_PORT), portBuf, 32);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_USERNAME), userBuf, 128);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_PASSWORD), passBuf, 128);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_BANKID), bankBuf, 64);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_BEPATH), beBuf, 512);
            GetWindowText(GetDlgItem(hDlg, IDC_C24_FEPATH), feBuf, 512);

            auto trim = [](std::string& s) {
                size_t f = s.find_first_not_of(" \t");
                size_t l = s.find_last_not_of(" \t");
                s = (f == std::string::npos) ? "" : s.substr(f, l - f + 1);
                };

            FDAC24Environment env;
            env.name = WideToNarrow(nameBuf);
            env.host = WideToNarrow(hostBuf);
            env.port = WideToNarrow(portBuf);
            env.username = WideToNarrow(userBuf);
            env.password = WideToNarrow(passBuf);
            env.bankId = WideToNarrow(bankBuf);
            env.bePath = WideToNarrow(beBuf);
            env.fePath = WideToNarrow(feBuf);

            trim(env.name); trim(env.host); trim(env.port); trim(env.username);
            trim(env.password); trim(env.bankId); trim(env.bePath); trim(env.fePath);

            if (env.name.empty())
            {
                MessageBox(hDlg, TEXT("Environment name cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_C24_NAME));
                return TRUE;
            }

            if (env.name.find('=') != std::string::npos || env.name.find('.') != std::string::npos)
            {
                MessageBox(hDlg, TEXT("Environment name cannot contain '=' or '.'."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_C24_NAME));
                return TRUE;
            }

            if (pParams && !pParams->nameReadOnly)
            {
                std::vector<std::string> existingNames = FDAConfig::getC24EnvironmentNames();
                for (const std::string& n : existingNames)
                {
                    if (n == env.name)
                    {
                        MessageBox(hDlg,
                            (L"Environment '" + NarrowToWide(env.name) + L"' already exists.\n"
                                L"Please choose a different name.").c_str(),
                            TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                        SetFocus(GetDlgItem(hDlg, IDC_C24_NAME));
                        return TRUE;
                    }
                }
            }

            if (env.host.empty() || env.port.empty() || env.username.empty() ||
                env.password.empty() || env.bankId.empty() || env.bePath.empty() || env.fePath.empty())
            {
                MessageBox(hDlg, TEXT("All fields must be filled in."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            for (char c : env.port)
            {
                if (!isdigit(static_cast<unsigned char>(c)))
                {
                    MessageBox(hDlg, TEXT("Port must contain numbers only."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    SetFocus(GetDlgItem(hDlg, IDC_C24_PORT));
                    return TRUE;
                }
            }

            if (pParams)
                pParams->env = env;

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
    static HICON  hIconBig = nullptr;

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
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));

        // Center
        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right - pr.left - (dr.right - dr.left)) / 2;
        int y = pr.top + (pr.bottom - pr.top - (dr.bottom - dr.top)) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);

        RECT listRect;
        GetClientRect(hList, &listRect);
        int totalWidth = listRect.right - listRect.left - 4;

        int colValue = totalWidth / 2;
        int colProperty = totalWidth / 4;
        int colAccess = totalWidth - colValue - colProperty;

        LVCOLUMN col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH;

        col.pszText = const_cast<LPWSTR>(L"Property");
        col.cx = colProperty;
        ListView_InsertColumn(hList, 0, &col);

        col.pszText = const_cast<LPWSTR>(L"Value");
        col.cx = colValue;
        ListView_InsertColumn(hList, 1, &col);

        col.pszText = const_cast<LPWSTR>(L"Access");
        col.cx = colAccess;
        ListView_InsertColumn(hList, 2, &col);

        ListView_SetExtendedListViewStyle(hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        PopulateList(hList);

        EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_EDIT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES_DELETE), FALSE);

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
        // Add (FI environment)
        // --------------------------------------------------
        case IDC_PROPERTIES_ADD:
        {
            PropertyEditParams params;
            params.key = "";
            params.value = "";
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
                    handleError(res, hDlg);
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
        // Add C24 Environment
        // --------------------------------------------------
        case IDC_PROPERTIES_ADD_C24:
        {
            C24EnvEditParams params;
            params.nameReadOnly = false;

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_C24_ENV_EDIT_DIALOG),
                hDlg,
                C24EnvEditDialogProc,
                reinterpret_cast<LPARAM>(&params)
            );

            if (result == IDOK)
            {
                int res = FDAConfig::addC24Environment(params.env);
                if (res != 0)
                {
                    handleError(res, hDlg);
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
        // Edit (FI property, c24.output.dir, or C24 environment)
        // --------------------------------------------------
        case IDC_PROPERTIES_EDIT:
        {
            HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);
            int  sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (sel < 0 || sel >= static_cast<int>(gDisplayRows.size())) return TRUE;

            const DisplayRow& row = gDisplayRows[sel];

            if (row.isC24Group)
            {
                FDAC24Environment env;
                bool decryptFailed = false;
                if (!FDAConfig::getC24Environment(row.identifier, env, decryptFailed)) return TRUE;

                if (decryptFailed)
                {
                    MessageBox(hDlg,
                        TEXT("Unable to decrypt credentials for this environment.\n\n")
                        TEXT("The stored data may be corrupted, or this environment was added on a different machine."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONERROR);
                    return TRUE; // abort — don't open the edit dialog with blank/corrupted credentials
                }

                C24EnvEditParams params;
                params.env = env;
                params.nameReadOnly = true;

                INT_PTR result = DialogBoxParam(
                    FDAApplication::getModuleHandle(),
                    MAKEINTRESOURCE(IDD_FDA_C24_ENV_EDIT_DIALOG),
                    hDlg,
                    C24EnvEditDialogProc,
                    reinterpret_cast<LPARAM>(&params)
                );

                if (result == IDOK)
                {
                    params.env.name = row.identifier;
                    int res = FDAConfig::updateC24Environment(params.env);
                    if (res != 0) handleError(res, hDlg);
                    else { PopulateList(hList); UpdateButtonStates(hDlg); }
                }
            }
            else
            {
                const std::vector<FDAProperty>& props = FDAConfig::getProperties();
                const FDAProperty* selected = nullptr;
                for (const FDAProperty& p : props)
                    if (p.key == row.identifier) { selected = &p; break; }

                if (!selected || !selected->editable) return TRUE;

                PropertyEditParams params;
                params.key = selected->key;
                params.value = selected->value;
                params.keyReadOnly = true;

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
                    if (res != 0) handleError(res, hDlg);
                    else { PopulateList(hList); UpdateButtonStates(hDlg); }
                }
            }

            return TRUE;
        }

        // --------------------------------------------------
        // Delete (FI property or C24 environment group)
        // --------------------------------------------------
        case IDC_PROPERTIES_DELETE:
        {
            HWND hList = GetDlgItem(hDlg, IDC_PROPERTIES_LIST);
            int  sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (sel < 0 || sel >= static_cast<int>(gDisplayRows.size())) return TRUE;

            const DisplayRow& row = gDisplayRows[sel];

            if (row.isC24Group)
            {
                std::wstring msg = L"Deleting environment '" + NarrowToWide(row.identifier) +
                    L"' will remove all its entries (host, port, username, password, "
                    L"bank ID, backend path, frontend path).\n\nContinue?";

                if (MessageBox(hDlg, msg.c_str(), TEXT("Finacle Dev Assist"),
                    MB_YESNO | MB_ICONWARNING) == IDYES)
                {
                    int res = FDAConfig::deleteC24Environment(row.identifier);
                    if (res != 0) handleError(res, hDlg);
                    else { PopulateList(hList); UpdateButtonStates(hDlg); }
                }
            }
            else
            {
                std::wstring msg = L"Delete property '" + NarrowToWide(row.identifier) + L"'?";

                if (MessageBox(hDlg, msg.c_str(), TEXT("Finacle Dev Assist"),
                    MB_YESNO | MB_ICONWARNING) == IDYES)
                {
                    int res = FDAConfig::deleteProperty(row.identifier);
                    if (res != 0) handleError(res, hDlg);
                    else { PopulateList(hList); UpdateButtonStates(hDlg); }
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
        if (hIconBig) { DestroyIcon(hIconBig);    hIconBig = nullptr; }
        break;
    }
    }

    return FALSE;
}